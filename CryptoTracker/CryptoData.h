#pragma once
#include <string>
#include <vector>

struct CryptoCoin {
    std::string id = "";
    std::string symbol = "";
    std::string name = "";
    double current_price = 0.0;
    double price_change_24h = 0.0;
    double market_cap = 0.0;
};