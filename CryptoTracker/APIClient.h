#pragma once
#include "httplib.h"
#include "json.hpp"
#include "CryptoData.h"
#include <iostream>
#include <vector>
#include <string>

using json = nlohmann::json;

class APIClient {
public:
    static std::vector<CryptoCoin> fetchTopCoins(std::string& statusMsg) {
        std::vector<CryptoCoin> coins;
        bool networkSuccess = false;

        // --- ATTEMPT 1: REAL NETWORK ---
        try {
            // Try CoinDesk (HTTP)
            httplib::Client cli("api.coindesk.com", 80);
            cli.set_connection_timeout(2); // Short timeout so UI doesn't freeze long

            auto res = cli.Get("/v1/bpi/currentprice.json");

            if (res && res->status == 200) {
                json result = json::parse(res->body);

                CryptoCoin btc;
                btc.id = "bitcoin";
                btc.name = "Bitcoin";
                btc.symbol = "BTC";
                btc.current_price = result["bpi"]["USD"]["rate_float"].get<double>();
                btc.price_change_24h = ((rand() % 200) - 100) / 100.0; // Fake change for demo
                btc.market_cap = 0.0;

                coins.push_back(btc);
                networkSuccess = true;
                statusMsg = "Live Data (CoinDesk)";
            }
        }
        catch (...) {
            // Network failed, just ignore and fall through to mock data
        }

        // --- ATTEMPT 2: OFFLINE FALLBACK (If Network Failed) ---
        if (!networkSuccess) {
            statusMsg = "Offline Mode (Simulated)";

            // Add Fake Data so the App is Usable
            coins.push_back({ "bitcoin", "BTC", "Bitcoin", 64230.50, -1.25, 1200000000 });
            coins.push_back({ "ethereum", "ETH", "Ethereum", 3450.10, 2.40, 400000000 });
            coins.push_back({ "solana", "SOL", "Solana", 145.20, 5.10, 65000000 });
            coins.push_back({ "cardano", "ADA", "Cardano", 0.45, -0.50, 15000000 });
            coins.push_back({ "ripple", "XRP", "Ripple", 0.60, 1.10, 30000000 });
            coins.push_back({ "polkadot", "DOT", "Polkadot", 7.20, -2.10, 10000000 });
            coins.push_back({ "dogecoin", "DOGE", "Dogecoin", 0.12, 8.50, 18000000 });
        }

        return coins;
    }
};