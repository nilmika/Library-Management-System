// Library Management System
// Console-based, using plain structs, global arrays and file I/O (no classes).

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <limits>
using namespace std;

const int MAX_USERS = 100;
const int MAX_BOOKS = 50;

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
};

struct Book {
    string title;
    string author;
    bool available;
};

User users[MAX_USERS];
Book books[MAX_BOOKS];
int userCount = 0;
int bookCount = 0;

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

int main() {
    loadData();

    string userID;
    char choice;
    int index = -1;

    while (true) {
        cout << "\n==== Library System ====\n";
        cout << "1. User Login\n";
        cout << "2. Admin Login\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == '1') {
            cout << "Are you a new user? (y/n): ";
            cin >> choice;

            if (choice == 'y') {
                registerUser();
            }
            else if (choice == 'n') {
                // continue
            }
            else {
                cout << "Invalid choice.\n";
                continue;
            }

            cout << "Enter your User ID: ";
            cin >> userID;
            index = findUser(userID);

            if (index != -1) {
                break; // go to user menu
            }

            cout << "User ID not found.\n";
            cout << "1. Try again\n";
            cout << "2. Forgot User ID/Name\n";
            cout << "Enter choice: ";
            cin >> choice;

            if (choice == '1') {
                continue;
            } else if (choice == '2') {
                string phone;
                cout << "Enter your registered phone number: ";
                cin >> phone;

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
            } else {
                cout << "Invalid choice.\n";
            }
        }
        else if (choice == '2') {
            if (adminLogin()) {
                adminMenu();
                saveData();
            }
        }
        else {
            cout << "Invalid choice.\n";
        }
    }

    User* currentUser = &users[index];

    do {
        cout << "\nChoose an action:\n";
        cout << "1. Borrow a book\n";
        cout << "2. Return a book\n";
        cout << "3. List available books\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case '1': borrowBook(currentUser); break;
            case '2': returnBook(currentUser); break;
            case '3': listBooks(); break;
            case '4': cout << "Exiting system. Goodbye!\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != '4');

    saveData();
    return 0;
}

// ================== FILE HANDLING ==================
// Records are stored one per line, fields separated by '|'.
// A plain ">>" read (the old approach) splits on ANY whitespace, so a name
// or book title containing a space (e.g. "Harry Potter") would get cut into
// two fields and corrupt every field after it. Reading full lines and
// splitting on '|' avoids that.
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
                 << (users[i].suspended ? 1 : 0) << "\n";
    }
    userFile.close();

    ofstream bookFile("books.txt");
    for (int i = 0; i < bookCount; i++) {
        bookFile << books[i].title << "|"
                 << books[i].author << "|"
                 << (books[i].available ? 1 : 0) << "\n";
    }
    bookFile.close();
}

// ================== USER FUNCTIONS ==================
void registerUser() {
    if (userCount >= MAX_USERS) {
        cout << "User limit reached.\n";
        return;
    }

    string newID;
    cout << "Enter new User ID: ";
    cin >> newID;

    for (int i = 0; i < userCount; i++) {
        if (users[i].id == newID) {
            cout << "User ID already exists. Try a different one.\n";
            return;
        }
    }
    users[userCount].id = newID;

    cout << "Enter your name: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, users[userCount].name);
    cout << "Enter your phone number: ";
    cin >> users[userCount].phone;

    users[userCount].borrowedBook = "";
    users[userCount].borrowDate = 0;
    users[userCount].hasBook = false;
    users[userCount].fine = 0;
    users[userCount].suspended = false; // default active
    userCount++;

    cout << "User registered successfully!\n";
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
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter book name: ";
    getline(cin, bookName);

    for (int i = 0; i < bookCount; i++) {
        if (books[i].title == bookName && books[i].available) {
            books[i].available = false;
            user->hasBook = true;
            user->borrowedBook = books[i].title;
            time(&user->borrowDate);
            cout << "Book borrowed successfully on " << ctime(&user->borrowDate);
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
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
            if (daysLate <= 0) {
                cout << "Book returned on time. No fine.\n";
            } else {
                if (daysLate <= 3) {
                    user->fine += FINE_TIER1;
                    cout << "Late by " << daysLate << " days. Fine: Rs. " << FINE_TIER1 << "\n";
                } else if (daysLate <= 6) {
                    user->fine += FINE_TIER2;
                    cout << "Late by " << daysLate << " days. Fine: Rs. " << FINE_TIER2 << "\n";
                } else {
                    user->fine += FINE_TIER3;
                    cout << "Late by " << daysLate << " days. Fine: Rs. " << FINE_TIER3 << "\n";
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

// ================== ADMIN FUNCTIONS ==================
bool adminLogin() {
    string username, password;
    cout << "Enter Admin Username: ";
    cin >> username;
    cout << "Enter Admin Password: ";
    cin >> password;

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
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, books[bookCount].title);

    cout << "Enter author: ";
    getline(cin, books[bookCount].author);

    books[bookCount].available = true;
    bookCount++;

    cout << "Book added successfully!\n";
}

void removeBook() {
    string title;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
    cout << "User " << users[index].name << " (ID: " << users[index].id << ") has been unsuspended.\n";
}

void clearFine() {
    string userID;
    cout << "Enter User ID to clear fine: ";
    cin >> userID;

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

void adminMenu() {
    char choice;
    do {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. Add Book\n";
        cout << "2. Remove Book\n";
        cout << "3. List All Books & Users\n";
        cout << "4. Suspend User\n";
        cout << "5. Unsuspend User\n";
        cout << "6. Clear User Fine\n";
        cout << "7. Exit Admin Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case '1': addBook(); break;
            case '2': removeBook(); break;
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
                break;
            case '4': suspendUser(); break;
            case '5': unsuspendUser(); break;
            case '6': clearFine(); break;
            case '7': cout << "Exiting Admin Menu.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != '7');
}
