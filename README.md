🧭 Table of Contents

Overview

Problem Statement

Requirements

Features

Technical Stack

Architecture

Screenshots

Getting Started

Future Enhancements

Achievements

About the Developer

Contributing

License

🎯 Overview

CryptoTracker is an advanced C++ application designed to monitor real-time cryptocurrency markets using a polished ImGui desktop interface.
It features:

Live API data retrieval from CoinGecko

A background multi-threaded data fetcher

Live price chart visualizations

Search, filtering, and favorites management

Persistent storage and clean UI

Modern C++17 architecture

This project was developed as part of the Advanced C++ course final project and showcases practical implementation of threading, networking, UI rendering, and system architecture.

❗ Problem Statement

Traditional cryptocurrency trackers often suffer from:

High resource usage

Slow or limited updates

Poor UI responsiveness

Lack of system-level programming practices

No offline caching or customization

CryptoTracker was designed to solve these problems while demonstrating:

Real-time API integration

Thread-safe background data processing

Modern C++ design

Scalable UI interactions

📋 Requirements

This project meets (and exceeds) the official course requirements:

✔ Multi-threading

✔ Mutex & atomic synchronization

✔ STL usage (vector, unordered_set/map)

✔ File I/O (favorites storage)

✔ External HTTP requests

✔ JSON parsing

✔ GUI using ImGui

✔ Real-time data updates

✔ Error handling & API rate-limit backoff

✔ Data visualization (graphs)

✨ Features
🔹 Live Market Data

Fetches real-time crypto market data from CoinGecko using an HTTPS API client.

🔹 Multi-threaded Data Fetcher

A dedicated background thread safely updates the dataset using mutex and atomic synchronization.

🔹 Search & Filtering

Search cryptocurrencies instantly and filter by favorites.

🔹 Favorites System

Favorite coins are stored persistently using filesystem storage.

🔹 Live Price History Graph

Draws a real-time graph updated with each new fetch cycle.

🔹 Smart API Rate-Limit Handling

Implements exponential backoff when API rate limits occur.

🔹 Clean & Modern UI

Built entirely with ImGui and DirectX11 for fluid desktop rendering.

🛠 Technical Stack
Component	Technology
Language	C++17
GUI	Dear ImGui + DirectX11 Backend
Networking	cpp-httplib (HTTPS)
Data Serialization	nlohmann/json
Threading	std::thread, std::mutex, std::atomic
Storage	fstream + std::filesystem
OS	Windows 10/11
🧩 Architecture
+-------------------------------------------------------------+
|                         UI Layer (ImGui)                    |
|  - Dashboard Table                                           |
|  - Search & Filters                                          |
|  - Selected Coin Details                                     |
|  - Live History Graph                                        |
+-------------------------------------------------------------+
|                     Application Controller                   |
|  - Event handling                                            |
|  - Rendering loop                                           |
+-------------------------------------------------------------+
|                 Background Data Fetcher (Thread)            |
|  - API requests via HTTPS                                   |
|  - JSON parsing                                             |
|  - Mutex-protected updates                                  |
|  - Exponential backoff on 429                                |
+-------------------------------------------------------------+
|                    Data Persistence Layer                    |
|  - favorites.txt handling                                   |
|  - filesystem operations                                     |
+-------------------------------------------------------------+

🖼 Screenshots

(Add your actual screenshots here)

Dashboard view

Selected coin details

Price history graph

Favorites view

Error/Rate-limit notifications

🚀 Getting Started
1️⃣ Clone the repository
git clone https://github.com/YOUR-USERNAME/CryptoTracker.git

2️⃣ Install Dependencies

Visual Studio 2022

C++17 toolset

ImGui (included or as submodule)

httplib

nlohmann/json

3️⃣ Build & Run

Open the .sln file in Visual Studio and run the main project.

🔮 Future Enhancements

Binance WebSocket support (true real-time prices)

Custom themes for ImGui

Multi-API fallback system

Sorting by market cap / price change

Candlestick charts (OHLC)

Enhanced error logging

🏆 Achievements

Fully meets the Advanced C++ project requirements

Demonstrates modern multi-threaded architecture

Implements live graphs and real-time UI

Stable long-term runtime with rate-limit protection

👤 About the Developer
Tayseer Zeer

Software Engineer | C++ Developer | Full Stack Enthusiast

🎓 BSc in Software Engineering
🌍 Based in Jerusalem
💼 Skilled in C++, Python, JavaScript, SQL, and software architecture
🧩 Passion for systems programming, networked applications, and UI/UX

Connect

📧 Email: tayseerzeer@gmail.com

🌐 GitHub: (your GitHub profile link)

💼 LinkedIn: (your LinkedIn profile link)

🤝 Contributing

This is a private academic project; external contributions are not accepted.
For code review or suggestions, please contact the developer directly.

🐞 Bug Reports

If you have identified a bug:

Clear description

How to reproduce

Expected vs actual behavior

Screenshots (if possible)

Submit directly to the developer.

📄 License
PROPRIETARY LICENSE

Copyright (c) 2025
All rights reserved.

This software is proprietary and may not be copied, modified,
distributed, or reused without explicit permission from the copyright holder.

<div align="center">

🌟 If you like this project, consider starring the repo!
Made with ❤️ by Tayseer Zeer

</div>
