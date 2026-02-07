#pragma once
#include <windows.h>
#include <wininet.h>
#include "json.hpp"
#include "CryptoData.h"
#include <iostream>
#include <vector>
#include <string>

// Link the native Windows Internet library
#pragma comment(lib, "wininet.lib")

using json = nlohmann::json;

class APIClient {
public:
    static std::vector<CryptoCoin> fetchTopCoins(std::string& statusMsg) {
        std::vector<CryptoCoin> coins;
        
        // 1. Initialize WinINet (Native Windows Internet)
        HINTERNET hInternet = InternetOpenA("CryptoTrackerApp", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            statusMsg = "Error: Could not open Windows Internet.";
            return coins;
        }

        // 2. Open Secure HTTPS Connection to CoinGecko
        // WinINet handles SSL/HTTPS automatically!
        statusMsg = "Connecting...";
        HINTERNET hConnect = InternetOpenUrlA(hInternet, 
            "https://api.coingecko.com/api/v3/coins/markets?vs_currency=usd&order=market_cap_desc&per_page=10&page=1&sparkline=false", 
            NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);

        if (hConnect) {
            // 3. Read the Data
            std::string responseData;
            char buffer[4096];
            DWORD bytesRead;

            while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                responseData.append(buffer, bytesRead);
            }
            InternetCloseHandle(hConnect);

            // 4. Parse JSON
            try {
                if (!responseData.empty()) {
                    json result = json::parse(responseData);
                    
                    if (result.is_array()) {
                        for (auto& item : result) {
                            CryptoCoin coin;
                            coin.id = item["id"].get<std::string>();
                            coin.symbol = item["symbol"].get<std::string>();
                            coin.name = item["name"].get<std::string>();
                            coin.current_price = item["current_price"].get<double>();
                            
                            // Safe parsing for optional fields
                            if (item.contains("price_change_percentage_24h") && !item["price_change_percentage_24h"].is_null()) {
                                coin.price_change_24h = item["price_change_percentage_24h"].get<double>();
                            }
                            if (item.contains("market_cap") && !item["market_cap"].is_null()) {
                                coin.market_cap = item["market_cap"].get<double>();
                            }

                            coins.push_back(coin);
                        }
                        statusMsg = "Live Data: Connected via WinINet (HTTPS)";
                    } else {
                        statusMsg = "Error: API did not return a list.";
                    }
                } else {
                    statusMsg = "Error: Received empty response from server.";
                }
            }
            catch (const std::exception& e) {
                statusMsg = std::string("JSON Error: ") + e.what();
            }
        }
        else {
            statusMsg = "Connection Failed (WinINet Error)";
        }

        InternetCloseHandle(hInternet);
        return coins;
    }
};