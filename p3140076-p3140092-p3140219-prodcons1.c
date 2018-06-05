//
// Created by kabalog on 23/5/2018.
//

#include <pthread.h>
#include <tgmath.h>
#include "p3140076-p3140092-p3140219-prodcons-common.c"

circular_buffer* cb;
pthread_mutex_t print_mutex;
pthread_mutex_t cb_mutex;


pthread_mutex_t write_prod_in_mutex;
pthread_mutex_t write_prod_out_mutex;


pthread_cond_t cbNotEmpty;
pthread_cond_t cbNotFull;
pthread_cond_t printingEnded;

pthread_mutex_t execution_ended;
pthread_cond_t execution_ended_signal;

int printCount = 0;
int threads_done = 0;
int number_of_threads;
int producers_count;
int numbers_to_produce;
int* producer_numbers_to_output;
int popCount = 0;

int currentSize = 0;
int buffer_size;
int consumers_count;
int consumers_got_out=0;
int bufferFull = 0;



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
        /* Insert into the buffer*/
        /********************************************/
        rc = pthread_mutex_lock(&cb_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }



            // The buffer is shared among producers and consumers so they must be sychronized
            while (cb->count == cb->capacity){
                //printf("Producer %d: Is going to wait \n",threadId);
                rc = pthread_cond_wait(&cbNotFull, &cb_mutex);
                if (rc != 0) {
                    printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                    pthread_exit(&rc);
                }
            }
            cb_push_back(cb,&number);
            //printf("Producer %d: Created and pushed random number %d \n",threadId,number);

            // We simply broadcast each time.
            rc = pthread_cond_broadcast(&cbNotEmpty);
            if (rc != 0) {
                printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
                pthread_exit(&rc);
            }

        rc = pthread_mutex_unlock(&cb_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/


        numbersProduced[i] = number;

        /*Create the input of the file*/
        /********************************************/
        char* output = "Producer ";
        char id[12];
        char number_str[12];
        char* upDown = ": ";
        char* endline = "\n";
        sprintf(id,"%d",threadId);
        sprintf(number_str,"%d", number);

        char* conc = (char*) malloc(1 + strlen(output) +strlen(id) +strlen(upDown) + strlen(number_str) +strlen(endline));
        strcpy(conc,output);
        strcat(conc,id);
        strcat(conc,upDown);
        strcat(conc, number_str);
        strcat(conc,endline);

        /* WRITE TO FILE */
        rc = pthread_mutex_lock(&write_prod_in_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

            writeToFile(conc,"prod_in.txt");

        rc = pthread_mutex_unlock(&write_prod_in_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/
    }


//    rc = pthread_mutex_lock(&execution_ended);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
//
//    threads_done++;
//    if(threads_done == number_of_threads){
//        rc = pthread_cond_broadcast(&execution_ended_signal);
//        if (rc != 0) {
//            printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//            pthread_exit(&rc);
//        }
//    }else{
//        while(threads_done != number_of_threads){
//           // printf("Producer %d: Is going to wait execution ending! \n",threadId);
//            printCount=1;
//            rc = pthread_cond_wait(&execution_ended_signal, &execution_ended);
//            if (rc != 0) {
//                printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//                pthread_exit(&rc);
//            }
//        }
//    }
//
//   // printf("Producer %d: Execution ending received \n",threadId);
//
//
//
//
//    rc = pthread_mutex_unlock(&execution_ended);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
//
//
//    rc = pthread_mutex_lock(&print_mutex);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
//
//
//
//
//    while(1){
//        if(printCount == threadId){
//
//            printf("Producer %d: ",threadId);
//
//            for (int i = 0; i < numbers_to_produce -1; ++i) {
//                printf("%d, ",numbersProduced[i]);
//            }
//
//            if(numbers_to_produce >0){
//                printf("%d",numbersProduced[numbers_to_produce-1]);
//            }
//            printf("\n");
//
//            printCount++;
//            rc = pthread_cond_broadcast(&printingEnded);
//            if (rc != 0) {
//                printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//                pthread_exit(&rc);
//            }
//            break;
//        }else{
//            while(printCount != threadId){
//                //printf("Producer %d: Is going to wait to print! \n",threadId);
//                rc = pthread_cond_wait(&printingEnded, &print_mutex);
//                if (rc != 0) {
//                    printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//                    pthread_exit(&rc);
//                }
//            }
//        }
//    }
//
//    rc = pthread_mutex_unlock(&print_mutex);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
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
        /* Insert into the buffer*/
        /********************************************/
        rc = pthread_mutex_lock(&cb_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }
                /*Wait if the list is empty*/
                while (cb->count == 0 && popCount != producers_count*numbers_to_produce){
                    //printf("Pop count is %d and total is %d \n",popCount, numbers_to_produce*producers_count);
                    //printf("Consumer: %d: Is going to wait, Size is %d! \n",*threadId,currentSize);



                    rc = pthread_cond_wait(&cbNotEmpty, &cb_mutex);
                    if (rc != 0) {
                        printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
                        pthread_exit(&rc);
                    }
                }

                /*Avoiding the fact that consumer n finished the process and other consumers where waiting for their turn to leave
                 * the while loop.
                 * */
                //printf("%d %d",popCount, numbers_to_produce*producers_count);

                if(popCount == numbers_to_produce*producers_count){
                    rc = pthread_mutex_unlock(&cb_mutex);
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


                //printf("Consumer: %d: Poped number %d! \n",*threadId,number);


                /*Update the pop count for all the consumers*/
                popCount++;

                /*we broadcast to all of the threads. An alrternative would have been
                       * pthread_cond_signal
                       * */
                rc = pthread_cond_broadcast(&cbNotFull);
                if (rc != 0) {
                    printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
                    pthread_exit(&rc);
                }


        rc = pthread_mutex_unlock(&cb_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }
        /********************************************/

        /*Not thread shared. Each consumer has it's own unique array. No need for a mutex*/
        numbersExtracted[i] = number;
        i++;


        char* output = "Consumer ";
        char id[12];
        char number_str[12];
        char* upDown = ": ";
        char* endline = "\n";
        sprintf(id,"%d",*threadId);
        sprintf(number_str,"%d", number);

        char* conc = (char*) malloc(1 + strlen(output) +strlen(id) +strlen(upDown) + strlen(number_str) +strlen(endline));
        strcpy(conc,output);
        strcat(conc,id);
        strcat(conc,upDown);
        strcat(conc, number_str);
        strcat(conc,endline);

        /* WRITE TO FILE */
        rc = pthread_mutex_lock(&write_prod_out_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
            pthread_exit(&rc);
        }

        writeToFile(conc,"cons_out.txt");

        rc = pthread_mutex_unlock(&write_prod_out_mutex);
        if (rc != 0) {
            printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
            pthread_exit(&rc);
        }

    }

    //printf("Consumer %d: GOT OUT! \n",*threadId);


//
//    rc = pthread_mutex_lock(&execution_ended);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
//
//    threads_done++;
//    if(threads_done == number_of_threads){
//        rc = pthread_cond_broadcast(&execution_ended_signal);
//        if (rc != 0) {
//            printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//            pthread_exit(&rc);
//        }
//    }else{
//        while(threads_done != number_of_threads){
//            //printf("Consumer %d: Is going to wait the execution end! \n",*threadId);
//
//            rc = pthread_cond_wait(&execution_ended_signal, &execution_ended);
//            if (rc != 0) {
//                printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//                pthread_exit(&rc);
//            }
//        }
//    }
//
//    //printf("Consumer %d: Execution ending received \n",*threadId);
//
//    rc = pthread_mutex_unlock(&execution_ended);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
//
//
//    rc = pthread_mutex_lock(&print_mutex);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
//
//    while (1){
//        if(printCount == *threadId + producers_count){
//
//            printf("Consumer %d: ",*threadId);
//
//            for (int j = 0; j < i -1; ++j) {
//                printf("%d, ",numbersExtracted[j]);
//            }
//
//            if(i >0){
//                printf("%d",numbersExtracted[i-1]);
//            }
//            printf("\n");
//
//
//
//
//            printCount++;
//            rc = pthread_cond_broadcast(&printingEnded);
//            if (rc != 0) {
//                printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//                pthread_exit(&rc);
//            }
//            break;
//        }else{
//            while(printCount != *threadId + producers_count){
//                //printf("Consumer %d: Is going to wait to print! \n",*threadId);
//                rc = pthread_cond_wait(&printingEnded, &print_mutex);
//                if (rc != 0) {
//                    printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
//                    pthread_exit(&rc);
//                }
//            }
//        }
//    }
//
//    rc = pthread_mutex_unlock(&print_mutex);
//    if (rc != 0) {
//        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
//        pthread_exit(&rc);
//    }
    pthread_exit(t);
}


void main(int argc, char *argv[]){
    int i, rc;

    /* Clear the files so we will not append to previous runs */
    clearFiles();

    //printArguments(argc,argv);

    /* If arguments are not supplied correctly terminate the program
     * We use 6 cause the program name is a parameter as well.
     */
    if(argc!=6){
        printf("Wrong number of arguments\n");
        exit(1);
    }

    /*Check if the buffer size is valid*/
    if(atoi(argv[3])<=10){
        printf("Buffer size not largest than 10!\n");
        exit(1);
    }

    producers_count = atoi(argv[1]);
    consumers_count = atoi(argv[2]);
    buffer_size = atoi(argv[3]);
    numbers_to_produce = atoi(argv[4]);
    int seed = atoi(argv[5]);


    if(numbers_to_produce == 0 ){
        printf("Numbers to produce are 0. Program finished!\n");
        exit(1);

    }


    if(producers_count <= 0){
        printf("You must specify at least one producer!\n");
        exit(1);
    }

    if(consumers_count <= 0){
        printf("You must specify at least one consumer!\n");
        exit(1);
    }

    printf("Producers: %d \n"
           "Consumers: %d \n"
           "Buffer Size: %d \n"
           "Numbers to produce: %d \n"
           "seed: %d \n",
           producers_count,consumers_count,buffer_size,numbers_to_produce,seed);

    /*Initialize mutex and condition*/

    /********************************************/

    rc = pthread_mutex_init(&execution_ended,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }

    rc = pthread_mutex_init(&cb_mutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }



    rc = pthread_mutex_init(&print_mutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }


    rc = pthread_mutex_init(&write_prod_in_mutex,NULL);
    if (rc != 0) {
        printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
        pthread_exit(&rc);
    }


    rc = pthread_mutex_init(&write_prod_out_mutex,NULL);
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
    rc = pthread_cond_init(&execution_ended_signal, NULL);
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


    number_of_threads = producers_count + consumers_count;

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

        printf("Main: Producer %d returned %d as status code.\n", producersParameters[threadCount].threadId, *(int *)status);
    }

    for ( int threadCount = 0; threadCount < consumers_count; threadCount++) {
        rc = pthread_join(consumer_threads[threadCount], &status);

        if (rc != 0) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

        printf("Main: Consumer %d returned %d as status code.\n", t_consumers[threadCount], *(int *)status);
    }


    free(producer_threads);
    free(consumer_threads);


    cb_free(cb);

    free(producer_numbers_to_output);

}












