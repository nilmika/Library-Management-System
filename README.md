# Library Management System

A console-based Library Management System built in C++, using plain structs, global arrays, and functions — no classes, no STL containers, no external libraries.

## Features

**User side**
- Register a new account (auto-assigned User ID, e.g. `U006`) with a 4-digit PIN
- Log in with User ID + PIN
- Recover a forgotten User ID via registered phone number
- Borrow an available book
- Return a borrowed book (with automatic late fines)
- List all currently available books
- Search books by title or author keyword (case-insensitive, partial match)
- View your own borrow history (past and current borrows)
- Automatic alert on login if you have an overdue book or an unpaid fine

**Admin side**
- Add or remove books from the catalog (removal is blocked while a book is checked out)
- List all books and all registered users
- Suspend / unsuspend a user's account
- Clear a user's outstanding fine
- View any user's full borrow history

**Account security**
- Every account is protected by a 4-digit PIN set at registration
- 3 incorrect PIN attempts in a row automatically suspends the account (admin can unsuspend it)
- A suspended account can't log in at all, not just borrow

**Late fines**

Books are due back within 7 days of borrowing:

| Days late | Fine    |
|-----------|---------|
| 1–3 days  | Rs. 20  |
| 4–6 days  | Rs. 50  |
| 7+ days   | Rs. 100 |

## Data persistence

Records are saved to `users.txt`, `books.txt`, and `history.txt` in a simple pipe-delimited (`|`) format, and reloaded automatically on startup — no database required.

## Getting started

### Option 1: Code::Blocks
Open `main.cbp` in Code::Blocks and build/run (F9).

### Option 2: Command line (g++)
```bash
g++ -std=c++11 -o library_system main.cpp
./library_system
```

### Demo logins
- Admin — Username: `admin`, Password: `admin123`
- Sample users (U001–U005) — PIN: `1234`

## Project structure
```
├── main.cpp      # All program logic
├── main.cbp      # Code::Blocks project file
├── users.txt     # Sample/seed user data
├── books.txt     # Sample/seed book data
├── history.txt   # Sample/seed borrow history
└── .gitignore
```

## Limitations

- Fixed capacity: up to 100 users, 50 books, and 500 history records (`MAX_USERS` / `MAX_BOOKS` / `MAX_HISTORY`)
- Single admin account with a hardcoded username/password, meant for demo purposes only
- PINs and the admin password are stored in plain text, not hashed — fine for a class project, not for real use

## License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.
