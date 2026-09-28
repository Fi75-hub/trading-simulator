# Trading Simulator

A C++ console application for exploring historical cryptocurrency order data, generating OHLC summaries and managing simulated wallets and transactions. Developed by **Faizan Ilyas** as the final project for Object Oriented Programming (CM2005), University of London.

## Features

- Load market orders from CSV and select from the available currency pairs.
- Summarise ask and bid prices as open, high, low and close values by day, month or year.
- Register a local demo account and log in using a generated ten-digit username.
- Deposit and withdraw simulated funds, view wallet balances and review recent transactions.
- Filter trading statistics by product and date prefix.
- Generate sample bids and asks using historical reference prices and the current timestamp.
- Persist local account, wallet and transaction state in CSV files.

## Technology

C++17 and the C++ standard library. Classes separate application menus, CSV parsing, market analysis, accounts and storage. No third-party C++ libraries are required.

## Build and run

Use a C++17 compiler such as GCC/MinGW. Run these commands from the repository root.

**Windows PowerShell (GCC/MinGW on PATH):**

```powershell
g++ -std=c++17 -Wall -Wextra -I include src/*.cpp -o trading-simulator.exe
.\trading-simulator.exe
```

**Linux/macOS with GCC or Clang:**

```sh
g++ -std=c++17 -Wall -Wextra -I include src/*.cpp -o trading-simulator
./trading-simulator
```

The program loads `data/20200601.csv` automatically. Choose **Register**, create a fictional demo account and save the generated username shown in the console. Use that username to log in and explore the numbered menus.

Run from the repository root so the relative `data/` paths resolve. The folder must be writable: missing `users.csv`, `wallets.csv` and `transactions.csv` are created automatically. Personal account records and compiled executables are excluded from this repository.

## Included data

- `data/20200601.csv`: the first **10,000 rows** of the original 1,021,772-row coursework dataset, kept at the filename expected by the application. This sample contains all five currency pairs, both ask and bid orders, and 22 timestamps.
- `data/20200317.csv`: the smaller supplied market dataset, retained in full.

The full June dataset remains in the original local coursework folder and has not been replaced there. The sample keeps this portfolio copy lightweight; its summaries represent only the included rows. Both supplied files cover a single date, so the current samples do not demonstrate changes over multiple days or years. Market CSV files were supplied with the coursework and are not claimed as original data collection.

## Project structure

```text
include/   Class declarations and shared data structures
src/       Application, market analysis, account and CSV implementation
data/      Historical market samples; local state is generated here
```

## Scope

This is an educational simulation with historical data and fictional balances. It does not connect to an exchange or execute real trades. Authentication is a coursework implementation using `std::hash` and a simplified reset flow; use fictional details and a disposable password. It is not a production account system.
