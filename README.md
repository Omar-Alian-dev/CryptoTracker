<div align="center">
  <h1>📈 CryptoTracker</h1>
  <h3>Real-Time Cryptocurrency Dashboard</h3>
  <p><strong>Analyze. Visualize. Track.</strong></p>
</div>

---

## 📑 Table of Contents
- [🎯 Overview](#-overview)
- [❗ Problem Statement](#-problem-statement)
- [✨ Features](#-features)
- [🛠️ Technical Stack](#-technical-stack)
- [🚀 Getting Started](#-getting-started)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

---

## 🎯 Overview

**CryptoTracker** is a high-performance, real-time cryptocurrency monitoring application built using **Modern C++17**, featuring a fully interactive **ImGui** desktop interface.

The application integrates with the **CoinGecko API** to fetch live price data and utilizes a robust multithreaded backend to ensure the UI remains fluid and responsive, even during heavy network operations.

🏆 **Goal:** To demonstrate mastery in Threading, Networking, UI Rendering, Synchronization, and System Architecture within the C++ ecosystem.

---

## ❗ Problem Statement

Traditional crypto dashboards often struggle with performance bottlenecks.
**Common issues include:**
* 🐢 **Slow API refresh cycles** leading to stale data.
* ❄️ **UI freezing** caused by blocking network operations on the main thread.
* 📉 **Lack of real-time insights** and visual history.
* 🧩 **Poor threading & synchronization** leading to race conditions.
* 💾 **No local data persistence** for user preferences.

**CryptoTracker solves these issues with:**
* 🚀 **Background Threading:** Data fetching happens asynchronously.
* ✔ **Real-Time Updates:** Live prices update without stuttering the UI.
* ✔ **Live Graphing:** Visual price-history plotting.
* ✔ **Smart Error Handling:** Automated API backoff logic.
* ✔ **Persistence:** Favorites system saved locally to disk.
* ✔ **Fluid UI:** Fast rendering via DirectX11 & ImGui.

---

## ✨ Features

### 📊 **Live Market Data**
* Fetches real-time data from **CoinGecko** via HTTPS.
* Displays Price, 24h Percentage Change, and Market Cap.

### ⚙️ **Threaded Data Fetching**
* Implements a background refresh loop using `std::thread`.
* Ensures thread-safety with `std::mutex` and `std::atomic` synchronization.

### ⭐ **Favorites System**
* Users can mark specific coins as favorites.
* Data is persisted between sessions using filesystem storage (`favorites.txt`).

### 📈 **Live Price Graph**
* Uses ImGui plotting to visualize price trends over time.
* History buffer updates automatically with every refresh cycle.

### 🔍 **Search & Filtering**
* Instant filtering by coin name.
* Optional **“Show Favorites Only”** toggle for a focused view.

### 🛡 **Smart API Backoff**
* Detects HTTP 429 (Rate Limit) errors.
* Automatically adjusts refresh delay using exponential backoff to prevent bans.

---

## 🛠️ Technical Stack

| Category | Technology | Description |
| :--- | :--- | :--- |
| **Core Language** | ![C++](https://img.shields.io/badge/-C++17-00599C?logo=c%2B%2B) | Main application logic |
| **UI Framework** | ![ImGui](https://img.shields.io/badge/-ImGui-764ABC) | Immediate Mode GUI for desktop rendering |
| **Networking** | **httplib** | SSL/HTTPS Client for API requests |
| **Serialization** | **nlohmann/json** | JSON parsing and object mapping |
| **Threading** | `std::thread`, `std::mutex` | Background worker threads & synchronization |
| **Storage** | `std::filesystem` | Persistent local storage for favorites |
| **Platform** | **Windows + DirectX11** | Backend rendering pipeline |

---
## 🚀 Getting Started
Prerequisites
Visual Studio 2022 (with C++ Desktop Development workload)

C++17 compliant compiler

Windows 10/11

DirectX11 compatible GPU

Installation
Clone the repository

Bash
git clone [https://github.com/Omar-Alian-dev/CryptoTracker.git](https://github.com/Omar-Alian-dev/CryptoTracker.git)
cd CryptoTracker
Build the project

Open the .sln solution file in Visual Studio.

Select Release or Debug configuration.

Press F5 to build and run.

---
## 🤝 Contributing
This is a private academic project. External contributions are not accepted at this time.

🐛 Bug Reports
If you've found a bug, please open an issue including:

A clear description of the issue.

Steps to reproduce.

---
## 📄 License
Proprietary License

Copyright (c) 2026 Tayseer Zeer, and Omar Alian All rights reserved.

This software may not be copied, modified, or distributed without explicit permission from the author.
