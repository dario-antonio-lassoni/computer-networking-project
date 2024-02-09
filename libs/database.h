/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef DATABASE_H 
#define DATABASE_H

#include <time.h>

struct booking {
	char table[3];
	struct tm timeinfo;
	char surname[20];
	char booking_code[10];
	struct booking* next;
};

struct table {
	char table[3];	
	int seats;
	char room[10];
	char position[20];
	struct table* next;
};

/* BOOKING */
struct booking* load_booking_list();
/* Ritorna la lista di prenotazioni che hanno il time slot indicato */
void select_booking_by_timestamp(struct booking** booking_list, int year, int month, int day, int hour);

/* TABLE_MAP */
void print_table_list(struct table* list);
void print_bookable_tables(struct table* list);
struct table* load_table_list();
/* Ritorna i tavoli prenotabili */
struct table* get_bookable_table(int year, int month, int day, int hour, int seats); 
void add_to_table_list(struct table** list, struct table* table);

#endif
