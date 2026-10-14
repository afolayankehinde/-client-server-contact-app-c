#include <stdio.h>
#include "contacts.h"
#include <sqlite3.h>

sqlite3 *db; // DEFINE global db here ONCE

int main() {
    // 1. Open DB
    if (sqlite3_open("contacts.db", &db)) {
        printf("Can't open DB: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // 2. Create table with CATEGORY
    const char *sql = "CREATE TABLE IF NOT EXISTS contacts("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT UNIQUE NOT NULL,"
                      "phone TEXT,"
                      "email TEXT,"
                      "category TEXT);";
    sqlite3_exec(db, sql, 0, 0, 0);

    // 3. LOAD DATA FROM DB INTO RAM
    loadContactsFromDB();

    int choice;
    do {
        printf("\n--- Contact Manager V1.0 ---\n");
        printf("1. Add Contact\n");
        printf("2. List All Contacts\n");
        printf("3. Search by Category\n");
        printf("4. Delete Contact\n");
        printf("5. Update Contact\n");
        printf("6. Export to CSV\n");
        printf("7. Show Stats\n");
        printf("8. Start as SERVER\n");
        printf("9. Connect as CLIENT\n");
        printf("10. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice)!= 1) {
            printf("Invalid input! Please enter a number 1-8\n");
            clearInputBuffer();
            choice = 0;
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: addContact(); break;
            case 2: listContactsSorted(); break;
            case 3: searchByCategory(); break;
            case 4: deleteContact(); break;
            case 5: updateContact(); break;
            case 6: {
            pthread_t tid;
    printf("\nExporting in background... Please wait\n");

    pthread_create(&tid, NULL, exportThread, NULL);
    pthread_join(tid, NULL); // WAIT for thread to finish instead of detach

    printf("Export Done! Back to menu.\n");
    break;
}
            case 7: {
    pthread_t tid;
    printf("\nCalculating stats... Please wait\n");

    pthread_create(&tid, NULL, statsThread, NULL);
    pthread_join(tid, NULL); // WAIT for thread to finish

    printf("Stats Done! Back to menu.\n");
    break;
}
            case 8: {
    pthread_t tid;
    printf("[SERVER] Starting in background on port %d...\n", PORT);
    pthread_create(&tid, NULL, serverThread, NULL);
    pthread_detach(tid); // important: detach so it runs
    break;
}
            case 9: startClient(); break;
            case 10: printf("Goodbye!\n"); break;
            default: printf("Invalid choice. Pick 1-8\n");
        }
    } while (choice!= 10);

    sqlite3_close(db);
    return 0;
}
