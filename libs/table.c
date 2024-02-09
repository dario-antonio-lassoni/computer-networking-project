/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "table.h"
#include "common_header.h"

void print_table_list(struct table* list) {
	struct table* curr = list;

	LOG_INFO("############# TABLE MAP #############");
	while(curr != NULL) {	
		set_LOG_INFO();		
		printf("%s %s %s POSTI:%d\n", curr->table, curr->room, curr->position, curr->seats);	
		fflush(stdout);
		curr = curr->next;
	}
	LOG_INFO("#####################################");
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
	
	if(prec != NULL) // Se c'è più di un tavolo
		prec->next = NULL; 
	
	free(curr); // Libero la memoria dall'ultima struttura non utilizzata 
	fclose(fptr);
	return list;	
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
