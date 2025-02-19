#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "books.h"
#include "struct.h"
#include "file_manager.h"
#include "publishers.h"

/* Сміттєва зона для книг (зберігається booksGarbage.bin) */
int booksGarbage[MAX_BOOKS];
int booksGarbageCount = 0;

/* Змінна для наступного id книги */
static int nextBookId = 1;

/* Завантаження значення nextBookId з файлу nextBookId.bin */
void loadNextBookId() {
    FILE *file = fopen("nextBookId.bin", "rb");
    if (file) {
        if (fread(&nextBookId, sizeof(int), 1, file) != 1) {
            nextBookId = 1;
        }
        fclose(file);
    } else {
        nextBookId = 1;  // Якщо файлу не існує, починаємо id з 1
    }
}

/* Збереження значення nextBookId у файл nextBookId.bin */
void saveNextBookId() {
    FILE *file = fopen("nextBookId.bin", "wb");
    if (file) {
        fwrite(&nextBookId, sizeof(int), 1, file);
        fclose(file);
    } else {
        printf("Error opening nextBookId.bin for writing.\n");
    }
}

/* Завантаження сміттєвої зони з файлу booksGarbage.bin */
void loadBooksGarbage() {
    FILE *garbageFile = fopen("booksGarbage.bin", "rb");
    if (garbageFile == NULL) {
        booksGarbageCount = 0;
        return;
    }
    fread(&booksGarbageCount, sizeof(int), 1, garbageFile);
    fread(booksGarbage, sizeof(int), booksGarbageCount, garbageFile);
    fclose(garbageFile);
}

/* Збереження сміттєвої зони у файл booksGarbage.bin */
void saveBooksGarbage() {
    FILE *garbageFile = fopen("booksGarbage.bin", "wb");
    if (garbageFile == NULL) {
        printf("Error opening booksGarbage.bin for writing.\n");
        return;
    }
    fwrite(&booksGarbageCount, sizeof(int), 1, garbageFile);
    fwrite(booksGarbage, sizeof(int), booksGarbageCount, garbageFile);
    fclose(garbageFile);
}

/* Пошук конкретної книги для конкретного видавництва (get-s) */
void get_s() {
    int pubId, bookId;
    printf("Enter publisher ID: ");
    if (scanf("%d", &pubId) != 1) {
        printf("Invalid input for publisher ID.\n");
        return;
    }
    getchar();
    Publisher p;
    int pRec;
    if (!getPublisherById(pubId, &p, &pRec)) {
        printf("Publisher with ID = %d not found.\n", pubId);
        return;
    }
    printf("Enter book ID to search: ");
    if (scanf("%d", &bookId) != 1) {
        printf("Invalid input for book ID.\n");
        return;
    }
    getchar();

    int bookRec = p.firstBook;
    int found = 0;
    while (bookRec != -1) {
        Book b = readBook(bookRec);
        if (!b.isDeleted && b.id == bookId) {
            printf("Found book:\n");
            printf("ID: %d; Title: %s; ISBN: %s; Genre: %s; Page Count: %d\n",
                   b.id, b.title, b.isbn, b.genre, b.pageCount);
            found = 1;
            break;
        }
        bookRec = b.nextBook;
    }
    if (!found)
        printf("Book with ID = %d not found for publisher with ID = %d.\n", bookId, pubId);
}

