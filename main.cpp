// Library Management System
// Console-based, using plain structs, global arrays and file I/O (no classes).

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <limits>
#include <cstdlib>
#include <cctype>
using namespace std;

const int MAX_USERS = 100;
const int MAX_BOOKS = 50;
const int MAX_HISTORY = 500;
const int MAX_PIN_ATTEMPTS = 3;

// Fine rules for late returns
const int LOAN_PERIOD_DAYS = 7;
const int FINE_TIER1 = 20;  // 1-3 days late
const int FINE_TIER2 = 50;  // 4-6 days late
const int FINE_TIER3 = 100; // 7+ days late

struct User {
    string id;
    string name;
    string phone;
    string borrowedBook;
    time_t borrowDate;
    int fine;
    bool hasBook;
    bool suspended;
    string pin;
    int failedAttempts;
};

struct Book {
    string title;
    string author;
    bool available;
};

// One row per borrow. Created when a book is borrowed (returnDate stays 0
// until it comes back), updated when it's returned. Lets a user or admin
// look back at everything that was ever borrowed, not just the current book.
struct HistoryRecord {
    string userID;
    string bookTitle;
    time_t borrowDate;
    time_t returnDate; // 0 means "not returned yet"
    int fine;          // fine charged for this specific borrow, if any
};

User users[MAX_USERS];
Book books[MAX_BOOKS];
HistoryRecord history[MAX_HISTORY];
int userCount = 0;
int bookCount = 0;
int historyCount = 0;

// Function declarations
void loadData();
void saveData();
void registerUser();
int findUser(string id);
void borrowBook(User* user);
void returnBook(User* user);
void listBooks();

bool adminLogin();
void adminMenu();
void addBook();
void removeBook();
void suspendUser();
void unsuspendUser();
void clearFine();
void viewUserHistoryAdmin();

bool isAllDigits(string s);
bool containsLetter(string s);
string toLowerStr(string s);
void searchBooks();
void printHistoryRecords(User* user);
void viewMyHistory(User* user);
void checkLoginAlerts(User* user);

// Reads a menu choice as a whole line rather than a single character, and
// only accepts it if the line is exactly one character long. This means
// typing something like "12" is rejected as invalid input instead of being
// silently read as "1" with the "2" thrown away.
char readMenuChoice() {
    string line;
    getline(cin, line);
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    if (line.length() == 1) {
        return line[0];
    }
    return '\0';
}


void flushLine() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Clears the console so each menu prints on a fresh screen instead of
// stacking endlessly below all the previous output.
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Waits for the user to press Enter before the next clearScreen() wipes the
// result of their last action off the screen, so they get a chance to read it.
void pauseForUser() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

// True only if every character is a digit 0-9 (and the string isn't empty).
// Used to make sure a phone number doesn't contain letters or symbols.
bool isAllDigits(string s) {
    if (s.empty()) return false;
    for (int i = 0; i < (int)s.length(); i++) {
        if (!isdigit((unsigned char)s[i])) {
            return false;
        }
    }
    return true;
}

// True if the string contains at least one letter. Used to make sure a name
// isn't left empty or made up of only numbers/symbols.
bool containsLetter(string s) {
    for (int i = 0; i < (int)s.length(); i++) {
        if (isalpha((unsigned char)s[i])) {
            return true;
        }
    }
    return false;
}

// Lowercases a copy of the string. Used so book search matches regardless
// of how the user or the catalog capitalizes things.
string toLowerStr(string s) {
    for (int i = 0; i < (int)s.length(); i++) {
        s[i] = tolower((unsigned char)s[i]);
    }
    return s;
}

