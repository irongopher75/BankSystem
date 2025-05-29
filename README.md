# Banking System Web Interface

This project is a simple banking system using C++ (with Crow for web backend) and an HTML frontend.

## Files
- `bank_web.cpp`: Backend server using Crow.
- `bank.cpp`: Additional C++ logic (optional, depending on your use).
- `banking_systems.html`: Frontend interface.

## How to Run
1. Compile the backend:
   ```sh
   g++ -std=c++17 bank_web.cpp -o bank_web -lpthread -I/opt/homebrew/include
   ./bank_web
