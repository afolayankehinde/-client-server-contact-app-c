#ifndef CONTACTS_H
#define CONTACTS_H

// === Standard Libraries ===
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sqlite3.h>

// === Windows Sockets ===
#include <winsock2.h>
#include <ws2tcpip.h> // for inet_pton



#define PORT 9090
#define TABLE_SIZE 100

// === CONTACT STRUCT ===
typedef struct Contact {
    int id;
    char name[50];
    char phone[20];
    char email[50];
    char category[20];
    struct Contact *next;
} Contact;

// === GLOBAL VARIABLES ===
extern Contact *hashTable[TABLE_SIZE];
extern sqlite3 *db;
extern pthread_mutex_t lock;

// === LOCAL DB FUNCTIONS ===
void addContact();
void listContactsSorted();
void showStats();
void exportToCSV();
void loadContactsFromDB();
void clearInputBuffer();
void searchByCategory();
void deleteContact();
void updateContact();
int execPrepared(sqlite3 *db, const char *sql, Contact *c);
int updatePrepared(sqlite3 *db, Contact *c);
void* exportThread(void *arg);
void* statsThread(void *arg);
void clearHashTable(); // Clear hash table from memory

// === NETWORK FUNCTIONS ===
void startServer(); // Runs on PC 1
void startClient(); // Runs on PC 2
void* handleClient(void *arg); // Server handles requests in a thread
void* serverThread(void *arg); // Server runs in background thread

#endif // CONTACTS_H
