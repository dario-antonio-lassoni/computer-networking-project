/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "logger.h"

#define BACKLOG_SIZE 10 /* Default backlog size del listener */
#define INPUT_SIZE 512

#define CMD_LOGIN_LEN 16

/* cmd_struct_find CONST */
#define FIND_LEN 5
#define SURNAME_LEN 100

/* cmd_struct_book CONST */
#define BOOK_LEN 5

/* cmd_struct_login CONST */
#define LOGIN_LEN 5
#define BOOKING_CODE_LEN 10

/* cmd_struct_ready CONST */
#define READY_LEN 6
#define COMANDA_COUNT_LEN 7 // Numero massimo possibile di comande: 999 <strlen('com' + '999')>
#define TABLE_LEN 3

#define TOTAL_COST_LEN 16
#define GENERIC_DATA_LEN 256

struct cmd_struct {
	char* cmd; // Contiene il comando
	void* args[6]; // Contiene i parametri passati al comando
};

enum device_type { CL, TD, KD };

struct device {
	int fd;
	int port; 
	enum device_type type; // Tipologia di device collegato
	struct table* bookable_table; // Copia dei tavoli prenotabili per facilitarne il recupero durante l'utilizzo del comando book.
	struct booking* booking; // Contiene informazioni utili al Table Device (se type == TD)
	struct cmd_struct* find_cmd; // Copia dei parametri della find per facilitare il recupero delle informazioni relative al timeslot (book)
	struct order* comande; // Lista delle comande relative al Table Device
	struct device* next; // Puntatore al prossimo device collegato della lista
};

#endif
