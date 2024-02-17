/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "database.h"
#include "common_utils.h"

void print_booking_list(struct booking* list) {
	struct booking* curr = list;

	LOG_INFO("############## BOOKING ##############");
	while(curr != NULL) {
		set_LOG_INFO();
		printf("%s %d-%d-%d %d %s %s\n", curr->table, curr->timeinfo.tm_mday, 
			curr->timeinfo.tm_mon, curr->timeinfo.tm_year, curr->timeinfo.tm_hour, 
			curr->surname, curr->booking_code);
		fflush(stdout);
		curr = curr->next;
	}
	LOG_INFO("#####################################");
}


void print_table_list(struct table* list) {
	struct table* curr = list;
	int i = 0;
	LOG_INFO("############# TABLE MAP #############");
	while(curr != NULL) {	
		set_LOG_INFO();		
		printf("%d) %s %s %s POSTI:%d\n", i, curr->table, curr->room, curr->position, curr->seats);	
		fflush(stdout);
		curr = curr->next;
		i++;
	}
	LOG_INFO("#####################################");
}


void print_bookable_tables(struct table* list) {
	struct table* curr = list;
	int i = 1;
	printf("Tavoli prenotabili per il timeslot selezionato:\n");
	while(curr != NULL) {	
		printf("%d) %s %s %s\n", i, curr->table, curr->room, curr->position);	
		fflush(stdout);
		curr = curr->next;
		i++;
	}
}

int count_elements_in_table_list(struct table* list) {
	
	struct table* curr;
	int num_elem = 0;

	if(list == NULL)
		return -1;
	
	curr = list;

	while(curr != NULL) {
		num_elem++;
		curr = curr->next;
	}

	return num_elem;

}

char* create_booking_code(char* table, int day, int month, int year, int hour) {

	char* booking_code = (char*)malloc(sizeof(char) * 10); 
	
	memset(booking_code, 0, 10);
	sprintf(booking_code, "%s%d%d%d%d", table, day, month, year, hour);

	/* I codici di prenotazione sono univoci per tavolo + timeslot
	 * dunque non si rischia di generarne due uguali */

	return booking_code;	
}

struct booking* load_booking_list() {
	struct booking *list, *prec, *curr;
	FILE* fptr;

	fptr = fopen("./database/booking.txt", "r");
	
	if(fptr == NULL) {
		LOG_ERROR("Errore durante la lettura delle prenotazioni pregresse");
		exit(1); 
	}

	/* Copia da file a lista in memoria di tutte le prenotazioni pregresse */
	list = (struct booking*)malloc(sizeof(struct booking));
	prec = NULL;
	curr = list;

	while(fscanf(fptr, "%s %d-%d-%d %d %s %s.", curr->table, &curr->timeinfo.tm_mday, &curr->timeinfo.tm_mon, 
			&curr->timeinfo.tm_year, &curr->timeinfo.tm_hour, curr->surname, curr->booking_code) == 7) {

		prec = curr;
		curr->next = (struct booking*)malloc(sizeof(struct booking));
		curr = curr->next;	
	}

	free(curr); // Libero la memoria dall'ultima struttura non utilizzata

	if(prec != NULL)  // Se c'è almeno una prenotazione
		prec->next = NULL; 
	else 
		list = NULL;
	
	fclose(fptr);

	return list;	
}

struct table* load_table_list() {
	struct table *list, *prec, *curr;	
	FILE* fptr;

	fptr = fopen("./database/table_map.txt", "r");

	if(fptr == NULL) {
		LOG_ERROR("Errore durante la lettura della mappa dei tavoli");
		exit(1);
	}
								
	/* Copia da file a lista in memoria di tutti i tavoli del ristorante */
	list = (struct table*)malloc(sizeof(struct table));
	prec = NULL;
	curr = list;

	while(fscanf(fptr, "%s %s %s POSTI:%d", curr->table, curr->room, curr->position, &curr->seats) != EOF) {	
		prec = curr;	
		curr->next = (struct table*)malloc(sizeof(struct table));
		curr = curr->next;
	}
	
	free(curr); // Libero la memoria dall'ultima struttura non utilizzata 
	
	if(prec != NULL)  // Se c'è almeno una prenotazione
		prec->next = NULL; 
	else 
		list = NULL;
	
	fclose(fptr);	

	return list;	
}


int save_booking(struct cmd_struct* book_cmd, struct cmd_struct* find_cmd, struct table* table, char** code) {
	
	FILE* fptr;
	int i;
	struct table* curr;
	char* code_with_result; // Se la prenotazione va a buon fine, conterrà il messaggio da inviare al client

	if(book_cmd == NULL || find_cmd == NULL || table == NULL)
		return -1;	

	fptr = fopen("./database/booking.txt", "a");

	if(fptr == NULL) {
		LOG_ERROR("Errore durante il salvataggio della prenotazione");
		return -1; // Gestisco l'errore fuori
	}
	
	/* Ricavo l'opzione scelta con il comando book dalla lista dei tavoli prenotabili */
	
	curr = table;
	
	for(i = 1; i < *((int*)book_cmd->args[0]); i++) // args[0] contiene l'opzione selezionata dal client con il comando book
		curr = curr->next;

	*code = create_booking_code(
			curr->table, // Tavolo 
			*((int*)find_cmd->args[2]) // Giorno
			, *((int*)find_cmd->args[3]), // Mese
			*((int*)find_cmd->args[4]), // Anno
			*((int*)find_cmd->args[5])); //Ora

	/* Scrittura in append sul file delle prenotazioni */

	fprintf(fptr, "%s %d-%d-%d %d %s %s\n", 
			curr->table, // Tavolo
			*((int*)find_cmd->args[2]), // Giorno 
			*((int*)find_cmd->args[3]), // Mese
			*((int*)find_cmd->args[4]), // Anno
			*((int*)find_cmd->args[5]), // Ora
			(char*)find_cmd->args[0],  // Cognome
			*code); // Codice prenotazione
	
	fclose(fptr);

	/* Creo la struttura del risultato che conterrà il codice prenotazione.
	   Formato: <BOOKING_CODE>_<TAVOLO>_<SALA> */	

	code_with_result = (char*)malloc(sizeof(char) * 60);
	memset(code_with_result, 0, 60);
	sprintf(code_with_result, "Codice prenotazione: %s, Tavolo: %s, Sala: %s", *code, curr->table, curr->room);		
	*code = code_with_result;	
	set_LOG_INFO();
	printf("Dati prenotazione confermata: %s\n", code_with_result);

	return 0; // Salvataggio andato a buon fine
}

