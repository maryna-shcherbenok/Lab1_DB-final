#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "publishers.h"
#include "struct.h"
#include "file_manager.h"

/* Змінні для індексної таблиці */
PublisherIndex publisherIndexArray[MAX_PUBLISHER];
int publisherIndexCount = 0;

/* Сміттєва зона для master‑записів (видавництва), що зберігається у файлі publishersGarbage.bin */
int publishersGarbage[MAX_PUBLISHER];
int publishersGarbageCount = 0;

/* Змінна для наступного id видавництва */
static int nextPublisherId = 1;

/* Функція для очищення буфера вводу */
static void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

/* Функція для сортування індексної таблиці за зростанням значення ключа (id видавництва) */
static int cmpPublisherIndex(const void *a, const void *b) {
    PublisherIndex *pi = (PublisherIndex *)a;
    PublisherIndex *pb = (PublisherIndex *)b;
    return (pi->key - pb->key);
}

/* Додавання запису до індексної таблиці */
static void addPublisherIndex(int key, int recNo) {
    if (publisherIndexCount >= MAX_PUBLISHER) {
        printf("Publisher index overflow!\n");
        return;
    }
    publisherIndexArray[publisherIndexCount].key = key;
    publisherIndexArray[publisherIndexCount].recNo = recNo;
    publisherIndexCount++;
    qsort(publisherIndexArray, publisherIndexCount, sizeof(PublisherIndex), cmpPublisherIndex);
    savePublisherIndex();
}

/* Видалення запису з індексної таблиці */
static void removePublisherIndex(int key) {
    int i, j;
    for (i = 0; i < publisherIndexCount; i++) {
        if (publisherIndexArray[i].key == key) {
            for (j = i; j < publisherIndexCount - 1; j++) {
                publisherIndexArray[j] = publisherIndexArray[j+1];
            }
            publisherIndexCount--;
            break;
        }
    }
    savePublisherIndex();
}

