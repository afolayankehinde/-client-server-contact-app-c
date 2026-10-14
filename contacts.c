#include "contacts.h"

// === GLOBAL VARIABLES - DEFINED HERE ONCE ===
// NOTE: db is defined in main.c, so we use extern here
extern sqlite3 *db;
Contact *hashTable[TABLE_SIZE] = {NULL};
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void clearHashTable() {
    for(int i=0; i<TABLE_SIZE; i++) {
        Contact *curr = hashTable[i];
        while(curr) {
            Contact *tmp = curr;
            curr = curr->next;
            free(tmp);
        }
        hashTable[i] = NULL;
    }
}

// === HASH FUNCTION ===
unsigned int hash(char *name) {
    unsigned long hash = 5381;
    int c;
    while ((c = *name++))
        hash = ((hash << 5) + hash) + c;
    return hash % TABLE_SIZE;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar())!= '\n' && c!= EOF);
}

// === DB HELPERS ===
int execPrepared(sqlite3 *db, const char *sql, Contact *c) {
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, 0)!= SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, c->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, c->category, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int updatePrepared(sqlite3 *db, Contact *c) {
    sqlite3_stmt *stmt;
    char *sql = "UPDATE contacts SET name=?, phone=?, email=?, category=? WHERE id=?";
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, 0)!= SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, c->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, c->phone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, c->email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, c->category, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, c->id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// === LOAD FROM DB TO HASH TABLE ===
void loadContactsFromDB() {
    // REMOVED pthread_mutex_lock
    char *sql = "SELECT * FROM contacts";
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while(sqlite3_step(stmt) == SQLITE_ROW) {
            Contact *c = malloc(sizeof(Contact));
            c->id = sqlite3_column_int(stmt, 0);
            strcpy(c->name, (char*)sqlite3_column_text(stmt, 1));
            strcpy(c->phone, (char*)sqlite3_column_text(stmt, 2));
            strcpy(c->email, (char*)sqlite3_column_text(stmt, 3));
            strcpy(c->category, (char*)sqlite3_column_text(stmt, 4));
            unsigned int index = hash(c->name);
            c->next = hashTable[index];
            hashTable[index] = c;
        }
    }
    sqlite3_finalize(stmt);
    // REMOVED pthread_mutex_unlock
}
// === LOCAL FUNCTIONS ===
void addContact() {
    Contact c;
    printf("Enter Name: "); fgets(c.name, 50, stdin); c.name[strcspn(c.name, "\n")] = 0;
    printf("Enter Phone: "); fgets(c.phone, 20, stdin); c.phone[strcspn(c.phone, "\n")] = 0;
    printf("Enter Email: "); fgets(c.email, 50, stdin); c.email[strcspn(c.email, "\n")] = 0;
    printf("Enter Category: "); fgets(c.category, 20, stdin); c.category[strcspn(c.category, "\n")] = 0;

    pthread_mutex_lock(&lock);
    char *sql = "INSERT INTO contacts(name,phone,email,category) VALUES(?,?,?,?)";
    if(execPrepared(db, sql, &c)) {
        c.id = sqlite3_last_insert_rowid(db);
        unsigned int index = hash(c.name);
        Contact *newNode = malloc(sizeof(Contact));
        *newNode = c;
        newNode->next = hashTable[index];
        hashTable[index] = newNode;
        printf("Contact added!\n");
    } else {
        printf("Error: Name might already exist\n");
    }
    pthread_mutex_unlock(&lock);
}

void listContactsSorted() {
    pthread_mutex_lock(&lock);
    clearHashTable(); // wipe RAM
    loadContactsFromDB(); // reload from DB into RAM

    int found = 0;
    printf("\n--- CONTACT LIST ---\n");
    for(int i = 0; i < TABLE_SIZE; i++) {
        Contact *curr = hashTable[i];
        while(curr) {
            printf("%d. %s | %s | %s | %s\n", curr->id, curr->name, curr->phone, curr->email, curr->category);
            found = 1;
            curr = curr->next;
        }
    }
    if(!found) printf("No contacts found\n");
    printf("--------------------\n");
    pthread_mutex_unlock(&lock);
}

void searchByCategory() {
    char cat[20];
    printf("Enter Category to search: ");
    fgets(cat, 20, stdin); cat[strcspn(cat, "\n")] = 0;

    pthread_mutex_lock(&lock);
    int found = 0;
    for(int i = 0; i < TABLE_SIZE; i++) {
        Contact *curr = hashTable[i];
        while(curr) {
            if(_stricmp(curr->category, cat) == 0) { // _stricmp for windows
                printf("%d. %s | %s | %s | %s\n", curr->id, curr->name, curr->phone, curr->email, curr->category);
                found = 1;
            }
            curr = curr->next;
        }
    }
    if(!found) printf("No contacts in category '%s'\n", cat);
    pthread_mutex_unlock(&lock);
}

