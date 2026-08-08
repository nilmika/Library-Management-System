# Library Management System

A console-based Library Management System built in C++, using plain structs, global arrays, and functions — no classes, no STL containers, no external libraries.

## Features

**User side**
- Register a new account or log in with an existing User ID
- Recover a forgotten User ID via registered phone number
- Borrow an available book
- Return a borrowed book (with automatic late fines)
- List all currently available books

**Admin side**
- Add or remove books from the catalog (removal is blocked while a book is checked out)
- List all books and all registered users
- Suspend / unsuspend a user's account
- Clear a user's outstanding fine

**Late fines**

Books are due back within 7 days of borrowing:

| Days late | Fine    |
|-----------|---------|
| 1–3 days  | Rs. 20  |
| 4–6 days  | Rs. 50  |
| 7+ days   | Rs. 100 |

## Data persistence

All user and book records are saved to `users.txt` and `books.txt` in a simple pipe-delimited (`|`) format, and reloaded automatically on startup — no database required.

## Getting started

### Option 1: Code::Blocks
Open `main.cbp` in Code::Blocks and build/run (F9).

### Option 2: Command line (g++)
```bash
g++ -std=c++11 -o library_system main.cpp
./library_system
```

### Admin login (demo)
- Username: `admin`
- Password: `admin123`

## Project structure
```
├── main.cpp      # All program logic
├── main.cbp      # Code::Blocks project file
├── users.txt     # Sample/seed user data
├── books.txt     # Sample/seed book data
└── .gitignore
```

## Limitations

- Fixed capacity: up to 100 users and 50 books (defined via `MAX_USERS` / `MAX_BOOKS`)
- Single admin account with a hardcoded username/password, meant for demo purposes only
- No password hashing or encryption on stored data

## License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.
