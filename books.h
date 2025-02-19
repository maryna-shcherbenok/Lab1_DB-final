#ifndef BOOKS_H
#define BOOKS_H

/* Функції для роботи з підзаписами у slave-файл (книжки у books.fl) */
void get_s();
void insert_s();
void update_s();
void del_s();
void calc_s();
void ut_s();

/* Функції для роботи з сміттєвою зоною для книжок */
void loadBooksGarbage();
void saveBooksGarbage();

/* Функції для роботи з nextBookId */
void loadNextBookId();
void saveNextBookId();

/* Функція для фізичної реорганізації файлу книг */
void reorganizeBooks();

/* Змінні для сміттєвої зони книжок */
extern int booksGarbage[];
extern int booksGarbageCount;

#endif /* BOOKS_H */
