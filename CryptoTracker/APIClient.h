#pragma once

#include <windows.h>
#include "json.hpp"
#include "CryptoData.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#include "httplib.h"
#pragma warning(pop)

#include <iostream>
#include <vector>
#include <string>

// Link the native Windows Internet library
#pragma comment(lib, "wininet.lib")

using json = nlohmann::json;

class APIClient {
public:
    // ---------------------------------------------------------------------
    // Main function used by your app: fetches top coins from CoinGecko
    // using HTTPLIB SSL
    // ---------------------------------------------------------------------
    static std::vector<CryptoCoin> fetchTopCoins(std::string& statusMsg) {
        std::vector<CryptoCoin> coins;

        try {
            statusMsg = "Connecting via httplib SSL (CoinGecko)...";

            // HTTPS client for CoinGecko (requires CPPHTTPLIB_OPENSSL_SUPPORT)
            httplib::SSLClient cli("api.coingecko.com");

            // For a course project it's OK to disable verification.
            // In real apps you keep this ON.
            cli.enable_server_certificate_verification(false);

            cli.set_connection_timeout(5);   // 5 seconds to connect
            cli.set_read_timeout(5, 0);      // 5 seconds to read

            const char* path =
                "/api/v3/coins/markets"
                "?vs_currency=usd"
                "&order=market_cap_desc"
                "&per_page=10"
                "&page=1"
                "&sparkline=false";

            auto res = cli.Get(path);

            // ---------- basic response checks ----------
            if (!res) {
                statusMsg = "[HTTPLIB SSL] No response from CoinGecko.";
                return coins;
            }

            if (res->status != 200) {
                if (res->status == 429) {
                    statusMsg = "API limit reached (HTTP 429). Using last data, will retry...";
                    return coins; 
                }

                statusMsg = "[HTTPLIB SSL] HTTP " + std::to_string(res->status) +
                    " from CoinGecko.";
                return coins;
            }

            if (res->body.empty()) {
                statusMsg = "[HTTPLIB SSL] Empty body from CoinGecko.";
                return coins;
            }
            // ---------- parse JSON ----------
            json result = json::parse(res->body);

            if (!result.is_array()) {
                statusMsg = "[HTTPLIB SSL] API did not return a list.";
                return coins;
            }
            for (auto& item : result) {
                CryptoCoin coin;

                // Required fields
                coin.id = item["id"].get<std::string>();
                coin.symbol = item["symbol"].get<std::string>();
                coin.name = item["name"].get<std::string>();
                coin.current_price = item["current_price"].get<double>();

                // Optional fields
                if (item.contains("price_change_percentage_24h") &&
                    !item["price_change_percentage_24h"].is_null()) {
                    coin.price_change_24h =
                        item["price_change_percentage_24h"].get<double>();
                }

                if (item.contains("market_cap") &&
                    !item["market_cap"].is_null()) {
                    coin.market_cap = item["market_cap"].get<double>();
                }

                coins.push_back(coin);
            }

            statusMsg =
                "Live Data: Connected via httplib (CoinGecko HTTPS)";
        }
        catch (const std::exception& e) {
            statusMsg = std::string("[HTTPLIB SSL EXCEPTION] ") + e.what();
        }
        catch (...) {
            statusMsg = "[HTTPLIB SSL EXCEPTION] Unknown error.";
        }
        return coins;
    }

};
