#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "logger.h"

#define BOOKING_CODE_LEN 10
#define CMD_LOGIN_LEN 16
#define COMANDA_COUNT_LEN 7 // Numero massimo possibile di comande: 999 <strlen('com' + '999')>
#define TABLE_LEN 3
#define TOTAL_COST_LEN 16 

struct cmd_struct {
	char* cmd;
	void* args[6];
};

enum client_type { CL, TD, KD };

struct client_device {
	int fd;
	int port; 
	enum client_type type; // Tipologia di client collegato
	struct table* bookable_table; // Copia dei tavoli prenotabili per facilitarne il recupero durante l'utilizzo del comando book.
	struct booking* booking; // Contiene informazioni utili al Table Device (se type == TD)
	struct cmd_struct* find_cmd; // Copia dei parametri della find per facilitare il recupero delle informazioni relative al timeslot (book)
	struct comanda* comande; // Lista delle comande relative al Table Device identificato dal client_device
	struct dish* dishes_ordered; // Comande relative al Table Device identificato dal client_device
	struct client_device* next; // Puntatore al prossimo client collegato della lista
};

#endif
