//
// Created by kabalog on 23/5/2018.
//

#include <pthread.h>
#include "prodcons1.h"

circular_buffer* cb;
pthread_mutex_t currentSizeMutex;
pthread_mutex_t cb_consumer_mutex;
pthread_mutex_t cb_producer_mutex;
pthread_mutex_t popout_mutex;
pthread_mutex_t print_mutex;



pthread_cond_t cbNotEmpty;
pthread_cond_t cbNotFull;
pthread_cond_t printingEnded;

int printCount = 0;


int producers_count;
int numbers_to_produce;
int* producer_numbers_to_output;
int popCount = 0;

int currentSize = 0;
int buffer_size;
int consumers_count;


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

        /********************************************/



        /* Insert into the buffer*/
        /********************************************/
        rc = pthread_mutex_lock(&cb_producer_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

            rc = pthread_mutex_lock(&currentSizeMutex);
            if (rc != 0) {
                printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
                pthread_exit(&rc);
            }

            // CurrentSize is shared among all threads
            // So each time we compare we must define a mutex
            while (currentSize == buffer_size){
                printf("Producer %d: Is going to wait ",threadId);

                rc = pthread_mutex_unlock(&currentSizeMutex);
                if (rc != 0) {
                    printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                    pthread_exit(&rc);
                }

                rc = pthread_cond_wait(&cbNotFull, &cb_producer_mutex);

                if (rc != 0) {
                    printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                    pthread_exit(&rc);
                }


                rc = pthread_mutex_lock(&currentSizeMutex);
                if (rc != 0) {
                    printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
                    pthread_exit(&rc);
                }

            }

            rc = pthread_mutex_unlock(&currentSizeMutex);
            if (rc != 0) {
                printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                pthread_exit(&rc);
            }

        cb_push_back(cb,&number);
        printf("Producer %d: Created and pushed random number %d \n",threadId,number);

        rc = pthread_mutex_unlock(&cb_producer_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

        rc = pthread_mutex_lock(&currentSizeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }


            currentSize++;
            //printf("Producer %d: Current size is %d \n",threadId,currentSize);

            rc = pthread_cond_broadcast(&cbNotEmpty);
            if (rc != 0) {
                printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
                pthread_exit(&rc);
            }

        rc = pthread_mutex_unlock(&currentSizeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }


        numbersProduced[i] = number;


        /********************************************/
    }


    rc = pthread_mutex_lock(&print_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }


    while(printCount != threadId){
        printf("Producer %d: Waiting to print \n",threadId);
        rc = pthread_cond_wait(&printingEnded, &print_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
            pthread_exit(&rc);
        }

    }


    printf("Producer %d: Fuckint printing ",threadId);

    printCount ++;


    rc = pthread_mutex_unlock(&print_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }





    pthread_exit(t);
}

void* consume(void* t){
    int *threadId = (int *) t;
    int rc;
    int numbersExtracted[producers_count*numbers_to_produce];
    int number;
    int i = 0;
    printf("Consumer %d:  Starting Consumption \n",*threadId);



    // while true
    while (1){
        printf("WTF");
        /* Insert into the buffer*/
        /********************************************/
        rc = pthread_mutex_lock(&cb_consumer_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }
                /* Current size is shared among producers and consumers thus it needs it's own mutex
                 *
                 * Popcount exists only in the consumers. Popcount is only updated inside the cv_consumer_mutex
                 */
                /********************************************/
                rc = pthread_mutex_lock(&currentSizeMutex);
                if (rc != 0) {
                    printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
                    pthread_exit(&rc);
                }



                /*Wait if the list is empty*/
                while (currentSize == 0 && popCount != producers_count*numbers_to_produce){
                    printf("Pop count is %d and total is %d \n",popCount, numbers_to_produce*producers_count);
                    printf("Consumer: %d: Is going to wait, Size is %d! \n",*threadId,currentSize);

                    rc = pthread_mutex_unlock(&currentSizeMutex);
                    if (rc != 0) {
                        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                        pthread_exit(&rc);
                    }


                    rc = pthread_cond_wait(&cbNotEmpty, &cb_consumer_mutex);
                    if (rc != 0) {
                        printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                        pthread_exit(&rc);
                    }



                    rc = pthread_mutex_lock(&currentSizeMutex);
                    if (rc != 0) {
                        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
                        pthread_exit(&rc);
                    }
                }

                /* if the while condition is false currentSizeMutex would not be unlocked
                 * So we unlock it now
                 * */
                rc = pthread_mutex_unlock(&currentSizeMutex);
                if (rc != 0) {
                    printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                    pthread_exit(&rc);
                }

                /*Avoiding the fact that consumer n finished the process and other consumers where waiting for their turn to leave
                 * the while loop.
                 * */
                printf("%d %d",popCount, numbers_to_produce*producers_count);

                if(popCount == numbers_to_produce*producers_count){

                    rc = pthread_mutex_unlock(&cb_consumer_mutex);
                    if (rc != 0) {
                        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
                        pthread_exit(&rc);
                    }


                    break;
                }
                /********************************************/

        /*Pop only comes when the list is not empty. Thus there will be no conflict with the producers
         * or with other consumers.
         * */
        cb_pop_front(cb,&number);


        printf("Consumer: %d: Poped number %d! \n",*threadId,number);


        /*Update the pop count for all the consumers*/
        popCount++;


        rc = pthread_mutex_unlock(&cb_consumer_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/

        /*We have finished inserting but now we need to update the current size
         * We reuse a lock for this
         * */
        /********************************************/
        rc = pthread_mutex_lock(&currentSizeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

        currentSize--;

        /*we broadcast to all of the threads. An alrternative would have been
         * pthread_cond_signal
         * */
        rc = pthread_cond_broadcast(&cbNotFull);
        if (rc != 0) {
            printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
            pthread_exit(&rc);
        }

        rc = pthread_mutex_unlock(&currentSizeMutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/

        /*Not thread shared. Each consumer has it's own unique array. No need for a mutex*/
        numbersExtracted[i] = number;
    }




    printf("\nCONSUMER GOT OUT\n");


    int consId = *threadId;
    consId += producers_count;
    rc = pthread_mutex_lock(&print_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }
    printf("Consumer %d: Done %d",*threadId,consumers_count);
    if(*threadId == consumers_count){
        printCount++;
    }

    while(printCount != consId){
        printf("Consumer %d: Waiting to print",*threadId);
        rc = pthread_cond_wait(&printingEnded, &print_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
            pthread_exit(&rc);
        }

    }


    printf("Consumer %d: Fucking printing",*threadId);

    printCount ++;

    rc = pthread_mutex_unlock(&print_mutex);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
        pthread_exit(&rc);
    }



    pthread_exit(t);
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
    consumers_count = atoi(argv[2]);
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
    rc = pthread_mutex_init(&currentSizeMutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_init(&popout_mutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_init(&cb_producer_mutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_init(&cb_consumer_mutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_init(&print_mutex,NULL);
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

    rc = pthread_cond_init(&printingEnded, NULL);
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

    producer_numbers_to_output = malloc(numbers_to_produce* sizeof(int));

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

    free(producer_numbers_to_output);

}












