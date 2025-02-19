#ifndef PUBLISHERS_H
#define PUBLISHERS_H

#include "struct.h"

/* Функції для роботи з master‑записами (видавництва) */
void get_m();
void insert_m();
void update_m();
void del_m();
void calc_m();
void ut_m();

/* Функція пошуку видавництва за id */
int getPublisherById(int id, Publisher *publisher, int *recNo);

/* Функції для роботи з індексною таблицею (зберігається у publishers.ind) */
void loadPublisherIndex();
void savePublisherIndex();

/* Функції для роботи зі сміттєвою зоною видавництв */
void loadPublishersGarbage();
void savePublishersGarbage();

/* Функція для фізичної реорганізації файлу видавництв */
void reorganizePublishers();

#endif /* PUBLISHERS_H */