void deleteContact() {
    int id;
    printf("Enter ID to delete: ");
    scanf("%d", &id);
    clearInputBuffer();

    pthread_mutex_lock(&lock);
    char *sql = "DELETE FROM contacts WHERE id=?";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, id);
    if(sqlite3_step(stmt) == SQLITE_DONE) printf("Contact deleted\n");
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&lock);

    // TODO: Also remove from hashTable. For now just reload
    loadContactsFromDB();
}

void updateContact() {
    Contact c;
    printf("Enter ID to update: ");
    scanf("%d", &c.id);
    clearInputBuffer();
    printf("Enter New Name: "); fgets(c.name, 50, stdin); c.name[strcspn(c.name, "\n")] = 0;
    printf("Enter New Phone: "); fgets(c.phone, 20, stdin); c.phone[strcspn(c.phone, "\n")] = 0;
    printf("Enter New Email: "); fgets(c.email, 50, stdin); c.email[strcspn(c.email, "\n")] = 0;
    printf("Enter New Category: "); fgets(c.category, 20, stdin); c.category[strcspn(c.category, "\n")] = 0;

    pthread_mutex_lock(&lock);
    if(updatePrepared(db, &c)) printf("Contact updated\n");
    pthread_mutex_unlock(&lock);
}

void showStats() {
    pthread_mutex_lock(&lock);
    int count = 0;
    for(int i = 0; i < TABLE_SIZE; i++) {
        Contact *curr = hashTable[i];
        while(curr) { count++; curr = curr->next; }
    }
    printf("Total Contacts: %d\n", count);
    pthread_mutex_unlock(&lock);
}

void exportToCSV() {
    FILE *fp = fopen("contacts.csv", "w");
    fprintf(fp, "ID,Name,Phone,Email,Category\n");
    pthread_mutex_lock(&lock);
    for(int i = 0; i < TABLE_SIZE; i++) {
        Contact *curr = hashTable[i];
        while(curr) {
            fprintf(fp, "%d,%s,%s,%s,%s\n", curr->id, curr->name, curr->phone, curr->email, curr->category);
            curr = curr->next;
        }
    }
    pthread_mutex_unlock(&lock);
    fclose(fp);
    printf("Exported to contacts.csv\n");
}

void* exportThread(void *arg) { exportToCSV(); return NULL; }
void* statsThread(void *arg) { showStats(); return NULL; }

// === NETWORK FUNCTIONS ===
void *handleClient(void *arg) {
    SOCKET client_socket = *(SOCKET*)arg;
    free(arg);

    char buffer[4096] = {0};
    int bytes = recv(client_socket, buffer, 4096, 0);

    if(bytes <= 0) {
        printf("[SERVER] Client disconnected or sent nothing\n");
        closesocket(client_socket);
        return NULL;
    }

    buffer[bytes] = '\0';
    printf("[SERVER] Received: %s\n", buffer);

    if(strncmp(buffer, "LIST", 4) == 0) {
        pthread_mutex_lock(&lock);
        clearHashTable();
        loadContactsFromDB();

        char response[4096] = "";
        for(int i = 0; i < TABLE_SIZE; i++) {
            Contact *curr = hashTable[i];
            while(curr) {
                char line[256];
                sprintf(line, "%d. %s | %s | %s | %s\n",
                    curr->id, curr->name, curr->phone, curr->email, curr->category);
                strcat(response, line);
                curr = curr->next;
            }
        }
        if(strlen(response) == 0) strcpy(response, "No contacts found\n");

        send(client_socket, response, strlen(response), 0); // send it
        printf("[SERVER] Sent %d bytes back to client\n", (int)strlen(response));
        pthread_mutex_unlock(&lock);
    }
    closesocket(client_socket);
    printf("[SERVER] Client connection closed\n");
    return NULL;
}

void startServer() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);
    printf("[SERVER] Running on port %d. Press Ctrl+C to stop\n", PORT);

    while(1) {
        SOCKET *client_sock = malloc(sizeof(SOCKET));
        *client_sock = accept(server_fd, NULL, NULL);
        printf("[SERVER] Client Connected!\n");

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleClient, client_sock);
        pthread_detach(thread_id);
    }
    closesocket(server_fd);
    WSACleanup();
}

void startClient() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "LIST";
    char buffer[4096] = {0};

    char server_ip[20];
    printf("Enter Server IP: ");
    scanf("%s", server_ip);
    clearInputBuffer();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);

    printf("[CLIENT] Connecting to %s:%d...\n", server_ip, PORT);
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        WSACleanup();
        return;
    }
    printf("[CLIENT] Connected! Sending LIST request...\n");

    send(sock, message, strlen(message), 0);
    int bytes = recv(sock, buffer, 4096, 0);

    if(bytes > 0) {
        buffer[bytes] = '\0';
        printf("\n--- SERVER CONTACT LIST ---\n%s---------------------------\n", buffer);
    } else {
        printf("[CLIENT] No data received from server\n");
    }

    closesocket(sock);
    WSACleanup();
}

void* serverThread(void *arg) {
    startServer(); // this has the while(1) loop
    return NULL;
}