/* Elimina dalla booking_list passata come argomento tutte le prenotazioni che non hanno lo stesso timeslot */
void select_booking_by_timestamp(struct booking** booking_list, int day, int month, int year, int hour) {
	struct booking *curr, *prev;

	curr = *booking_list;
	prev = NULL;

	if(curr == NULL)
		return;

	while(curr != NULL) {
		if(curr->timeinfo.tm_mday != day || curr->timeinfo.tm_mon != month || curr->timeinfo.tm_year != year || curr->timeinfo.tm_hour != hour) {
			if(prev == NULL) { // Se si tratta dell'elemento in testa
				*booking_list = (*booking_list)->next;
				//free(curr); // Elimina l'elemento in testa
				curr = *booking_list;
			} else {
				prev->next = curr->next;
				free(curr); // Elimina l'elemento
				curr = prev->next;
			}
		} else { // Se la prenotazione non ha il time slot indicato passo avanti...
			prev = curr;
			curr = curr->next;
		}
	}
}


/* Elimina dalla table_list passata come argomento tutti i tavoli che non hanno abbastanza posti */
void select_table_by_seats(struct table** table_list, int seats) {
	struct table *curr, *prec;

	curr = *table_list;
	prec = NULL;

	if(curr == NULL)
		return;
	
	while(curr != NULL) {
		if(curr->seats < seats) {
			if(prec == NULL) { // Se si tratta dell'elemento in testa
				*table_list = (*table_list)->next;
				free(curr); // Elimina l'elemento in testa
				curr = *table_list;
				continue;
			} else {
				prec->next = curr->next;
				free(curr); // Elimina l'elemento
				curr = prec->next;
			}
		} else { // Se il tavolo ha abbastanza posti lo tengo in lista e passo avanti...
			prec = curr;
			curr = curr->next;
		}
	}
}

struct table* get_bookable_table(int seats, int day, int month, int year, int hour) {
	struct booking *booking_list, *curr_book;
	struct table *table_list, *curr_table, *prec_table;

	booking_list = load_booking_list();
	table_list = load_table_list();	
		
	if(table_list == NULL) {
		LOG_WARN("La table_map è vuota o non è stata valorizzata correttamente!");
		return NULL;
	}

	/* Elimina dalla booking_list tutte le prenotazioni che non hanno il timeslot indicato */
	select_booking_by_timestamp(&booking_list, day, month, year, hour);

	/* Elimina dalla table_list tutti i tavoli che non hanno il numero minimo di posti indicato */
	select_table_by_seats(&table_list, seats); 
	
	curr_book = booking_list;
	curr_table = table_list;
	prec_table = NULL;

	print_table_list(curr_table); // Debug, da eliminare
	print_booking_list(curr_book); // Debug, da eliminare

	/* Uso la booking_list che contiene tutte le prenotazioni che vanno 
	   in conflitto con il timeslot selezionato per trovare i tavoli prenotabili,
	   rimuovendo i tavoli prenotati dalla lista dei tavoli che hanno abbastanza posti
	   per la prenotazione che vogliamo completare */

	while(curr_book != NULL) {
		while(curr_table != NULL) {
			if(strcmp(curr_book->table, curr_table->table) == 0) {
				if(prec_table == NULL) { // Se si tratta dell'elemento in testa
					table_list = table_list->next;
					free(curr_table); // Elimina il primo elemento dalla lista
					curr_table = table_list;
				} else {
					prec_table->next = curr_table->next;
					free(curr_table);
					curr_table = prec_table->next;
				}
				break;
			} else {
				prec_table = curr_table;
				curr_table = curr_table->next;
			}
		}
		curr_table = table_list;	
		prec_table = NULL;
		curr_book = curr_book->next;
	}
	
	free(booking_list);
	print_table_list(table_list); // Debug, da eliminare
	return table_list;
}

void add_to_table_list(struct table** list, struct table* table) {
	
	struct table* curr;
	
	/* Inserisco il tavolo in testa se è il primo in lista */
	if(*list == NULL) {
		*list = table;
		return;
	} 
	
	/* Avanzo fino alla coda della lista */
	curr = *list;
	while(curr->next != NULL) {
		curr = curr->next;
	}

	/* Inserisco in coda */
	curr->next = table;

}

void free_table_list(struct table** list) {
	
	struct table *prec, *curr;

	if(*list == NULL)
		return;

	curr = *list;	
	while(curr != NULL) {
		prec = curr;
		curr = curr->next;
		free_mem((void*)&prec);
	}

}
