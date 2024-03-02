/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef DATABASE_H 
#define DATABASE_H

#include <time.h>
#include "common_header.h"

struct booking {
	char table[TABLE_LEN];
	struct tm timeinfo;
	char surname[20];
	char booking_code[10];
	struct booking* next;
};

struct table {
	char table[TABLE_LEN];	
	int seats;
	char room[10];
	char position[20];
	struct table* next;
};

struct dish {
	char identifier[3];
	char description[30];
	int price;
	int quantity; // Campo usato solo dalla 'comanda'
	struct dish* next;
};

struct comanda {
	char table[TABLE_LEN];
	char com_count[COMANDA_COUNT_LEN]; // Numero della comanda relativa al tavolo
	char state; // Stato ->  a:'in attesa', p:'in preparazione', s:'in servizio'
	struct dish* dish_list; // Lista ordinazioni nella comanda
	int sd; // socket descriptor del Table Device che ha inviato la comanda (lato server)
	time_t timestamp; // Istante in cui è stato effettuato l'ordine dal Table Device	
	struct comanda* next;
};

/* BOOKING */
struct booking* load_booking_list();
/* Ritorna la lista di prenotazioni che hanno il time slot indicato */
void select_booking_by_timestamp(struct booking** booking_list, int year, int month, int day, int hour);
/* Salva il booking su file */
int save_booking(struct cmd_struct* book_cmd, struct cmd_struct* find_cmd, struct table* table, char** code);
/* Verifica che il codice di prenotazione sia valido */
void verify_booking_code(char* booking_code, char** res);
void free_booking_list(struct booking** list);
struct booking* get_booking_from_code(char* booking_code);

/* TABLE_MAP */
void print_table_list(struct table* list);
void print_bookable_tables(struct table* list);
int count_elements_in_table_list(struct table* list);
struct table* load_table_list();
/* Restituisce i tavoli prenotabili */
struct table* get_bookable_table(int year, int month, int day, int hour, int seats); 
void add_to_table_list(struct table** list, struct table* table);
void free_table_list(struct table** list);

/* MENU_DISHES */
struct dish* load_menu_dishes();
void add_to_dish_list(struct dish** list, struct dish* dish);
void print_menu_dishes(struct dish* list);
/* Controlla che la comanda contenga solo piatti presenti nel menu del giorno */
int check_dishes(struct dish* list);
struct dish* get_all_dishes_by_order(struct comanda* comanda);
char* get_total_cost_by_dish_list(struct dish* list);

/* COMANDA */
void add_to_orders_list(struct comanda** list, struct comanda* comanda);
/* Prima di aggiungere in lista si occupa di incrementare il contatore della comanda */
void add_to_orders_list_with_increment(struct comanda** list, struct comanda* comanda);
int delete_from_orders_list(struct comanda** list, char* com_count, char* table);
struct comanda* find_order_in_orders_list(struct comanda* list, char* com_count, char* table);
void print_all_orders(struct comanda* list);
void print_orders_by_state(struct comanda* list, char state);
void print_orders_by_table(struct comanda* list, char* table);
void print_taken_order(struct comanda* list); // Print specifica per Kitchen Device
struct comanda* get_oldest_order_in_pending(struct client_device* list);

#endif
