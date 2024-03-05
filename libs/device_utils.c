/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "device_utils.h"
#include "common_header.h"

/* FUNZIONE DI DEBUG */
void print_list(struct device** head) {

	struct device* current = *head;
	int i = 0;

	while(current != NULL) {
		set_LOG_INFO();
		printf("Device in lista: elem %d, fd:%d porta: %d, type: %d\n", i, current->fd, current->port, current->type);
		current = current->next;
		i++;
	}
}

struct device* find_device_by_fd(struct device** head, int fd) {
	
	struct device* current = *head;

	while(current != NULL) {
		if(current->fd == fd)
			return current;
		current = current->next;
	}

	LOG_ERROR("La find non ha restituito nulla!");
	return NULL;
}

struct device* find_device_by_port(struct device** head, int port) {
	struct device* current = *head;

	while(current != NULL) {		
		if(current->port == port) 
			return current;
		current = current->next;
	} 
	
	return NULL;
}

/* Funzione utilizzata dai device per creare il proprio descrittore 
   che sarà utilizzato in fase di riconoscimento nel server (cmd RECOGNIZE_ME) */
struct device* create_device(int fd, int port, enum device_type type) {
	
	struct device* dev = (struct device*)malloc(sizeof(struct device));

	if(dev == NULL) {
		LOG_ERROR("Errore durante l'allocazione della memoria per il nuovo device descriptor!");
		exit(1);
	}
	
	dev->fd = fd;
	dev->port = port;
	dev->type = type;
	dev->bookable_table = NULL;
	dev->booking = NULL;
	dev->find_cmd = NULL;
	dev->comande = NULL;
	dev->dishes_ordered = NULL;
	dev->next = NULL;
	
	return dev;
}

int delete_device(struct device** head, int fd) {
	
	struct device* current = *head;
 	struct device* prec = NULL;
	
	if(*head == NULL) {
		LOG_WARN("Delete device fallita: la lista dei device connessi è vuota.");
		return 0;
	}

	while(current != NULL && current->fd != fd) {
		prec = current;
		current = current->next;
	}

	if(current == NULL) {
		set_LOG_WARN();	
		printf("Il device con fd %d non è presente nella lista dei device connessi", fd);
		fflush(stdout);
		return 0;
	}

	if(prec == NULL) { /* Il device da eliminare è il primo */
		*head = current->next; // Avanzo il puntatore alla testa all'elemento successivo
	} else { /* Il device da eliminare non è il primo */
		prec->next = current->next;
	}
	
	free_mem((void*)&current);
	print_list(head); // DEBUG DA ELIMINARE
	return 1;
}

int add_device(struct device** head, struct device* device) {
	/* Se è il primo device a collegarsi allora creo la lista */
	if(*head == NULL) {
		if(device != NULL) {
			*head = (struct device*)malloc(sizeof(struct device));
			(*head)->fd = device->fd;
			(*head)->port = device->port;
			(*head)->type = device->type;
			(*head)->bookable_table = device->bookable_table;
			(*head)->booking = device->booking;
			(*head)->find_cmd = device->find_cmd;
			(*head)->comande = device->comande;
			(*head)->dishes_ordered = device->dishes_ordered;
			(*head)->next = NULL;
		} else {
			LOG_ERROR("Errore durante l'aggiunta del descrittore device alla lista dei device collegati.");
			return -1;
		}
	} else {
		/* Cerco prima se esiste già un device con la stesso porta. Se esiste lancio
		   un log di WARN e ritorno 0 */		
		if(find_device_by_port(head, device->port) != NULL) {
			set_LOG_WARN();
			printf("Il device con porta %d risulta già collegato al server!\n", device->port);
			return 0; /* Errore non bloccante, faccio andare avanti il server ignorando la richiesta ritornando 0 */
		}
	
		/* Scorro la lista per inserire in coda il device */
		struct device* current = *head;
		while(current->next != NULL)
			current = current->next;
		current->next = create_device(device->fd, device->port, device->type);
		
		print_list(head); //DEBUG DA ELIMINARE
	}
	return 1; /* Ritorna 1 se l'aggiunta del device è andata a buon fine */
}

/* Verifica che non esistano comande in stato 'in preparazione' o 'in attesa' */
int check_shutdown(struct device* list) {

	struct device* dev;
	struct order* order;

	dev = list;

	while(dev != NULL) {
		order = dev->comande;

		while(order != NULL) {
			if(order->state == 'a' || order->state == 'p')
				return 0; // E' stata trovata una comanda 'in preparazione' o 'in attesa'
			order = order->next;
		}

		dev = dev->next;
	}

	return 1; // Nessuna delle comande è 'in preparazione' o 'in attesa'

}