/* Пошук у індексній таблиці за ключем */
static int findPublisherInIndex(int key) {
    int low = 0, high = publisherIndexCount - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (publisherIndexArray[mid].key == key)
            return publisherIndexArray[mid].recNo;
        if (publisherIndexArray[mid].key < key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

/* Функція пошуку видавництва за id */
int getPublisherById(int id, Publisher *publisher, int *recNo) {
    int r = findPublisherInIndex(id);
    if (r == -1) return 0;
    *recNo = r;
    *publisher = readPublisher(r);
    if (publisher->isDeleted) return 0;
    return 1;
}

/* Завантаження індексної таблиці з файлу publishers.ind */
void loadPublisherIndex() {
    FILE *indFile = fopen("publishers.ind", "rb");
    if (indFile == NULL) {
        publisherIndexCount = 0;
        nextPublisherId = 1; // Якщо індекс відсутній, починаємо з 1
        return;
    }
    fseek(indFile, 0, SEEK_END);
    long size = ftell(indFile);
    rewind(indFile);
    publisherIndexCount = size / sizeof(PublisherIndex);
    fread(publisherIndexArray, sizeof(PublisherIndex), publisherIndexCount, indFile);
    fclose(indFile);

    /* Визначаємо наступний id */
    nextPublisherId = 1;
    for (int i = 0; i < publisherIndexCount; i++) {
        if (publisherIndexArray[i].key >= nextPublisherId) {
            nextPublisherId = publisherIndexArray[i].key + 1;
        }
    }
}

/* Збереження індексної таблиці у файл publishers.ind */
void savePublisherIndex() {
    FILE *indFile = fopen("publishers.ind", "wb");
    if (indFile == NULL) {
        printf("Error opening index file for writing.\n");
        return;
    }
    fwrite(publisherIndexArray, sizeof(PublisherIndex), publisherIndexCount, indFile);
    fclose(indFile);
}

/* Завантаження сміттєвої зони з файлу publishersGarbage.bin */
void loadPublishersGarbage() {
    FILE *garbageFile = fopen("publishersGarbage.bin", "rb");
    if (garbageFile == NULL) {
        publishersGarbageCount = 0;
        return;
    }
    fread(&publishersGarbageCount, sizeof(int), 1, garbageFile);
    fread(publishersGarbage, sizeof(int), publishersGarbageCount, garbageFile);
    fclose(garbageFile);
}

/* Збереження сміттєвої зони у файл publishersGarbage.bin */
void savePublishersGarbage() {
    FILE *garbageFile = fopen("publishersGarbage.bin", "wb");
    if (garbageFile == NULL) {
        printf("Error opening publishersGarbage.bin for writing.\n");
        return;
    }
    fwrite(&publishersGarbageCount, sizeof(int), 1, garbageFile);
    fwrite(publishersGarbage, sizeof(int), publishersGarbageCount, garbageFile);
    fclose(garbageFile);
}

/* Пошук та вивід master‑запису (видавництва) та його підзаписів (книжок) (get-m) */
void get_m() {
    int id;
    printf("Enter publisher ID for search: ");
    scanf("%d", &id);
    getchar();
    Publisher p;
    int recNo;
    if (!getPublisherById(id, &p, &recNo)) {
        printf("Publisher with ID = %d not found.\n", id);
        return;
    }
    printf("Publisher Data\n");
    printf("ID: %d; Name: %s; Phone: %s; Email: %s; Address: %s; Book Count: %d\n",
           p.id, p.name, p.phone, p.email, p.address, p.bookCount);
    printf("First book record: %d\n", p.firstBook);

    int bookRec = p.firstBook;
    if (bookRec == -1) {
        printf("No books found for this publisher.\n");
        return;
    }
    printf("Publisher's Books\n");
    while (bookRec != -1) {
        Book b = readBook(bookRec);
        if (!b.isDeleted) {
            printf("Book ID: %d; Title: %s; ISBN: %s; Genre: %s; Page Count: %d\n",
                   b.id, b.title, b.isbn, b.genre, b.pageCount);
        }
        bookRec = b.nextBook;
    }
}

/* Додавання нового master‑запису (видавництва) (insert-m) */
void insert_m() {
    Publisher p;

    /* Автоматичне присвоєння id */
    p.id = nextPublisherId++;
    printf("Automatically assigned publisher ID: %d\n", p.id);

    /* Очищення буфера вводу */
    clearInputBuffer();

    printf("Enter publisher name: ");
    if (fgets(p.name, NAME_LEN, stdin) == NULL) {
        printf("Invalid input for publisher name. Record cannot be saved.\n");
        return;
    }
    p.name[strcspn(p.name, "\n")] = '\0';

    printf("Enter phone: ");
    if (fgets(p.phone, PHONE_LEN, stdin) == NULL) {
        printf("Invalid input for phone. Record cannot be saved.\n");
        return;
    }
    p.phone[strcspn(p.phone, "\n")] = '\0';

    printf("Enter email: ");
    if (fgets(p.email, EMAIL_LEN, stdin) == NULL) {
        printf("Invalid input for email. Record cannot be saved.\n");
        return;
    }
    p.email[strcspn(p.email, "\n")] = '\0';

    printf("Enter address: ");
    if (fgets(p.address, ADDR_LEN, stdin) == NULL) {
        printf("Invalid input for address. Record cannot be saved.\n");
        return;
    }
    p.address[strcspn(p.address, "\n")] = '\0';

    /* Перевірка на унікальність */
    int recCount = getPublisherRecordCount();
    for (int i = 0; i < recCount; i++) {
        Publisher existing = readPublisher(i);
        if (!existing.isDeleted) {
            if (strcmp(existing.name, p.name) == 0) {
                printf("Error: Publisher name must be unique. Record not added.\n");
                return;
            }
            if (strcmp(existing.phone, p.phone) == 0) {
                printf("Error: Phone number must be unique. Record not added.\n");
                return;
            }
            if (strcmp(existing.email, p.email) == 0) {
                printf("Error: Email must be unique. Record not added.\n");
                return;
            }
        }
    }

    p.firstBook = -1;
    p.bookCount = 0;
    p.isDeleted = 0;

    int recNo;
    if (publishersGarbageCount > 0) {
        recNo = publishersGarbage[--publishersGarbageCount];
        fseek(fpMaster, recNo * sizeof(Publisher), SEEK_SET);
    } else {
        recNo = getPublisherRecordCount();
        fseek(fpMaster, 0, SEEK_END);
    }
    fwrite(&p, sizeof(Publisher), 1, fpMaster);
    fflush(fpMaster);

    addPublisherIndex(p.id, recNo);
    printf("Publisher added at record number %d\n", recNo);
}

/* Оновлення даних master‑запису (видавництва) (update-m) */
void update_m() {
    int pubId;
    printf("Enter publisher ID to update: ");
    if (scanf("%d", &pubId) != 1) {
        printf("Invalid input for publisher ID. Update aborted.\n");
        clearInputBuffer();
        return;
    }
    getchar();

    Publisher p;
    int pRec;
    if (!getPublisherById(pubId, &p, &pRec)) {
        printf("Publisher with ID = %d not found.\n", pubId);
        return;
    }

    /* Можливість вибору поля для змін */
    int choice;
    do {
        printf("Select a field to update:\n");
        printf("1. Name\n");
        printf("2. Phone\n");
        printf("3. Email\n");
        printf("4. Address\n");
        printf("0. Finish update\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input for choice. Update aborted.\n");
            clearInputBuffer();
            return;
        }
        getchar();

        char tempValue[ADDR_LEN];

        switch (choice) {
            case 1:
                printf("Enter new name: ");
                if (fgets(tempValue, NAME_LEN, stdin) == NULL) {
                    printf("Invalid input for name.\n");
                    break;
                }
                tempValue[strcspn(tempValue, "\n")] = '\0';

                /* Перевірка унікальності */
                for (int i = 0; i < getPublisherRecordCount(); i++) {
                    Publisher existing = readPublisher(i);
                    if (!existing.isDeleted && existing.id != p.id && strcmp(existing.name, tempValue) == 0) {
                        printf("Error: Publisher name must be unique. Update aborted.\n");
                        return;
                    }
                }

                strcpy(p.name, tempValue);
                break;

            case 2:
                printf("Enter new phone: ");
                if (fgets(tempValue, PHONE_LEN, stdin) == NULL) {
                    printf("Invalid input for phone.\n");
                    break;
                }
                tempValue[strcspn(tempValue, "\n")] = '\0';

                /* Перевірка унікальності */
                for (int i = 0; i < getPublisherRecordCount(); i++) {
                    Publisher existing = readPublisher(i);
                    if (!existing.isDeleted && existing.id != p.id && strcmp(existing.phone, tempValue) == 0) {
                        printf("Error: Phone number must be unique. Update aborted.\n");
                        return;
                    }
                }

                strcpy(p.phone, tempValue);
                break;

            case 3:
                printf("Enter new email: ");
                if (fgets(tempValue, EMAIL_LEN, stdin) == NULL) {
                    printf("Invalid input for email.\n");
                    break;
                }
                tempValue[strcspn(tempValue, "\n")] = '\0';

                /* Перевірка унікальності */
                for (int i = 0; i < getPublisherRecordCount(); i++) {
                    Publisher existing = readPublisher(i);
                    if (!existing.isDeleted && existing.id != p.id && strcmp(existing.email, tempValue) == 0) {
                        printf("Error: Email must be unique. Update aborted.\n");
                        return;
                    }
                }

                strcpy(p.email, tempValue);
                break;

            case 4:
                printf("Enter new address: ");
                if (fgets(tempValue, ADDR_LEN, stdin) == NULL) {
                    printf("Invalid input for address.\n");
                    break;
                }
                tempValue[strcspn(tempValue, "\n")] = '\0';

                /* Перевірка унікальності */
                for (int i = 0; i < getPublisherRecordCount(); i++) {
                    Publisher existing = readPublisher(i);
                    if (!existing.isDeleted && existing.id != p.id && strcmp(existing.address, tempValue) == 0) {
                        printf("Error: Address must be unique. Update aborted.\n");
                        return;
                    }
                }

                strcpy(p.address, tempValue);
                break;

            case 0:
                break;

            default:
                printf("Incorrect option number. Try again.\n");
        }
    } while (choice != 0);

    writePublisher(pRec, &p);
    printf("Publisher data updated.\n");
}

/* Логічне видалення master‑запису (видавництва) та всіх його підзаписів (книжок) (del-m) */
void del_m() {
    int pubId;
    printf("Enter publisher ID to delete: ");
    scanf("%d", &pubId);
    getchar();
    Publisher p;
    int pRec;
    if (!getPublisherById(pubId, &p, &pRec)) {
        printf("Publisher with ID = %d not found.\n", pubId);
        return;
    }
    p.isDeleted = 1;
    writePublisher(pRec, &p);
    publishersGarbage[publishersGarbageCount++] = pRec;
    removePublisherIndex(pubId);

    int bookRec = p.firstBook;
    while (bookRec != -1) {
        Book b = readBook(bookRec);
        if (!b.isDeleted) {
            b.isDeleted = 1;
            writeBook(bookRec, &b);
            extern int booksGarbage[];
            extern int booksGarbageCount;
            booksGarbage[booksGarbageCount++] = bookRec;
        }
        bookRec = b.nextBook;
    }
    printf("Publisher and all their books have been logically deleted.\n");
}

/* Підрахунок активних master‑записів (видавництв) (calc-m) */
void calc_m() {
    int total = 0;
    int recCount = getPublisherRecordCount();
    for (int i = 0; i < recCount; i++) {
        Publisher p = readPublisher(i);
        if (!p.isDeleted)
            total++;
    }
    printf("Active publishers: %d\n", total);
}

/* Вивід усіх записів master‑файлу (publishers.fl) (ut-m) */
void ut_m() {
    int recCount = getPublisherRecordCount();
    printf("Content of file publishers.fl\n");
    for (int i = 0; i < recCount; i++) {
        Publisher p = readPublisher(i);
        printf("Record %d: ID = %d; Name = %s; Phone = %s; Email = %s; Address = %s; Book Count: %d; isDeleted: %d\n",
               i, p.id, p.name, p.phone, p.email, p.address, p.bookCount, p.isDeleted);
    }
}

/*
   Функція фізичної реорганізації файлу видавництв:
   - Копіює активні записи з файлу publishers.fl у тимчасовий файл.
   - Формує мапінг старих індексів до нових.
   - Замінює старий файл новим, скидає сміттєву зону.
   - Перебудовує індексну таблицю з урахуванням нових позицій записів.
*/
void reorganizePublishers() {
    int totalRecords = getPublisherRecordCount();
    if (totalRecords == 0) {
        printf("No publisher records to reorganize.\n");
        return;
    }

    /* Виділяємо пам'ять для мапінгу старих індексів до нових */
    int *mapping = (int *)malloc(totalRecords * sizeof(int));
    if (!mapping) {
        printf("Memory allocation error during publisher reorganization.\n");
        return;
    }
    for (int i = 0; i < totalRecords; i++) {
        mapping[i] = -1;
    }

    /* Відкриваємо тимчасовий файл для запису активних записів */
    FILE *tempFile = fopen("publishers_temp.fl", "wb+");
    if (!tempFile) {
        printf("Error opening temporary file for publisher reorganization.\n");
        free(mapping);
        return;
    }

    int newIndex = 0;
    /* Копіюємо активні записи та формуємо мапінг */
    for (int i = 0; i < totalRecords; i++) {
        Publisher p = readPublisher(i);
        if (!p.isDeleted) {
            mapping[i] = newIndex;
            fwrite(&p, sizeof(Publisher), 1, tempFile);
            newIndex++;
        }
    }
    fclose(tempFile);

    /* Закриваємо поточний файл publishers.fl */
    if (fpMaster) {
        fclose(fpMaster);
    }

    /* Замінюємо старий файл новим */
    remove("publishers.fl");
    if (rename("publishers_temp.fl", "publishers.fl") != 0) {
        printf("Error renaming temporary file to publishers.fl.\n");
        free(mapping);
        return;
    }

    /* Перевідкриваємо fpMaster з оновленим файлом */
    fpMaster = fopen("publishers.fl", "rb+");
    if (!fpMaster) {
        printf("Error reopening publishers.fl after reorganization.\n");
        free(mapping);
        return;
    }

    /* Скидаємо сміттєву зону */
    publishersGarbageCount = 0;
    savePublishersGarbage();

    /* Перебудовуємо індексну таблицю: очищуємо глобальні дані та проходимо по всіх записах нового файлу */
    publisherIndexCount = 0;
    int newTotalRecords = getPublisherRecordCount();
    for (int i = 0; i < newTotalRecords; i++) {
        Publisher p = readPublisher(i);
        /* Додаємо кожен активний запис до індексної таблиці */
        addPublisherIndex(p.id, i);
    }

    free(mapping);
    printf("Publishers file reorganized successfully. Active records: %d\n", newIndex);
}
