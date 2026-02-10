#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <algorithm>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>       // For file saving (Required)
#include <unordered_set> // For storing favorites (Required)
#include <unordered_map>
#include <algorithm>     // For search text conversion
#include <filesystem>   // For filesystem (Required)
namespace fs = std::filesystem;



#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "ws2_32.lib")

#include <tchar.h>
#include <thread>
#include <atomic>
#include <mutex>
#include "APIClient.h"

// --- DX11 GLOBAL VARIABLES ---
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// --- APP STATE ---
std::vector<CryptoCoin> g_coins;
std::unordered_set<std::string> g_favorites; // Stores symbols of favorite coins (e.g., "BTC", "ETH")
std::string g_statusMessage = "Initializing...";
std::mutex g_dataMutex;
std::atomic<bool> g_running(true);
std::atomic<bool> g_loading(false);
std::string g_selectedSymbol; // Symbol of the coin currently selected in the UI
std::unordered_map<std::string, std::vector<float>> g_priceHistory;

// NEW: refresh interval (seconds)
constexpr int DEFAULT_REFRESH_SECONDS = 30;  // was 10; now more gentle
constexpr int MAX_REFRESH_SECONDS = 300; // 5 minutes max backoff
std::atomic<int> g_refreshSeconds(DEFAULT_REFRESH_SECONDS);

// --- FILE PATHS (Grade Requirement: filesystem) ---
const fs::path DATA_DIR = "data";
const fs::path FAVORITES_FILE = DATA_DIR / "favorites.txt";


// Helper Functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// --- FILE SYSTEM FUNCTIONS (Grade Requirement: fstream) ---
void LoadFavorites() {
    try {
        // Ensure data directory exists
        if (!fs::exists(DATA_DIR)) {
            fs::create_directories(DATA_DIR);
        }

        // If the file doesn't exist yet, nothing to load
        if (!fs::exists(FAVORITES_FILE)) {
            return;
        }

        std::ifstream file(FAVORITES_FILE);
        if (file.is_open()) {
            std::string symbol;
            while (std::getline(file, symbol)) {
                if (!symbol.empty()) {
                    g_favorites.insert(symbol);
                }
            }
            file.close();
        }
    }
    catch (const std::exception& e) {
        g_statusMessage = std::string("Filesystem error (load): ") + e.what();
    }
}

void SaveFavorites() {
    try {
        // Ensure data directory exists
        if (!fs::exists(DATA_DIR)) {
            fs::create_directories(DATA_DIR);
        }

        std::ofstream file(FAVORITES_FILE);
        if (file.is_open()) {
            for (const auto& symbol : g_favorites) {
                file << symbol << "\n";
            }
            file.close();
        }
    }
    catch (const std::exception& e) {
        g_statusMessage = std::string("Filesystem error (save): ") + e.what();
    }
}

void ToggleFavorite(const std::string& symbol) {
    if (g_favorites.count(symbol)) {
        g_favorites.erase(symbol);
    }
    else {
        g_favorites.insert(symbol);
    }
    SaveFavorites(); // Save immediately when changed
}

// --- BACKGROUND THREAD ---
void DataFetcher() {
    int currentSleep = DEFAULT_REFRESH_SECONDS;

    while (g_running) {
        g_loading = true;
        std::string localError;
        std::vector<CryptoCoin> newData = APIClient::fetchTopCoins(localError);

        bool rateLimited = false;

        {
            std::lock_guard<std::mutex> lock(g_dataMutex);

            if (!newData.empty()) {
                // Update current snapshot
                g_coins = newData;

                // --- update price history (you already had this part earlier) ---
                constexpr size_t MAX_HISTORY_POINTS = 120; // or whatever you chose
                for (const auto& coin : g_coins) {
                    auto& history = g_priceHistory[coin.symbol];
                    history.push_back(static_cast<float>(coin.current_price));
                    if (history.size() > MAX_HISTORY_POINTS) {
                        size_t extra = history.size() - MAX_HISTORY_POINTS;
                        history.erase(history.begin(), history.begin() + extra);
                    }
                }
                // ----------------------------------------------------------------

                g_statusMessage = "Live Data: refreshed every "
                    + std::to_string(currentSleep) + "s";
            }
            else {
                // Error path
                g_statusMessage = "Error: " + localError;

                // Detect rate-limit hint (HTTP 429 or message text)
                if (localError.find("429") != std::string::npos ||
                    localError.find("limit") != std::string::npos ||
                    localError.find("Limit") != std::string::npos) {
                    rateLimited = true;
                }
            }
        }

        g_loading = false;

        // Adjust sleep time based on rate limiting
        if (rateLimited) {
            // Exponential backoff: double, but clamp to MAX_REFRESH_SECONDS
            currentSleep = std::min(currentSleep * 2, MAX_REFRESH_SECONDS);
        }
        else {
            // On success or non-rate-limit errors, use default frequency
            currentSleep = DEFAULT_REFRESH_SECONDS;
        }

        g_refreshSeconds = currentSleep;

        std::this_thread::sleep_for(std::chrono::seconds(currentSleep));
    }
}


