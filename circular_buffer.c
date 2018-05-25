/*
 * ΟΔΗΓΙΕΣ
 * 
 * 1. 	Αντιγράψτε τα #include και το typedef struct circular_buffer στο αρχείο .h και τον κώδικα στο αρχείο .c. 
 *      Για να χρησιμοποιήσετε τη δομή, χρειάζονται τα ακόλουθα βήματα.
 * 
 * 2.	circular_buffer *cb; // δήλωση δείκτη σε δομή circular_buffer
 * 	
 * 3.	cb = (circular_buffer*)malloc(sizeof (struct circular_buffer)); // δέσμευση χώρου στην μνήμη
 * 
 * 4.	cb_init(cb, 10, 1); // Aρχικοποίηση δομής.
 *  Στο παράδειγμα επιλέγουμε ο ενταμιευτής να χωράει 10 αριθμούς,
 *  κάθε ένας από τους οποίους έχει μέγεθος 1 byte (unsigned char)
 * 
 * 5.	char *tmp_read = (char*) malloc(1); // δέσμευση χώρου στην μνήμη για char αριθμό
 *		cb_pop_front(cb, (void*)tmp_read);  // διάβασμα από τον ενταμιευτή
 * 		..									// απαραίτητες πράξεις..
 * 		free(tmp_read);						// αποδέσμευση χώρου
 * 
 * 6.	char input = rand()%256;			// διάβασμα τυχαίου αριθμού [0,256)
 * 		cb_push_back(cb, (void*)&input);	// εισαγωγή στον ενταμιευτή
 * 
 * 
 * 7.   ΜΗΝ τροποποιήσετε τη δομή circular_buffer και ΜΗ παραδώσετε αυτό το αρχείο 
 * 
 * H συγκεκριμένη υλοποίηση είναι διαθέσιμη εδώ: http://stackoverflow.com/questions/827691/how-do-you-implement-a-circular-buffer-in-c
*/

/* Αντιγράψτε από εδώ στο αρχείο .h */

#include "stdio.h" 
#include "sys/types.h"
#include "stdlib.h"
#include "string.h"

typedef struct circular_buffer
{
    void *buffer;     // data buffer
    void *buffer_end; // end of data buffer
    size_t capacity;  // maximum number of items in the buffer
    size_t count;     // number of items in the buffer
    size_t sz;        // size of each item in the buffer
    void *head;       // pointer to head
    void *tail;       // pointer to tail
} circular_buffer;

/* Αντιγράψτε μέχρι εδώ στο αρχείο .h */

/* Αντιγράψτε από εδώ στο αρχείο .c */

//initialize circular buffer
//capacity: maximum number of elements in the buffer
//sz: size of each element 
void cb_init(circular_buffer *cb, size_t capacity, size_t sz)
{
    cb->buffer = malloc(capacity * sz);
    if(cb->buffer == NULL){
		printf("Could not allocate memory..Exiting! \n");
		exit(1);
		}
        // handle error
    cb->buffer_end = (char *)cb->buffer + capacity * sz;
    cb->capacity = capacity;
    cb->count = 0;
    cb->sz = sz;
    cb->head = cb->buffer;
    cb->tail = cb->buffer;
}

//destroy circular buffer
void cb_free(circular_buffer *cb)
{
    free(cb->buffer);
    // clear out other fields too, just to be safe
}

//add item to circular buffer
void cb_push_back(circular_buffer *cb, const void *item)
{
    if(cb->count == cb->capacity)
        {
			printf("Access violation. Buffer is full\n");
			exit(1);
		}
    memcpy(cb->head, item, cb->sz);
    cb->head = (char*)cb->head + cb->sz;
    if(cb->head == cb->buffer_end)
        cb->head = cb->buffer;
    cb->count++;
}

//remove first item from circular item
void cb_pop_front(circular_buffer *cb, void *item)
{
    if(cb->count == 0)
        {
			printf("Access violation. Buffer is empy\n");
			exit(1);
	}
    memcpy(item, cb->tail, cb->sz);
    cb->tail = (char*)cb->tail + cb->sz;
    if(cb->tail == cb->buffer_end)
        cb->tail = cb->buffer;
    cb->count--;
}

/* Αντιγράψτε μέχρι εδώ στο αρχείο .c */

