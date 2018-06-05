//
// Created by kabalog on 6/6/2018.
//

#include "p3140076-p3140092-p3140219-prodcons.h"

typedef struct producerParameters{
    int threadId;
    int numbers_to_produce;
    int seed;
} producerParameters;



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

void clearFiles(){
    fflush(fopen("prod_in.txt", "w"));
    fflush(fopen("cons_out.txt", "w"));
}

void writeToFile(char* str, char* filename){


    FILE* fp;
    int x = 10;
    //printf("%s",filename);
    fp = fopen(filename, "a");
    if(fp == NULL) {
        printf("IN NULL");
        //fp = fopen(filename, "w+");
        exit(-1);
    }
    fprintf(fp,"%s",str);
    fflush(fp);

}