// --- MAIN FUNCTION ---
int main(int, char**)
{
    // 1. Initialize Networking & Files
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    LoadFavorites(); // Load saved data

    // 2. Setup Window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"CryptoTracker", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Crypto Tracker Project", WS_OVERLAPPEDWINDOW, 100, 100, 1000, 600, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // 3. Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // 4. Start Thread
    std::thread fetchThread(DataFetcher);

    // 5. UI Variables
    static char searchBuffer[128] = "";
    static bool showFavoritesOnly = false;

    // 6. Main Loop
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

            // --- HEADER ---
            ImGui::Text("Crypto Tracker - Final Project");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Advanced C++ Assignment");
            ImGui::Separator();

            // --- CONTROLS ---
            ImGui::InputText("Search Name", searchBuffer, IM_ARRAYSIZE(searchBuffer));
            ImGui::SameLine();
            ImGui::Checkbox("Show Favorites Only", &showFavoritesOnly);

            ImGui::Spacing();

            ImGui::Text("Status: %s", g_statusMessage.c_str());
            ImGui::SameLine();
            ImGui::Text("(Refresh: %d s)", g_refreshSeconds.load());


            // --- TABLE ---
            if (ImGui::BeginTable("Coins", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Fav", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Symbol");
                ImGui::TableSetupColumn("Price (USD)");
                ImGui::TableSetupColumn("24h Change");
                ImGui::TableHeadersRow();

                std::lock_guard<std::mutex> lock(g_dataMutex);

                for (const auto& coin : g_coins) {
                    // FILTER 1: Search Logic
                    std::string nameLower = coin.name;
                    std::string searchLower = searchBuffer;
                    // Simple lowercase conversion for case-insensitive search
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

                    if (searchLower.length() > 0 && nameLower.find(searchLower) == std::string::npos) {
                        continue; // Skip if name doesn't match search
                    }

                    // FILTER 2: Favorites Logic
                    bool isFav = g_favorites.count(coin.symbol);
                    if (showFavoritesOnly && !isFav) {
                        continue; // Skip if we only want favorites
                    }

                    // RENDER ROW
                    ImGui::TableNextRow();

                    // Column 1: Favorite Checkbox
                    ImGui::TableSetColumnIndex(0);
                    if (ImGui::Checkbox(("##" + coin.symbol).c_str(), &isFav)) {
                        ToggleFavorite(coin.symbol);
                    }

                    // Column 2: Name (clickable – selects this coin)
                    ImGui::TableSetColumnIndex(1);
                    bool isSelected = (!g_selectedSymbol.empty() && g_selectedSymbol == coin.symbol);
                    if (ImGui::Selectable(coin.name.c_str(), isSelected)) {
                        g_selectedSymbol = coin.symbol;
                    }

                    // Column 3: Symbol
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text(coin.symbol.c_str());

                    // Column 4: Price
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("$%.2f", coin.current_price);

                    // Column 5: 24h Change
                    ImGui::TableSetColumnIndex(4);
                    if (coin.price_change_24h >= 0)
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "+%.2f%%", coin.price_change_24h);
                    else
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%.2f%%", coin.price_change_24h);
                }
                // --- SELECTED COIN DETAILS ---
                if (!g_selectedSymbol.empty()) {
                    const CryptoCoin* selected = nullptr;

                    // Find the selected coin by symbol
                    for (const auto& coin : g_coins) {
                        if (coin.symbol == g_selectedSymbol) {
                            selected = &coin;
                            break;
                        }
                    }

                    if (selected) {
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Text("Selected Coin Details");

                        ImGui::Text("Name: %s (%s)", selected->name.c_str(), selected->symbol.c_str());
                        ImGui::Text("Current Price: $%.2f", selected->current_price);
                        ImGui::Text("24h Change: %.2f%%", selected->price_change_24h);
                        ImGui::Text("Market Cap: $%.0f", selected->market_cap);

                        // --- NEW: price history graph ---
                        auto it = g_priceHistory.find(selected->symbol);
                        if (it != g_priceHistory.end() && it->second.size() >= 2) {
                            const std::vector<float>& hist = it->second;

                            // Compute min/max for nicer scaling
                            float minPrice = hist[0];
                            float maxPrice = hist[0];
                            for (float v : hist) {
                                if (v < minPrice) minPrice = v;
                                if (v > maxPrice) maxPrice = v;
                            }

                            // Add a small margin
                            float scaleMin = minPrice * 0.95f;
                            float scaleMax = maxPrice * 1.05f;

                            ImGui::Spacing();
                            ImGui::Text("Price History (recent refreshes):");
                            ImGui::PlotLines(
                                "",                    // no label inside the graph
                                hist.data(),
                                static_cast<int>(hist.size()),
                                0,
                                nullptr,               // no overlay text
                                scaleMin,
                                scaleMax,
                                ImVec2(0, 100.0f)      // width auto, height ~100px
                            );
                        }
                        else {
                            ImGui::Spacing();
                            ImGui::Text("Price History: collecting data...");
                        }
                        // --- END: price history graph ---
                    }
                }
                // (we'll add the details panel here in the next step)
                ImGui::EndTable();
            }
            ImGui::End();
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    g_running = false;
    if (fetchThread.joinable()) fetchThread.join();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    WSACleanup();

    return 0;
}

// --- BOILERPLATE HELPER FUNCTIONS (UNCHANGED) ---
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;
    CreateRenderTarget();
    return true;
}
void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}
void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}
void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}