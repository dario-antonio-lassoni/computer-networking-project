/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "device_utils.h"
#include "common_header.h"

/* FUNZIONE DI DEBUG, DA ELIMINARE */
void print_list(struct client_device** head) {
	struct client_device* current = *head;
	int i = 0;
	while(current != NULL) {
		set_LOG_INFO();
		printf("Elementi in lista: elem %d, fd:%d porta: %d, type: %d\n", i, current->fd, current->port, current->type);
		current = current->next;
		i++;
	}
}

struct client_device* find_client_device_by_fd(struct client_device** head, int fd) {
	struct client_device* current = *head;

	while(current != NULL) {
		if(current->fd == fd)
			return current;
		current = current->next;
	}
	LOG_ERROR("La find non ha restituito nulla!");
	return NULL;
}

//Nota: serve per forza il puntatore a puntatore se ci vogliamo portare dietro dal main alla funzione il puntatore principale e modificarlo
struct client_device* find_client_device_by_port(struct client_device** head, int port) {
	struct client_device* current = *head;

	while(current != NULL) {		
		if(current->port == port) 
			return current;
		current = current->next;
	} 
	LOG_ERROR("La find non ha restituito nulla!");
	return NULL;
}

/* Funzione utilizzata dai client per creare il proprio descrittore 
   che sarà utilizzato in fase di riconoscimento nel server (cmd RECOGNIZE_ME) */
struct client_device* create_client_device(int fd, int port, enum client_type type) {
	struct client_device* dev = (struct client_device*)malloc(sizeof(struct client_device));

	if(dev == NULL) {
		LOG_ERROR("Errore durante l'allocazione della memoria per il nuovo device descriptor!");
		exit(1);
	}
	
	dev->fd = fd;
	dev->port = port;
	dev->type = type;
	dev->bookable_table = NULL;
	dev->next = NULL;
	
	return dev;
}

int delete_client_device(struct client_device** head, int fd) {
	
	struct client_device* current = *head;
 	struct client_device* prec = NULL;
	
	if(*head == NULL) {
		LOG_WARN("Delete client device fallita: la lista dei device connessi è vuota.");
		return 0;
	}

	while(current != NULL && current->fd != fd) {
		prec = current;
		current = current->next;
	}

	if(current == NULL) {
		set_LOG_WARN();	
		printf("Il client device con fd %d non è presente nella lista dei device connessi", fd);
		fflush(stdout);
		return 0;
	}

	if(prec == NULL) { /* Il client device da eliminare è il primo */
		*head = current->next; // Avanzo il puntatore alla testa all'elemento successivo
	} else { /* Il client device da eliminare non è il primo */
		prec->next = current->next;
	}
	
	free_mem((void*)&current);
	print_list(head); // DEBUG DA ELIMINARE
	return 1;
}

int add_client_device(struct client_device** head, struct client_device* client) {
	/* Se è il primo client a collegarsi allora creo la lista */
	if(*head == NULL) {
		if(client != NULL) {
			*head = (struct client_device*)malloc(sizeof(struct client_device));
			(*head)->fd = client->fd;
			(*head)->port = client->port;
			(*head)->type = client->type;
			(*head)->bookable_table = client->bookable_table;
			(*head)->next = NULL;
		} else {
			LOG_ERROR("Errore durante l'aggiunta del descrittore client device alla lista dei client collegati.");
			return -1;
		}
	} else {
		/* Cerco prima se esiste già un device con la stesso porta. Se esiste lancio
		   un log di WARN e ritorno 0 */		
		if(find_client_device_by_port(head, client->port) != NULL) {
			set_LOG_WARN();
			printf("Il device con porta %d risulta già collegato al server!\n", client->port);
			return 0; /* Errore non bloccante, faccio andare avanti il server ignorando la richiesta ritornando 0 */
		}
	
		/* Scorro la lista per inserire in coda il client */
		struct client_device* current = *head;
		while(current->next != NULL)
			current = current->next;
		current->next = create_client_device(client->fd, client->port, client->type);
		
		print_list(head); //DEBUG DA ELIMINARE
	}
	return 1; /* Ritorna 1 se l'aggiunta del device è andata a buon fine */
}

