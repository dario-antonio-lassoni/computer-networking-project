/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef DEVICE_UTILS_H 
#define DEVICE_UTILS_H

#include "common_header.h"
#include "common_utils.h"

enum client_type { CL, TD, KD };

struct client_device {
	int fd;
	int port; 
	enum client_type type; // Tipologia di client collegato
	struct table* bookable_table; // Copia dei tavoli prenotabili per facilitarne il recupero durante l'utilizzo del comando book
	struct cmd_struct* find_cmd; // Copia dei parametri della find per facilitare il recupero delle informazioni relative al timeslot (book)
//	struct dish* comanda; // Comande relative al Table Device identificato dal client_device
	struct client_device* next; // Puntatore al prossimo client collegato della lista
};

struct client_device* find_client_device_by_fd(struct client_device** head, int i);
struct client_device* find_client_device_by_port(struct client_device** head, int i);
struct client_device* create_client_device(int fd, int port, enum client_type type);
int delete_client_device(struct client_device** head, int fd);
int add_client_device(struct client_device** head, struct client_device* client);

#endif
