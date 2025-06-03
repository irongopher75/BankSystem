#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

using namespace std;
string sha256(const string &str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)str.c_str(), str.length(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}


#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>  // For std::invalid_argument
using namespace std;

class BankSystem {
private:
    struct BankDetailsAuto {
        string AccountNumber;
        string ifsc;
    };
    struct TransferLog {
        string fromAccount;
        string toAccount;
        string toIFSC;
        double amount;
        string type;  // "Internal" or "External"
    };

    struct BankDetailsMan {
        string name, dob, address;
        long long balance;
        double loanTaken;
    };

    struct LoginCredentials {
        string AccountNumber;
        string Password;
    };

    vector<BankDetailsMan> manualAccounts;
    vector<BankDetailsAuto> autoAccounts;
    vector<LoginCredentials> credentials;
    vector<TransferLog> transferLogs;
    map<string, string> cityIFSCCodePrefix;
    map<string, double> rates;  // Fixed: removed reference

    string generateAccountNumber() {
        return "BANK" + to_string(1000 + credentials.size());
    }

public:
    BankSystem() {
        rates["Savings"] = 3.5;
        rates["FixedDeposit"] = 6.7;
        rates["RecurringDeposit"] = 5.5;
        cityIFSCCodePrefix["Mumbai"] = "MBI";
        cityIFSCCodePrefix["Delhi"] = "DEL";
        cityIFSCCodePrefix["Bangalore"] = "BLR";
        cityIFSCCodePrefix["Chennai"] = "CHE";
        cityIFSCCodePrefix["Hyderabad"] = "HYD";
        cityIFSCCodePrefix["Pune"] = "PUN";
        cityIFSCCodePrefix["Ahmedabad"] = "AMD";
    }

    double getMaxLoanCapacity(int idx) {
        return manualAccounts[idx].balance * 0.5;
    }

    void createAccount() {
        string name, dob, address;

        try {
            cout << "Enter name: ";
            getline(cin >> ws, name);

            cout << "Enter date of birth (ddmmyyyy): ";
            cin >> dob;
            if (dob.length() != 8)
                throw invalid_argument("DOB must be 8 digits (ddmmyyyy)");

            dob = dob.substr(0, 2) + "/" + dob.substr(2, 2) + "/" + dob.substr(4, 4);

            cout << "Enter home address: ";
            getline(cin >> ws, address);
            string city;
            cout << "Enter your city: ";
            getline(cin >> ws, city);

            string accountNumber = generateAccountNumber();
            string password;
            cout << "Set a password for your account: ";
            cin >> password;

            manualAccounts.push_back({name, dob, address, 0}); // balance = 0 initially
            string ifscCode = "BANK" + cityIFSCCodePrefix[city] + to_string(1000 + credentials.size());
            autoAccounts.push_back({accountNumber, ifscCode});
            credentials.push_back({accountNumber, sha256(password)});

            cout << "\nAccount created successfully!\n";
            cout << "Name: " << name << "\nDOB: " << dob << "\nAddress: " << address << endl;
            cout << "Account Number: " << accountNumber << "\nPlease remember your password.\n";
        } catch (const invalid_argument &e) {
            cout << "Error: " << e.what() << endl;
        } catch (...) {
            cout << "An unknown error occurred while creating account.\n";
        }
    }

    bool check(string &AccountNumber, string Password) {
        for (const auto &cred : credentials) {
            if (cred.AccountNumber == AccountNumber && cred.Password == sha256(Password)) {
                return true;
            }
        }
        return false;
    }

    string loginAccount() {
    string AccountNumber, Password;
    cout << "Enter your Account Number: ";
    cin >> AccountNumber;
    cout << "Enter your password: ";
    cin >> Password;

    if (check(AccountNumber, Password)) {
        cout << "Logged in Successfully\n";
        return AccountNumber;
    }

    cout << "Invalid Credentials. You have 2 more attempts.\n";
    return "";
    }


    void viewInterestRates() {
        cout << "===== Current Interest Rates =====" << endl;
        for (const auto &pair : rates) {
            cout << pair.first << ": " << pair.second << "%" << endl;
        }
    }

    void updateInterestRates() {
        string accountType;
        double newRate;
        cout << "Enter the account type to update (e.g., Savings, FixedDeposit): ";
        cin >> accountType;
        cout << "Enter the new interest rate: ";
        cin >> newRate;
        rates[accountType] = newRate;
        cout << "Updated " << accountType << " interest rate to " << newRate << "%.\n";
    }