/* Додавання нового підзапису (книжки) для конкретного видавництва (insert-s) */
void insert_s() {
    int pubId;
    printf("Enter publisher id for which to add a book: ");
    if (scanf("%d", &pubId) != 1) {
        printf("Invalid input for publisher ID. Record cannot be saved.\n");
        return;
    }
    getchar();

    Publisher p;
    int pRec;
    if (!getPublisherById(pubId, &p, &pRec)) {
        printf("Publisher with ID = %d not found.\n", pubId);
        return;
    }

    Book b;

    /* Автоматичне присвоєння id для книги */
    b.id = nextBookId;
    nextBookId++;
    saveNextBookId();  // Зберігаємо оновлене значення nextBookId

    printf("Automatically assigned book ID: %d\n", b.id);

    printf("Enter book title: ");
    if (fgets(b.title, TITLE_LEN, stdin) == NULL) {
        printf("Invalid input for book title. Record cannot be saved.\n");
        return;
    }
    b.title[strcspn(b.title, "\n")] = '\0';

    b.publisherId = pubId;

    printf("Enter ISBN: ");
    if (fgets(b.isbn, ISBN_LEN, stdin) == NULL) {
        printf("Invalid input for ISBN. Record cannot be saved.\n");
        return;
    }
    b.isbn[strcspn(b.isbn, "\n")] = '\0';

    /* Перевірка унікальності ISBN перед додаванням */
    int totalBooks = getBookRecordCount();
    for (int i = 0; i < totalBooks; i++) {
        Book existingBook = readBook(i);
        if (!existingBook.isDeleted && strcmp(existingBook.isbn, b.isbn) == 0) {
            printf("Error: ISBN must be unique. Record cannot be saved.\n");
            return;
        }
    }

    printf("Enter genre: ");
    if (fgets(b.genre, GENRE_LEN, stdin) == NULL) {
        printf("Invalid input for genre. Record cannot be saved.\n");
        return;
    }
    b.genre[strcspn(b.genre, "\n")] = '\0';

    printf("Enter page count: ");
    if (scanf("%d", &b.pageCount) != 1) {
        printf("Invalid input for page count. Record cannot be saved.\n");
        return;
    }
    getchar();

    b.isDeleted = 0;
    b.nextBook = p.firstBook;
    int bookRec;

    if (booksGarbageCount > 0) {
        bookRec = booksGarbage[--booksGarbageCount];
        fseek(fpSlave, bookRec * sizeof(Book), SEEK_SET);
        printf("Using freed record number %d from garbage.\n", bookRec);
    } else {
        bookRec = getBookRecordCount();
        fseek(fpSlave, 0, SEEK_END);
    }

    fwrite(&b, sizeof(Book), 1, fpSlave);
    fflush(fpSlave);

    p.firstBook = bookRec;
    p.bookCount++;
    writePublisher(pRec, &p);
    printf("Book added at record number %d for publisher ID = %d\n", bookRec, pubId);
}

/* Оновлення даних підзапису (книжки) (update-s) */
void update_s() {
    int pubId, bookId;
    printf("Enter publisher ID for updating a book: ");
    if (scanf("%d", &pubId) != 1) {
        printf("Invalid input for publisher ID. Update aborted.\n");
        return;
    }
    getchar();

    Publisher p;
    int pRec;
    if (!getPublisherById(pubId, &p, &pRec)) {
        printf("Publisher with ID = %d not found.\n", pubId);
        return;
    }

    printf("Enter book ID to update: ");
    if (scanf("%d", &bookId) != 1) {
        printf("Invalid input for book ID. Update aborted.\n");
        return;
    }
    getchar();

    int currRec = p.firstBook;
    int found = 0;
    while (currRec != -1) {
        Book b = readBook(currRec);
        if (!b.isDeleted && b.id == bookId) {
            found = 1;

            /* Можливість вибору конкретного поля для оновлення */
            int choice;
            do {
                printf("Select a field to update for the book:\n");
                printf("1. Title\n");
                printf("2. ISBN\n");
                printf("3. Genre\n");
                printf("4. Page Count\n");
                printf("0. Finish update\n");
                printf("Enter your choice: ");
                if (scanf("%d", &choice) != 1) {
                    printf("Invalid input for choice. Update aborted.\n");
                    return;
                }
                getchar();

                switch (choice) {
                    case 1:
                        printf("Enter new title: ");
                        if (fgets(b.title, TITLE_LEN, stdin) == NULL) {
                            printf("Invalid input for title.\n");
                        } else {
                            b.title[strcspn(b.title, "\n")] = '\0';
                        }
                        break;
                    case 2:
                        printf("Enter new ISBN: ");
                        if (fgets(b.isbn, ISBN_LEN, stdin) == NULL) {
                            printf("Invalid input for ISBN.\n");
                        } else {
                            b.isbn[strcspn(b.isbn, "\n")] = '\0';

                            /* Перевірка унікальності нового ISBN */
                            int totalBooks = getBookRecordCount();
                            for (int i = 0; i < totalBooks; i++) {
                                Book existingBook = readBook(i);
                                if (!existingBook.isDeleted && strcmp(existingBook.isbn, b.isbn) == 0 && existingBook.id != b.id) {
                                    printf("Error: ISBN must be unique. Update aborted.\n");
                                    return;
                                }
                            }
                        }
                        break;
                    case 3:
                        printf("Enter new genre: ");
                        if (fgets(b.genre, GENRE_LEN, stdin) == NULL) {
                            printf("Invalid input for genre.\n");
                        } else {
                            b.genre[strcspn(b.genre, "\n")] = '\0';
                        }
                        break;
                    case 4:
                        printf("Enter new page count: ");
                        if (scanf("%d", &b.pageCount) != 1) {
                            printf("Invalid input for page count.\n");
                        }
                        getchar();
                        break;
                    case 0:
                        break;
                    default:
                        printf("Incorrect option number. Try again.\n");
                }
            } while (choice != 0);

            writeBook(currRec, &b);
            printf("Book data updated.\n");
            break;
        }
        currRec = b.nextBook;
    }

    if (!found) {
        printf("Book with ID = %d not found for publisher with ID = %d.\n", bookId, pubId);
    }
}

