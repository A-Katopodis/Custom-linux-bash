//
// Created by kabalog on 23/5/2018.
//

#ifndef LABS_EXERCISE_H
#define LABS_EXERCISE_H
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

#endif //LABS_EXERCISE_H