    void withdrawMoney(const string& accNum) {
    double amount;
    bool found = false;

    for (size_t i = 0; i < autoAccounts.size(); ++i) {
        if (autoAccounts[i].AccountNumber == accNum) {
            cout << "Enter amount to withdraw: ";
            cin >> amount;
            if (amount <= manualAccounts[i].balance) {
                manualAccounts[i].balance -= amount;
                cout << "Withdrawal successful! New Balance: ₹" << manualAccounts[i].balance << endl;
            } else {
                cout << "Insufficient balance.\n";
            }
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Account not found!\n";
    }
}

    void checkMaxLoanCapacity(const string& accNum) {
    bool found = false;

    for (size_t i = 0; i < autoAccounts.size(); ++i) {
        if (autoAccounts[i].AccountNumber == accNum) {
            double limit = manualAccounts[i].balance * 2;
            cout << "Your maximum loan eligibility is ₹" << limit << endl;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Account not found!\n";
    }
}

    void transferMoney(const string& fromAccNum) {
    string toAccNum, toIfsc;
    double amount;
    int fromIndex = -1;

    for (size_t i = 0; i < autoAccounts.size(); ++i) {
        if (autoAccounts[i].AccountNumber == fromAccNum) {
            fromIndex = i;
            break;
        }
    }

    if (fromIndex == -1) {
        cout << "Sender account not found!\n";
        return;
    }

    cout << "Enter recipient Account Number: ";
    cin >> toAccNum;
    cout << "Enter recipient IFSC code: ";
    cin >> toIfsc;
    cout << "Enter amount to transfer: ";
    cin >> amount;

    if (manualAccounts[fromIndex].balance < amount) {
        cout << "Insufficient balance for transfer.\n";
        return;
    }

    bool isInternal = false;
    int toIndex = -1;

    for (size_t i = 0; i < autoAccounts.size(); ++i) {
        if (autoAccounts[i].AccountNumber == toAccNum && autoAccounts[i].ifsc == toIfsc) {
            isInternal = true;
            toIndex = i;
            break;
        }
    }

    manualAccounts[fromIndex].balance -= amount;

    if (isInternal) {
        manualAccounts[toIndex].balance += amount;
        cout << "₹" << amount << " successfully transferred to internal account.\n";

        transferLogs.push_back({fromAccNum, toAccNum, toIfsc, amount, "Internal"});
    } else {
        cout << "₹" << amount << " transferred to account " << toAccNum
             << " at IFSC " << toIfsc << ". (External bank)\n";

        transferLogs.push_back({fromAccNum, toAccNum, toIfsc, amount, "External"});
    }
}

    void viewTransferLogs() {
    cout << "\n====== Transfer Logs ======\n";
    if (transferLogs.empty()) {
        cout << "No transfers recorded yet.\n";
        return;
    }

    for (const auto &log : transferLogs) {
        cout << "From: " << log.fromAccount << " → To: " << log.toAccount
             << " (IFSC: " << log.toIFSC << "), Amount: ₹" << log.amount
             << ", Type: " << log.type << endl;
    }
}

    void adminMenu() {
    int adminChoice;
    do {
        cout << "\n========== ADMIN PANEL ==========\n";
        cout << "1. View Interest Rates\n";
        cout << "2. Update Interest Rates\n";
        cout << "3. View All Accounts\n";
        cout << "4. View Transfer Logs\n";
        cout << "5. Exit Admin Panel\n";
        cout << "=================================\n";
        cout << "Enter your choice: ";
        cin >> adminChoice;

        switch (adminChoice) {
            case 1:
                viewInterestRates();
                break;
            case 2:
                updateInterestRates();
                break;
            case 3:
                viewAllAccounts();
                break;
            case 4:
                viewTransferLogs();
                break;
            case 5:
                cout << "Exiting Admin Panel.\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    } while (adminChoice != 5);
}

    void viewAllAccounts() {
    cout << "\n====== All Registered Accounts ======\n";
    for (size_t i = 0; i < autoAccounts.size(); ++i) {
        cout << "Account Number: " << autoAccounts[i].AccountNumber << "\n";
        cout << "IFSC: " << autoAccounts[i].ifsc << "\n";
        cout << "Name: " << manualAccounts[i].name << "\n";
        cout << "DOB: " << manualAccounts[i].dob << "\n";
        cout << "Address: " << manualAccounts[i].address << "\n";
        cout << "Balance: ₹" << manualAccounts[i].balance << "\n";
        cout << "-------------------------------------\n";
    }
}

    void userMenu(BankSystem &bank, const string &AccountNumber) {
    int choice;
    do {
        cout << "\n===== Account Menu =====\n";
        cout << "1. Deposit Money\n";
        cout << "2. Withdraw Money \n";
        cout << "3. Issue Loan\n";
        cout << "4. Repay Loan\n";
        cout << "5. Check Balance\n";
        cout << "6. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                bank.depositMoney(AccountNumber);
                break;
            case 2:
                cout << "Withdraw Money - Coming Soon\n";
                break;
            case 3: {
                double amount;
                cout << "Enter loan amount to request: ";
                cin >> amount;
                bank.issueLoan(AccountNumber, amount);
                break;
            }
            case 4: {
                double amount;
                cout << "Enter amount to repay loan: ";
                cin >> amount;
                bank.repayLoan(AccountNumber, amount);
                break;
            }
            case 5:
                bank.checkBalance(AccountNumber);
                break;
            case 6:
                cout << "Logging out...\n";
                break;
            default:
                cout << "Invalid choice, try again.\n";
        }
    } while (choice != 6);
}

    int findUserIndex(const string& AccountNumber) {
        for (size_t i = 0; i < autoAccounts.size(); ++i) {
            if (autoAccounts[i].AccountNumber == AccountNumber) {
                return i;
            }
        }
        return -1;
    }

    void depositMoney(const string& AccountNumber) {
        int idx = findUserIndex(AccountNumber);
        if (idx == -1) {
            cout << "Account not found.\n";
            return;
        }

        long long amount;
        cout << "Enter amount to deposit: ";
        cin >> amount;

        if (amount <= 0) {
            cout << "Invalid deposit amount.\n";
            return;
        }

        manualAccounts[idx].balance += amount;
        cout << "₹" << amount << " deposited successfully.\n";
        cout << "New balance: ₹" << manualAccounts[idx].balance << endl;
    }

    void checkBalance(const string& AccountNumber) {
        int idx = findUserIndex(AccountNumber);
        if (idx == -1) {
            cout << "Account not found.\n";
            return;
        }

        cout << "Current balance: ₹" << manualAccounts[idx].balance << endl;
        cout << "Outstanding loan: ₹" << manualAccounts[idx].loanTaken << endl;
        double maxCapacity = manualAccounts[idx].balance * 0.5;
        cout << "Available loan capacity: ₹" << (maxCapacity - manualAccounts[idx].loanTaken) << endl;
    }

    void issueLoan(const string& AccountNumber, double requestedLoan) {
        int idx = findUserIndex(AccountNumber);
        if (idx == -1) {
            cout << "Account not found.\n";
            return;
        }

        BankDetailsMan &user = manualAccounts[idx];
        double maxCapacity = user.balance * 0.5;
        double availableLoanCapacity = maxCapacity - user.loanTaken;

        if (requestedLoan <= 0) {
            cout << "Invalid loan amount.\n";
            return;
        }

        if (requestedLoan > availableLoanCapacity) {
            cout << "Loan request exceeds your available capacity of ₹" << availableLoanCapacity << ".\n";
            return;
        }

        user.loanTaken += requestedLoan;
        user.balance += requestedLoan;

        cout << "Loan of ₹" << requestedLoan << " approved.\n";
        cout << "Remaining loan capacity: ₹" << (maxCapacity - user.loanTaken) << endl;
    }

    void repayLoan(const string& AccountNumber, double repayAmount) {
        int idx = findUserIndex(AccountNumber);
        if (idx == -1) {
            cout << "Account not found.\n";
            return;
        }

        BankDetailsMan &user = manualAccounts[idx];

        if (repayAmount <= 0) {
            cout << "Invalid repayment amount.\n";
            return;
        }

        if (repayAmount > user.loanTaken) {
            cout << "Repayment amount exceeds loan taken.\n";
            return;
        }

        if (repayAmount > user.balance) {
            cout << "Insufficient balance to repay loan.\n";
            return;
        }

        user.loanTaken -= repayAmount;
        user.balance -= repayAmount;

        cout << "Repayment of ₹" << repayAmount << " successful.\n";
        cout << "Remaining loan balance: ₹" << user.loanTaken << endl;
    }

};

int main(void) {
    BankSystem bank;
    int choice;
    while (true) {
        cout << "============================================\n";
        cout << "         WELCOME TO SECURE BANKING          \n";
        cout << "============================================\n";
        cout << "\nPlease choose an option from the menu below:\n";
        cout << "--------------------------------------------\n";
        cout << "1. Create New Account\n";
        cout << "2. Login to Existing Account\n";
        cout << "3. View Interest Rates\n";
        cout << "4. Customer Support\n";
        cout << "5. Exit\n";
        cout << "6. Administrator Login\n";
        cout << "--------------------------------------------\n";
        cout << "Enter your choice (1-6): ";
        cin >> choice;

        switch (choice) {
            case 1:
                bank.createAccount();
                break;
             case 2: {
                string accNum;
                cout << "Enter your Account Number: ";
                cin >> accNum;

                string password;
                cout << "Enter your password: ";
                cin >> password;

                if (bank.check(accNum, password)) {
                    cout << "Logged in Successfully.\n";
                    bank.userMenu(bank, accNum);  // pass control to user menu
                } else {
                    cout << "Invalid Credentials. You have 2 more attempts left.\n";
                }
                break;
            }


            case 3:
                bank.viewInterestRates();
                break;
            case 4:
                cout << "Customer Support: Call 9769268125\n";
                break;
            case 5:
                cout << "Thank you for using Secure Banking. Goodbye!\n";
                return 0;
            case 6: {
                string pass;
                cout << "Enter Admin Password: ";
                cin >> pass;
                if (pass == "admin123") {
                    bank.adminMenu();
                } else {
                    cout << "Unauthorized access.\n";
                }
                break;
            }

            default:
                cout << "Invalid choice. Please try again.\n";
        }

        cout << "\nPress Enter to continue...\n";
        cin.ignore();
        cin.get();
    }

    return 0;
}