/* Логічне видалення підзапису (книжки) (del-s) */
void del_s() {
    int pubId, bookId;
    printf("Enter publisher ID: ");
    if (scanf("%d", &pubId) != 1) {
        printf("Invalid input for publisher ID. Deletion aborted.\n");
        return;
    }
    getchar();
    Publisher p;
    int pRec;
    if (!getPublisherById(pubId, &p, &pRec)) {
        printf("Publisher with ID = %d not found.\n", pubId);
        return;
    }
    printf("Enter book ID to delete: ");
    if (scanf("%d", &bookId) != 1) {
        printf("Invalid input for book ID. Deletion aborted.\n");
        return;
    }
    getchar();

    int currRec = p.firstBook;
    int prevRec = -1;
    int found = 0;
    while (currRec != -1) {
        Book b = readBook(currRec);
        if (!b.isDeleted && b.id == bookId) {
            b.isDeleted = 1;
            writeBook(currRec, &b);
            booksGarbage[booksGarbageCount++] = currRec;
            printf("Added record number %d to booksGarbage. Total garbage count: %d\n", currRec, booksGarbageCount);
            if (prevRec == -1) {
                p.firstBook = b.nextBook;
            } else {
                Book prevBook = readBook(prevRec);
                prevBook.nextBook = b.nextBook;
                writeBook(prevRec, &prevBook);
            }
            p.bookCount--;
            writePublisher(pRec, &p);
            printf("Book deleted.\n");
            found = 1;
            break;
        }
        prevRec = currRec;
        currRec = b.nextBook;
    }
    if (!found)
        printf("Book with ID = %d not found for publisher with ID = %d.\n", bookId, pubId);
}

/* Підрахунок активних (не видалених) підзаписів (книжок) (calc-s) */
void calc_s() {
    int total = 0;
    int recCount = getBookRecordCount();
    for (int i = 0; i < recCount; i++) {
        Book b = readBook(i);
        if (!b.isDeleted)
            total++;
    }
    printf("Active books in total: %d\n", total);
    printf("Book count per publisher (according to bookCount field):\n");

    extern PublisherIndex publisherIndexArray[];
    extern int publisherIndexCount;
    for (int i = 0; i < publisherIndexCount; i++) {
        Publisher p = readPublisher(publisherIndexArray[i].recNo);
        if (!p.isDeleted)
            printf("Publisher ID = %d; Name = %s; Books Count: %d\n", p.id, p.name, p.bookCount);
    }
}

/* Вивід усіх записів з файлу books.fl (ut-s) */
void ut_s() {
    int recCount = getBookRecordCount();
    printf("Content of file books.fl\n");
    for (int i = 0; i < recCount; i++) {
        Book b = readBook(i);
        printf("Record %d: ID = %d; Title: %s; Publisher ID: %d; ISBN: %s; Genre: %s; Page Count: %d; isDeleted: %d\n",
               i, b.id, b.title, b.publisherId, b.isbn, b.genre, b.pageCount, b.isDeleted);
    }
}