int main() {
    loadData();

    string userID;
    char choice;
    int index = -1;

    while (true) {
        clearScreen();
        cout << "\n==== Library System ====\n";
        cout << "1. User Login\n";
        cout << "2. Admin Login\n";
        cout << "Enter choice: ";
        choice = readMenuChoice();

        if (choice == '1') {
            clearScreen();
            cout << "Are you a new user? (y/n): ";
            choice = readMenuChoice();

            if (choice == 'y') {
                registerUser();
            }
            else if (choice == 'n') {
                // continue
            }
            else {
                cout << "Invalid choice.\n";
                pauseForUser();
                continue;
            }

            clearScreen();
            cout << "Enter your User ID: ";
            cin >> userID;
            flushLine();
            index = findUser(userID);

            if (index != -1) {
                if (users[index].suspended) {
                    clearScreen();
                    cout << "This account is suspended. Please contact admin.\n";
                    pauseForUser();
                    continue;
                }

                clearScreen();
                string enteredPin;
                cout << "Enter your 4-digit PIN: ";
                cin >> enteredPin;
                flushLine();

                if (enteredPin == users[index].pin) {
                    users[index].failedAttempts = 0;
                    break; // go to user menu
                } else {
                    users[index].failedAttempts++;
                    if (users[index].failedAttempts >= MAX_PIN_ATTEMPTS) {
                        users[index].suspended = true;
                        users[index].failedAttempts = 0;
                        cout << "Incorrect PIN. Too many failed attempts - this account has been suspended. Please contact admin.\n";
                    } else {
                        cout << "Incorrect PIN. " << (MAX_PIN_ATTEMPTS - users[index].failedAttempts) << " attempt(s) remaining.\n";
                    }
                    saveData(); // persist the attempt count / lockout immediately, in case the program closes before a normal exit
                    pauseForUser();
                    continue;
                }
            }

            clearScreen();
            cout << "User ID not found.\n";
            cout << "1. Try again\n";
            cout << "2. Forgot User ID/Name\n";
            cout << "Enter choice: ";
            choice = readMenuChoice();

            if (choice == '1') {
                continue;
            } else if (choice == '2') {
                clearScreen();
                string phone;
                cout << "Enter your registered phone number: ";
                cin >> phone;
                flushLine();

                bool found = false;
                for (int i = 0; i < userCount; i++) {
                    if (users[i].phone == phone) {
                        cout << "Your User ID is: " << users[i].id << "\n";
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    cout << "Phone number not found in system.\n";
                }

                cout << "Returning to login...\n";
                pauseForUser();
            } else {
                cout << "Invalid choice.\n";
                pauseForUser();
            }
        }
        else if (choice == '2') {
            if (adminLogin()) {
                adminMenu();
                saveData();
            }
            pauseForUser();
        }
        else {
            cout << "Invalid choice.\n";
            pauseForUser();
        }
    }

    User* currentUser = &users[index];
    checkLoginAlerts(currentUser);

    do {
        clearScreen();
        cout << "\nChoose an action:\n";
        cout << "1. Borrow a book\n";
        cout << "2. Return a book\n";
        cout << "3. List available books\n";
        cout << "4. Search books\n";
        cout << "5. View my borrow history\n";
        cout << "6. Exit\n";
        cout << "Enter choice: ";
        choice = readMenuChoice();

        switch (choice) {
            case '1': borrowBook(currentUser); pauseForUser(); break;
            case '2': returnBook(currentUser); pauseForUser(); break;
            case '3': listBooks(); pauseForUser(); break;
            case '4': searchBooks(); pauseForUser(); break;
            case '5': viewMyHistory(currentUser); pauseForUser(); break;
            case '6': cout << "Exiting system. Goodbye!\n"; break;
            default: cout << "Invalid choice.\n"; pauseForUser();
        }
    } while (choice != '6');

    saveData();
    return 0;
}

// ================== FILE HANDLING ==================
// Records are stored one per line, fields separated by '|'.
void loadData() {
    ifstream userFile("users.txt");
    string line;

    while (userCount < MAX_USERS && getline(userFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string field;
        User temp = {};

        getline(ss, temp.id, '|');
        getline(ss, temp.name, '|');
        getline(ss, temp.phone, '|');
        getline(ss, temp.borrowedBook, '|');

        getline(ss, field, '|');
        temp.borrowDate = (time_t)stol(field);

        getline(ss, field, '|');
        temp.hasBook = (field == "1");

        getline(ss, field, '|');
        temp.fine = stoi(field);

        getline(ss, field, '|');
        temp.suspended = (field == "1");

        getline(ss, temp.pin, '|');

        getline(ss, field, '|');
        temp.failedAttempts = field.empty() ? 0 : stoi(field);

        users[userCount++] = temp;
    }
    userFile.close();

    ifstream bookFile("books.txt");
    while (bookCount < MAX_BOOKS && getline(bookFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string field;
        Book temp = {};

        getline(ss, temp.title, '|');
        getline(ss, temp.author, '|');

        getline(ss, field, '|');
        temp.available = (field == "1");

        books[bookCount++] = temp;
    }
    bookFile.close();

    ifstream historyFile("history.txt");
    while (historyCount < MAX_HISTORY && getline(historyFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string field;
        HistoryRecord temp = {};

        getline(ss, temp.userID, '|');
        getline(ss, temp.bookTitle, '|');

        getline(ss, field, '|');
        temp.borrowDate = (time_t)stol(field);

        getline(ss, field, '|');
        temp.returnDate = (time_t)stol(field);

        getline(ss, field, '|');
        temp.fine = stoi(field);

        history[historyCount++] = temp;
    }
    historyFile.close();
}

void saveData() {
    ofstream userFile("users.txt");
    for (int i = 0; i < userCount; i++) {
        userFile << users[i].id << "|"
                 << users[i].name << "|"
                 << users[i].phone << "|"
                 << users[i].borrowedBook << "|"
                 << users[i].borrowDate << "|"
                 << (users[i].hasBook ? 1 : 0) << "|"
                 << users[i].fine << "|"
                 << (users[i].suspended ? 1 : 0) << "|"
                 << users[i].pin << "|"
                 << users[i].failedAttempts << "\n";
    }
    userFile.close();

    ofstream bookFile("books.txt");
    for (int i = 0; i < bookCount; i++) {
        bookFile << books[i].title << "|"
                 << books[i].author << "|"
                 << (books[i].available ? 1 : 0) << "\n";
    }
    bookFile.close();

    ofstream historyFile("history.txt");
    for (int i = 0; i < historyCount; i++) {
        historyFile << history[i].userID << "|"
                    << history[i].bookTitle << "|"
                    << history[i].borrowDate << "|"
                    << history[i].returnDate << "|"
                    << history[i].fine << "\n";
    }
    historyFile.close();
}

// ================== USER FUNCTIONS ==================
void registerUser() {
    clearScreen();

    if (userCount >= MAX_USERS) {
        cout << "User limit reached.\n";
        pauseForUser();
        return;
    }

    // Auto-generate the ID as "U" followed by a zero-padded number based on
    // how many users have registered so far (U001, U002, U003, ...). This
    // guarantees every ID is unique and consistently formatted.
    int idNumber = userCount + 1;
    string numPart = to_string(idNumber);
    while (numPart.length() < 3) {
        numPart = "0" + numPart;
    }
    string newID = "U" + numPart;

    string name;
    do {
        cout << "Enter your name: ";
        getline(cin, name);
        if (!containsLetter(name)) {
            cout << "Name must contain at least one letter. Please try again.\n";
        }
    } while (!containsLetter(name));

    string phone;
    do {
        cout << "Enter your phone number (digits only): ";
        cin >> phone;
        flushLine();
        if (!isAllDigits(phone)) {
            cout << "Phone number must contain digits only. Please try again.\n";
        }
    } while (!isAllDigits(phone));

    string pin;
    do {
        cout << "Set a 4-digit PIN (you'll need this to log in): ";
        cin >> pin;
        flushLine();
        if (!isAllDigits(pin) || pin.length() != 4) {
            cout << "PIN must be exactly 4 digits. Please try again.\n";
        }
    } while (!isAllDigits(pin) || pin.length() != 4);

    users[userCount].id = newID;
    users[userCount].name = name;
    users[userCount].phone = phone;
    users[userCount].pin = pin;
    users[userCount].failedAttempts = 0;
    users[userCount].borrowedBook = "";
    users[userCount].borrowDate = 0;
    users[userCount].hasBook = false;
    users[userCount].fine = 0;
    users[userCount].suspended = false; // default active
    userCount++;

    cout << "User registered successfully!\n";
    cout << "Your User ID is: " << newID << " - please remember it (and your PIN) for future logins.\n";
    pauseForUser();
}

int findUser(string id) {
    for (int i = 0; i < userCount; i++) {
        if (users[i].id == id) return i;
    }
    return -1;
}

void borrowBook(User* user) {
    if (user->suspended) {
        cout << "Your account is suspended. Please contact admin.\n";
        return;
    }

    if (user->fine > 0) {
        cout << "You have an unpaid fine of Rs. " << user->fine
             << ". Please contact admin to clear it before borrowing.\n";
        return;
    }

    if (user->hasBook) {
        cout << "You already borrowed a book: " << user->borrowedBook << "\n";
        return;
    }

    string bookName;
    cout << "Enter book name: ";
    getline(cin, bookName);

    for (int i = 0; i < bookCount; i++) {
        if (books[i].title == bookName && books[i].available) {
            books[i].available = false;
            user->hasBook = true;
            user->borrowedBook = books[i].title;
            time(&user->borrowDate);
            cout << "Book borrowed successfully on " << ctime(&user->borrowDate);

            if (historyCount < MAX_HISTORY) {
                history[historyCount].userID = user->id;
                history[historyCount].bookTitle = books[i].title;
                history[historyCount].borrowDate = user->borrowDate;
                history[historyCount].returnDate = 0;
                history[historyCount].fine = 0;
                historyCount++;
            }
            return;
        }
    }

    cout << "Book not available or not found.\n";
}

void returnBook(User* user) {
    if (!user->hasBook) {
        cout << "You have no book to return.\n";
        return;
    }

    string bookName;
    cout << "Enter book name to return: ";
    getline(cin, bookName);

    if (user->borrowedBook != bookName) {
        cout << "You did not borrow this book.\n";
        return;
    }

    for (int i = 0; i < bookCount; i++) {
        if (books[i].title == bookName) {
            books[i].available = true;
            user->hasBook = false;
            user->borrowedBook = "";

            time_t now;
            time(&now);
            double seconds = difftime(now, user->borrowDate);
            int daysBorrowed = seconds / (60 * 60 * 24);

            int daysLate = daysBorrowed - LOAN_PERIOD_DAYS;
            int fineCharged = 0;
            if (daysLate <= 0) {
                cout << "Book returned on time. No fine.\n";
            } else {
                if (daysLate <= 3) {
                    fineCharged = FINE_TIER1;
                } else if (daysLate <= 6) {
                    fineCharged = FINE_TIER2;
                } else {
                    fineCharged = FINE_TIER3;
                }
                user->fine += fineCharged;
                cout << "Late by " << daysLate << " days. Fine: Rs. " << fineCharged << "\n";
            }

            // Find the matching open borrow record (returnDate still 0) and fill it in.
            for (int j = 0; j < historyCount; j++) {
                if (history[j].userID == user->id && history[j].bookTitle == bookName && history[j].returnDate == 0) {
                    history[j].returnDate = now;
                    history[j].fine = fineCharged;
                    break;
                }
            }

            return;
        }
    }

    cout << "Book not found in system.\n";
}

void listBooks() {
    cout << "\nAvailable Books:\n";
    for (int i = 0; i < bookCount; i++) {
        if (books[i].available) {
            cout << "- " << books[i].title << " by " << books[i].author << "\n";
        }
    }
}

void searchBooks() {
    string keyword;
    cout << "Enter a title or author keyword to search: ";
    getline(cin, keyword);
    string keywordLower = toLowerStr(keyword);

    cout << "\nSearch results:\n";
    bool foundAny = false;
    for (int i = 0; i < bookCount; i++) {
        if (toLowerStr(books[i].title).find(keywordLower) != string::npos ||
            toLowerStr(books[i].author).find(keywordLower) != string::npos) {
            cout << "- " << books[i].title << " by " << books[i].author;
            cout << (books[i].available ? " (Available)\n" : " (Borrowed)\n");
            foundAny = true;
        }
    }
    if (!foundAny) {
        cout << "No books matched your search.\n";
    }
}

// Prints every history row belonging to the given user. Shared by the
// user's own "view my history" option and the admin's "view user history".
void printHistoryRecords(User* user) {
    bool any = false;
    for (int i = 0; i < historyCount; i++) {
        if (history[i].userID == user->id) {
            any = true;
            cout << "- " << history[i].bookTitle << "\n";
            cout << "  Borrowed: " << ctime(&history[i].borrowDate);
            if (history[i].returnDate == 0) {
                cout << "  Status: Currently borrowed\n";
            } else {
                cout << "  Returned: " << ctime(&history[i].returnDate);
                cout << "  Fine charged: Rs. " << history[i].fine << "\n";
            }
        }
    }
    if (!any) {
        cout << "No borrow history found.\n";
    }
}

void viewMyHistory(User* user) {
    cout << "\n--- Your Borrow History ---\n";
    printHistoryRecords(user);
}

// Called right after a successful login, before the action menu appears.
// Flags anything the user should know about right away instead of finding
// out only when they try to return a book or borrow a new one.
void checkLoginAlerts(User* user) {
    bool hasAlert = false;
    clearScreen();

    if (user->hasBook) {
        time_t now;
        time(&now);
        double seconds = difftime(now, user->borrowDate);
        int daysBorrowed = seconds / (60 * 60 * 24);
        int daysLate = daysBorrowed - LOAN_PERIOD_DAYS;
        if (daysLate > 0) {
            cout << "You have an overdue book: \"" << user->borrowedBook << "\" - "
                 << daysLate << " day(s) late. Please return it soon.\n";
            hasAlert = true;
        }
    }

    if (user->fine > 0) {
        cout << "You have an outstanding fine of Rs. " << user->fine
             << ". Please clear it with admin.\n";
        hasAlert = true;
    }

    if (hasAlert) {
        pauseForUser();
    }
}

// ================== ADMIN FUNCTIONS ==================
bool adminLogin() {
    clearScreen();
    string username, password;
    cout << "Enter Admin Username: ";
    cin >> username;
    flushLine();
    cout << "Enter Admin Password: ";
    cin >> password;
    flushLine();

    if (username == "admin" && password == "admin123") {
        cout << "Admin login successful!\n";
        return true;
    } else {
        cout << "Invalid username or password.\n";
        return false;
    }
}

void addBook() {
    if (bookCount >= MAX_BOOKS) {
        cout << "Book limit reached.\n";
        return;
    }

    cout << "Enter book title: ";
    getline(cin, books[bookCount].title);

    cout << "Enter author: ";
    getline(cin, books[bookCount].author);

    books[bookCount].available = true;
    bookCount++;

    cout << "Book added successfully!\n";
}

void removeBook() {
    string title;
    cout << "Enter book title to remove: ";
    getline(cin, title);

    for (int i = 0; i < bookCount; i++) {
        if (books[i].title == title) {
            if (!books[i].available) {
                cout << "Cannot remove \"" << books[i].title
                     << "\" - it is currently borrowed by a user.\n";
                return;
            }

            for (int j = i; j < bookCount - 1; j++) {
                books[j] = books[j + 1];
            }
            bookCount--;
            cout << "Book removed successfully!\n";
            return;
        }
    }
    cout << "Book not found.\n";
}

void suspendUser() {
    string userID;
    cout << "Enter User ID to suspend: ";
    cin >> userID;
    flushLine();

    int index = findUser(userID);
    if (index == -1) {
        cout << "User not found.\n";
        return;
    }

    users[index].suspended = true;
    cout << "User " << users[index].name << " (ID: " << users[index].id << ") has been suspended.\n";
}

void unsuspendUser() {
    string userID;
    cout << "Enter User ID to unsuspend: ";
    cin >> userID;
    flushLine();

    int index = findUser(userID);
    if (index == -1) {
        cout << "User not found.\n";
        return;
    }

    if (!users[index].suspended) {
        cout << "User is not suspended.\n";
        return;
    }

    users[index].suspended = false;
    users[index].failedAttempts = 0; // clear any PIN-lockout history too
    cout << "User " << users[index].name << " (ID: " << users[index].id << ") has been unsuspended.\n";
}

void clearFine() {
    string userID;
    cout << "Enter User ID to clear fine: ";
    cin >> userID;
    flushLine();

    int index = findUser(userID);
    if (index == -1) {
        cout << "User not found.\n";
        return;
    }

    if (users[index].fine > 0) {
        cout << "User " << users[index].name << " has a fine of Rs. "
             << users[index].fine << ". Clearing now...\n";
        users[index].fine = 0;
        cout << "Fine cleared successfully.\n";
    } else {
        cout << "This user has no pending fine.\n";
    }
}

void viewUserHistoryAdmin() {
    string userID;
    cout << "Enter User ID to view borrow history: ";
    cin >> userID;
    flushLine();

    int index = findUser(userID);
    if (index == -1) {
        cout << "User not found.\n";
        return;
    }

    cout << "\nBorrow history for " << users[index].name << " (" << users[index].id << "):\n";
    printHistoryRecords(&users[index]);
}

void adminMenu() {
    char choice;
    do {
        clearScreen();
        cout << "\n--- Admin Menu ---\n";
        cout << "1. Add Book\n";
        cout << "2. Remove Book\n";
        cout << "3. List All Books & Users\n";
        cout << "4. Suspend User\n";
        cout << "5. Unsuspend User\n";
        cout << "6. Clear User Fine\n";
        cout << "7. View User Borrow History\n";
        cout << "8. Exit Admin Menu\n";
        cout << "Enter choice: ";
        choice = readMenuChoice();

        switch (choice) {
            case '1': addBook(); pauseForUser(); break;
            case '2': removeBook(); pauseForUser(); break;
            case '3':
                for (int i = 0; i < bookCount; i++) {
                    cout << "- " << books[i].title << " by " << books[i].author;
                    if (books[i].available) cout << " (Available)\n";
                    else cout << " (Borrowed)\n";
                }
                cout << "\n--- User List ---\n";
                for (int i = 0; i < userCount; i++) {
                    cout << users[i].id << " - " << users[i].name
                         << " | Fine: Rs." << users[i].fine
                         << " | Suspended: " << (users[i].suspended ? "Yes" : "No") << "\n";
                }
                pauseForUser();
                break;
            case '4': suspendUser(); pauseForUser(); break;
            case '5': unsuspendUser(); pauseForUser(); break;
            case '6': clearFine(); pauseForUser(); break;
            case '7': viewUserHistoryAdmin(); pauseForUser(); break;
            case '8': cout << "Exiting Admin Menu.\n"; break;
            default: cout << "Invalid choice.\n"; pauseForUser();
        }
    } while (choice != '8');
}
