# -client-server-contact-app-c

Contact Management System in C using Sockets, Pthreads, SQLite - built and debugged with GDB

## Features
- Add, list (sorted), search by category, update and delete contacts
- Contacts are stored in an SQLite database
- Export contacts to CSV and show stats, each run in its own thread
- Start a server in the background, or connect to one as a client

## Project files
- `main.c`: the menu and program start
- `contacts.c` and `contacts.h`: the contact functions, the database code, the threads and the network code
- `PROS.cbp`: the Code::Blocks project file

## How to build
1. Download the SQLite amalgamation (`sqlite3.c` and `sqlite3.h`) from sqlite.org and put both files in the same folder as the source files.
2. Open `PROS.cbp` in Code::Blocks and press F9 to build and run.

## Notes
- The contacts database (`contacts.db`) is not included. The program creates it the first time it runs.

## Known limitations
- If the server is still running when you choose Exit, the program closes the database while the server thread may still be using it.
