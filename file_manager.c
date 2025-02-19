#include <stdio.h>
#include <stdlib.h>
#include "file_manager.h"

// Глобальні файлові дескриптори для master (publishers.fl) та slave (books.fl)
FILE *fpMaster = NULL;  // це publishers.fl
FILE *fpSlave = NULL;   // це books.fl

/*
 * Пояснення:
 * Після фізичної реорганізації файлів (функції reorganizeBooks та reorganizePublishers)
 * записи у файлах записані послідовно, тому використання виразу
 *     recNo * sizeof(Record)
 * для обчислення позиції запису є коректним.
 */

int getPublisherRecordCount() {
    if (fpMaster == NULL) return 0;
    fseek(fpMaster, 0, SEEK_END);
    long size = ftell(fpMaster);
    return (int)(size / sizeof(Publisher));
}

int getBookRecordCount() {
    if (fpSlave == NULL) return 0;
    fseek(fpSlave, 0, SEEK_END);
    long size = ftell(fpSlave);
    return (int)(size / sizeof(Book));
}

Publisher readPublisher(int recNo) {
    Publisher p;
    if (fpMaster == NULL) {
        fprintf(stderr, "Error: fpMaster is not open.\n");
        exit(1);
    }
    fseek(fpMaster, recNo * sizeof(Publisher), SEEK_SET);
    if (fread(&p, sizeof(Publisher), 1, fpMaster) != 1) {
        fprintf(stderr, "Error reading publisher record %d.\n", recNo);
    }
    return p;
}

void writePublisher(int recNo, Publisher *p) {
    if (fpMaster == NULL) {
        fprintf(stderr, "Error: fpMaster is not open.\n");
        exit(1);
    }
    fseek(fpMaster, recNo * sizeof(Publisher), SEEK_SET);
    if (fwrite(p, sizeof(Publisher), 1, fpMaster) != 1) {
        fprintf(stderr, "Error writing publisher record %d.\n", recNo);
    }
    fflush(fpMaster);
}

Book readBook(int recNo) {
    Book b;
    if (fpSlave == NULL) {
        fprintf(stderr, "Error: fpSlave is not open.\n");
        exit(1);
    }
    fseek(fpSlave, recNo * sizeof(Book), SEEK_SET);
    if (fread(&b, sizeof(Book), 1, fpSlave) != 1) {
        fprintf(stderr, "Error reading book record %d.\n", recNo);
    }
    return b;
}

void writeBook(int recNo, Book *b) {
    if (fpSlave == NULL) {
        fprintf(stderr, "Error: fpSlave is not open.\n");
        exit(1);
    }
    fseek(fpSlave, recNo * sizeof(Book), SEEK_SET);
    if (fwrite(b, sizeof(Book), 1, fpSlave) != 1) {
        fprintf(stderr, "Error writing book record %d.\n", recNo);
    }
    fflush(fpSlave);
}
