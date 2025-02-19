#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdio.h>
#include "struct.h"

/*
 * Глобальні файлові дескриптори:
 * fpMaster – файл publishers.fl (для master‑записів, видавництва)
 * fpSlave  – файл books.fl (для slave‑записів, книжки)
 */
extern FILE *fpMaster;
extern FILE *fpSlave;

/*
 * Функції для роботи з master‑файлом (видавництва):
 * Після фізичної реорганізації файлу записи зберігаються послідовно,
 * тому розрахунок позиції запису виконується за формулою:
 *     recNo * sizeof(Publisher)
 */
int getPublisherRecordCount();
Publisher readPublisher(int recNo);
void writePublisher(int recNo, Publisher *p);

/*
 * Функції для роботи зі slave‑файлом (книжки):
 * Після фізичної реорганізації файлу записи зберігаються послідовно,
 * тому розрахунок позиції запису виконується за формулою:
 *     recNo * sizeof(Book)
 */
int getBookRecordCount();
Book readBook(int recNo);
void writeBook(int recNo, Book *b);

#endif /* FILE_MANAGER_H */