/*
   Функція фізичної реорганізації файлу книг:
   - Копіює всі активні записи у тимчасовий файл.
   - Формує мапінг старих індексів до нових та оновлює поле nextBook.
   - Замінює старий файл на новий та скидає сміттєву зону.
   - Оновлює поле firstBook у записах видавництв згідно з новими індексами.
*/
void reorganizeBooks() {
    int totalRecords = getBookRecordCount();
    if (totalRecords == 0) {
        printf("No records to reorganize.\n");
        return;
    }

    /* Виділяємо пам'ять для мапінгу: для кожного старого індексу зберігатимемо новий */
    int *mapping = (int *)malloc(totalRecords * sizeof(int));
    if (!mapping) {
        printf("Memory allocation error during reorganization.\n");
        return;
    }
    for (int i = 0; i < totalRecords; i++) {
        mapping[i] = -1;
    }

    /* Відкриваємо тимчасовий файл для запису активних записів */
    FILE *tempFile = fopen("books_temp.fl", "wb+");
    if (!tempFile) {
        printf("Error opening temporary file for reorganization.\n");
        free(mapping);
        return;
    }

    int newIndex = 0;
    /* Копіюємо активні записи до тимчасового файлу та формуємо мапінг */
    for (int i = 0; i < totalRecords; i++) {
        Book b = readBook(i);
        if (!b.isDeleted) {
            mapping[i] = newIndex;
            fwrite(&b, sizeof(Book), 1, tempFile);
            newIndex++;
        }
    }

    /* Оновлюємо поле nextBook для записів у тимчасовому файлі */
    fseek(tempFile, 0, SEEK_SET);
    for (int i = 0; i < newIndex; i++) {
        long pos = ftell(tempFile);
        Book b;
        fread(&b, sizeof(Book), 1, tempFile);
        if (b.nextBook != -1) {
            b.nextBook = mapping[b.nextBook];
        }
        fseek(tempFile, pos, SEEK_SET);
        fwrite(&b, sizeof(Book), 1, tempFile);
        fflush(tempFile);
        fseek(tempFile, (i + 1) * sizeof(Book), SEEK_SET);
    }
    fclose(tempFile);

    /* Закриваємо поточний файл книг, якщо він відкритий */
    if (fpSlave) {
        fclose(fpSlave);
    }

    /* Замінюємо старий файл книг новим */
    remove("books.fl");
    if (rename("books_temp.fl", "books.fl") != 0) {
        printf("Error renaming temporary file to books.fl.\n");
        free(mapping);
        return;
    }

    /* Перевідкриваємо fpSlave з оновленим файлом */
    fpSlave = fopen("books.fl", "rb+");
    if (!fpSlave) {
        printf("Error reopening books.fl after reorganization.\n");
        free(mapping);
        return;
    }

    /* Оновлюємо сміттєву зону, оскільки тепер файл містить лише активні записи */
    booksGarbageCount = 0;
    saveBooksGarbage();

    /*
       Оновлюємо поля firstBook у записах видавництв.
       Для кожного видавництва, якщо p.firstBook не дорівнює -1,
       то замінюємо його на mapping[p.firstBook].
    */
    extern PublisherIndex publisherIndexArray[];
    extern int publisherIndexCount;
    for (int i = 0; i < publisherIndexCount; i++) {
        Publisher p = readPublisher(publisherIndexArray[i].recNo);
        if (!p.isDeleted && p.firstBook != -1) {
            int oldFirst = p.firstBook;
            if (oldFirst < 0 || oldFirst >= totalRecords || mapping[oldFirst] == -1) {
                p.firstBook = -1;  // Запис був видалений
            } else {
                p.firstBook = mapping[oldFirst];
            }
            writePublisher(publisherIndexArray[i].recNo, &p);
        }
    }

    free(mapping);
    printf("Books file reorganized successfully. Active records: %d\n", newIndex);
}
