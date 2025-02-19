#include <stdio.h>
#include <stdlib.h>
#include "file_manager.h"
#include "publishers.h"
#include "books.h"

/* Меню (функції вибору) */
void menu() {
    printf("\nMENU\n");
    printf("1. Search and display publisher and their books (get-m)\n");
    printf("2. Search for a specific book for a publisher (get-s)\n");
    printf("3. Add a new publisher (insert-m)\n");
    printf("4. Add a new book for a publisher (insert-s)\n");
    printf("5. Update publisher data (update-m)\n");
    printf("6. Update book data (update-s)\n");
    printf("7. Delete a publisher and their books (del-m)\n");
    printf("8. Delete a book (del-s)\n");
    printf("9. Count publishers (calc-m)\n");
    printf("10. Count books (calc-s)\n");
    printf("11. Display all publisher records (ut-m)\n");
    printf("12. Display all book records (ut-s)\n");
    printf("13. Reorganize files (physical reorganization)\n");
    printf("14. Exit\n");
    printf("Select option number: ");
}

int main() {
    int choice;

    fpMaster = fopen("publishers.fl", "rb+");
    if (fpMaster == NULL) {
        fpMaster = fopen("publishers.fl", "wb+");
        if (fpMaster == NULL) {
            printf("Error opening file publishers.fl\n");
            exit(1);
        }
    }
    fpSlave = fopen("books.fl", "rb+");
    if (fpSlave == NULL) {
        fpSlave = fopen("books.fl", "wb+");
        if (fpSlave == NULL) {
            printf("Error opening file books.fl\n");
            exit(1);
        }
    }

    loadPublisherIndex();
    loadPublishersGarbage();
    loadBooksGarbage();
    loadNextBookId();

    do {
        menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input for menu choice.\n");
            while(getchar() != '\n'); // Очищення буфера вводу
            continue;
        }
        switch(choice) {
            case 1:
                get_m();
                break;
            case 2:
                get_s();
                break;
            case 3:
                insert_m();
                break;
            case 4:
                insert_s();
                break;
            case 5:
                update_m();
                break;
            case 6:
                update_s();
                break;
            case 7:
                del_m();
                break;
            case 8:
                del_s();
                break;
            case 9:
                calc_m();
                break;
            case 10:
                calc_s();
                break;
            case 11:
                ut_m();
                break;
            case 12:
                ut_s();
                break;
            case 13:
                /* Виклик функцій фізичної реорганізації файлів */
                reorganizeBooks();
                reorganizePublishers();
                break;
            case 14:
                goto exit_program;
            default:
                printf("Incorrect option number.\n");
                break;
        }
    } while (1);

exit_program:
    fclose(fpMaster);
    fclose(fpSlave);
    savePublisherIndex();
    savePublishersGarbage();
    saveBooksGarbage();
    saveNextBookId();
    return 0;
}
