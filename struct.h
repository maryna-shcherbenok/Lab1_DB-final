#ifndef STRUCT_H
#define STRUCT_H

#define MAX_PUBLISHER   1000
#define MAX_BOOKS       1000

#define NAME_LEN        50
#define TITLE_LEN       100
#define PHONE_LEN       20
#define EMAIL_LEN       50
#define ADDR_LEN        100
#define ISBN_LEN        20
#define GENRE_LEN       50

/* Структура запису даних видавництва */
typedef struct {
    int id;
    char name[NAME_LEN];
    char phone[PHONE_LEN];
    char email[EMAIL_LEN];
    char address[ADDR_LEN];
    int firstBook;   // Номер першої книги або -1, якщо книг немає
    int bookCount;   // Загальна кількість книг видавництва
    int isDeleted;   // Логічне видалення: 0 – активний, 1 – видалений
} Publisher;

/* Структура запису даних книги */
typedef struct {
    int id;
    char title[TITLE_LEN];
    int publisherId;   // id видавництва (зв’язок з видавництвом)
    int nextBook;      // Номер наступної книги цього видавництва або -1
    int isDeleted;     // Логічне видалення: 0 – активний, 1 – видалений
    char isbn[ISBN_LEN];
    char genre[GENRE_LEN];
    int pageCount;
} Book;

/* Структура індексної таблиці для master‑записів (видавництва) */
typedef struct {
    int key;    // id видавництва, що є ключем
    int recNo;  // Номер запису у файлі publishers.fl
} PublisherIndex;

#endif /* STRUCT_H */
