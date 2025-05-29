#include "crow/app.h"
#include "crow/json.h"
#include <unordered_map>
#include <string>
#include <mutex>
#include <iostream>

// Dummy user database: username -> hashed password (in real app, store securely)
std::unordered_map<std::string, std::string> users = {
    {"user1", "hashedpassword1"},
    {"user2", "hashedpassword2"}
};

// User balances
std::unordered_map<std::string, double> balances = {
    {"user1", 1000.0},
    {"user2", 500.0}
};

// Mutex to protect balance updates
std::mutex balance_mutex;

// Dummy password verification function
bool verify_user(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) return false;
    return it->second == password;
}

int main() {
    crow::SimpleApp app;

    std::cout << "Starting server on port 8080...\n";

    // Login endpoint: POST /login
    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        if (verify_user(username, password)) {
            return crow::response(200, "Login successful");
        } else {
            return crow::response(401, "Invalid credentials");
        }
    });

    // Check balance endpoint: GET /balance/<username>
    CROW_ROUTE(app, "/balance/<string>")([](const std::string& username) {
        std::lock_guard<std::mutex> lock(balance_mutex);

        auto it = balances.find(username);
        if (it == balances.end())
            return crow::response(404, "User not found");

        crow::json::wvalue res;
        res["username"] = username;
        res["balance"] = it->second;

        return crow::response{res};
    });

    // Transfer funds endpoint: POST /transfer
    CROW_ROUTE(app, "/transfer").methods("POST"_method)([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string from = body["from"].s();
        std::string to = body["to"].s();
        double amount = 0.0;
        try {
            amount = body["amount"].d();
        } catch (...) {
            return crow::response(400, "Invalid amount");
        }

        if (users.find(from) == users.end() || users.find(to) == users.end())
            return crow::response(404, "Sender or receiver not found");

        if (amount <= 0)
            return crow::response(400, "Amount must be greater than zero");

        {
            std::lock_guard<std::mutex> lock(balance_mutex);

            if (balances[from] < amount)
                return crow::response(400, "Insufficient balance");

            balances[from] -= amount;
            balances[to] += amount;
        }

        crow::json::wvalue res;
        res["status"] = "success";
        res["from"] = from;
        res["to"] = to;
        res["amount"] = amount;

        return crow::response{res};
    });

    app.port(8080).multithreaded().run();
    return 0;
}
