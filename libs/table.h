/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef TABLE_H 
#define TABLE_H

#include "table.h"

struct table {
	char table[3];	
	int seats;
	char room[10];
	char position[20];
	struct table* next;
};

/* Carica la TABLE_MAP */
struct table* load_table_list();
/* Ritorna i tavoli prenotabili */
struct table* get_bookable_table(int year, int month, int day, int hour, int seats); 

#endif
