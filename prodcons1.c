//
// Created by kabalog on 23/5/2018.
//

#include <pthread.h>
#include "prodcons1.h"

circular_buffer* cb;
pthread_mutex_t producerMutex;
pthread_mutex_t consumerMutex;
pthread_mutex_t currentSizeMutex;


pthread_cond_t cbNotEmpty;
pthread_cond_t cbNotFull;
int producers_count;
int popIndex = 0;
int numbers_to_produce;
int producersFinished = 0;

int pushCount = 0;
int popCount = 0;

int currentSize = 0;
int buffer_size;
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
    fflush(fopen("cons_out.txt.txt", "w"));

}

void writeToFile(char* str, char* filename){


    FILE* fp;
    int x = 10;
    printf("%s",filename);
    fp = fopen(filename, "a");
    if(fp == NULL) {
        printf("IN NULL");
        //fp = fopen(filename, "w+");
        exit(-1);
    }
    fprintf(fp,"%s",str);
    fflush(fp);

}

void printArguments(int argc, char* argv[]){
    int i = 0;
    for (i = 0; i < argc; i++)
        printf("\n%s", argv[i]);

}



void* produce(void* t){
    producerParameters* parametersInfo = (producerParameters*)t;
    int threadId = parametersInfo ->threadId;
    int numbers_to_produce = parametersInfo->numbers_to_produce;
    int* seed = &parametersInfo ->seed;
    int rc;
    int numbersProduced[numbers_to_produce];

    printf("Producer %d:  Starting Production ",threadId);
    printf(" of %d numbers with seed = %d \n",numbers_to_produce,*seed);

    for(int i = 0; i < numbers_to_produce; i++){
        int number = rand_r(seed);

        /* Check if the buffer is full and wait for signal */
        /********************************************/
        rc = pthread_mutex_lock(&producerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }


        while (currentSize == buffer_size){
            rc = pthread_cond_wait(&cbNotFull, &producerMutex);
            if (rc != 0) {
                printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                pthread_exit(&rc);
            }
        }

        rc = pthread_mutex_unlock(&producerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/



        /* Insert into the buffer*/
        /********************************************/
        rc = pthread_mutex_lock(&producerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

        cb_push_back(cb,&number);
        printf("Producer %d: Created and pushed random number %d \n",threadId,number);

        rc = pthread_mutex_lock(&currentSizeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }


        currentSize++;
        printf("Producer %d: Current size is %d \n",threadId,currentSize);

        rc = pthread_mutex_unlock(&currentSizeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }


        numbersProduced[i] = number;
        /*If the producer has finished producing the numbers we modify the variable
         * Only ONE producer has to update the variable at the same time.
         * Thus the reason we are doing it with mutex locked.
         * */
        if(numbers_to_produce == i+1){
            printf("PRODUCER FINISHED \n");
            producersFinished++;
        }

        rc = pthread_cond_broadcast(&cbNotEmpty);
        if (rc != 0) {
            printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
            pthread_exit(&rc);
        }



        rc = pthread_mutex_unlock(&producerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/
    }

    return (void*)"done";
}


void* consume(void* t){
    int *threadId = (int *) t;
    int rc;
    int numbersExtracted[producers_count*numbers_to_produce];
    int number;
    int i = 0;
    printf("Consumer %d:  Starting Consumption \n",*threadId);
    while (popCount < numbers_to_produce*producers_count){

        /* Checks if*/
        /********************************************/
        rc = pthread_mutex_lock(&consumerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }


        /*Wait if the list is empty*/
        while (currentSize == 0){
            printf("Pop count is %d and total is %d",popCount, numbers_to_produce*producers_count);
            printf("Consumer: %d: Is going to wait, Size is %d! \n",*threadId,currentSize);
            rc = pthread_cond_wait(&cbNotEmpty, &consumerMutex);
            if (rc != 0) {
                printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                pthread_exit(&rc);
            }
        }


        rc = pthread_mutex_unlock(&consumerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/


        /* Insert into the buffer*/
        /********************************************/
        rc = pthread_mutex_lock(&consumerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

        cb_pop_front(cb,&number);
        printf("Consumer: %d: Poped number %d! \n",*threadId,number);
        numbersExtracted[i] = number;
        popCount++; /* Pop count is used only by the consumers. So it's safe to update it here*/

                        /*Decrease current size. Since producer is using it as well we need mutex*/
                        /********************************************/
                        rc = pthread_mutex_lock(&currentSizeMutex);
                        if (rc != 0) {
                            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
                            pthread_exit(&rc);
                        }


                        currentSize--;
                        printf("Consumer: %d: Current size is %d! \n",*threadId,currentSize);


                        rc = pthread_mutex_unlock(&currentSizeMutex);
                        if (rc != 0) {
                            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                            pthread_exit(&rc);
                        }
                        /********************************************/
        /* Broadcast that the list is not full */
        /********************************************/
        rc = pthread_cond_broadcast(&cbNotFull);
        if (rc != 0) {
            printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/

        rc = pthread_mutex_unlock(&consumerMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/

    }

    return (void*)"done";
}



void* printHelloWorld(void* t){
    int *threadId = (int *) t;

    printf("Hello world from thread %d \n",*threadId);
    return (void*)"result";
}

void main(int argc, char *argv[]){
    int i, rc;

    /* Clear the files so we will not append to previous runs */
    //clearFiles();

    //printArguments(argc,argv);

    /* If arguments are not supplied correctly terminate the program
     * We use 6 cause the program name is a parameter as well.
     */
    if(argc!=6){
        printf("Wrong number of arguments");
        exit(1);
    }

    /*Check if the buffer size is valid*/
    if(atoi(argv[3])<10){
        printf("Buffer size not largest than 10!");
        exit(1);
    }

    producers_count = atoi(argv[1]);
    int consumers_count = atoi(argv[2]);
    buffer_size = atoi(argv[3]);
    numbers_to_produce = atoi(argv[4]);
    int seed = atoi(argv[5]);

    printf("Producers: %d \n"
           "Consumers: %d \n"
           "Buffer Size: %d \n"
           "Numbers to produce: %d \n"
           "seed: %d \n",
           producers_count,consumers_count,buffer_size,numbers_to_produce,seed);

    /*Initialize mutex and condition*/

    /********************************************/
    rc = pthread_mutex_init(&producerMutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_init(&consumerMutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_init(&currentSizeMutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_cond_init(&cbNotEmpty, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&cbNotFull, NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
        exit(-1);
    }

    /********************************************/



    /*Initialize cb*/
    /********************************************/
    cb = (circular_buffer*)malloc(sizeof (struct circular_buffer));

    /*Create the buffer*/
    cb_init(cb, buffer_size, sizeof(unsigned int));
    /********************************************/


    int number_of_threads = producers_count + consumers_count;

    /* Initialize producers threads*/
    /********************************************/
    pthread_t* producer_threads;

    producer_threads = malloc(producers_count * sizeof(pthread_t));

    producerParameters producersParameters[producers_count];


    for (int j = 0; j < producers_count; ++j) {
        producersParameters[j].threadId = j+1;
        producersParameters[j].numbers_to_produce = numbers_to_produce;
        producersParameters[j].seed = seed *j+1;
    }
    /********************************************/



    /* Initialize consumers threads*/
    /********************************************/
    pthread_t * consumer_threads;

    consumer_threads = malloc(consumers_count * sizeof(pthread_t));



    int t_consumers[consumers_count];

    for (int j = 0; j < consumers_count; ++j) {
        t_consumers[j] = j+1;
    }
    /********************************************/


    /* Start the producers */
    /********************************************/

    for ( int k = 0; k < producers_count; k++) {
        rc = pthread_create(&producer_threads[k], NULL, produce, &producersParameters[k]);
        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    /********************************************/


    /* Start the consumers */
    /********************************************/
    for ( int k = 0 ; k < consumers_count; k++) {
        rc = pthread_create(&consumer_threads[k], NULL, consume, &t_consumers[k]);
        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    /********************************************/



    void *status;
    for ( int threadCount = 0; threadCount < producers_count; threadCount++) {
        rc = pthread_join(producer_threads[threadCount], &status);

        if (rc != 0) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

        printf("Main: Producer %d returned %s as status code.\n", producersParameters[threadCount].threadId, (char *)status);
    }

    for ( int threadCount = 0; threadCount < consumers_count; threadCount++) {
        rc = pthread_join(consumer_threads[threadCount], &status);

        if (rc != 0) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

        printf("Main: Consumer %d returned %s as status code.\n", t_consumers[threadCount], (char *)status);
    }


    free(producer_threads);
    free(consumer_threads);


    cb_free(cb);

}












