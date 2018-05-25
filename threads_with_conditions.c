/**
	Ylopoiiste ena progromma to opoio tha dimiourgei 3 nea threads. Ta dyo prwta
	tha ayksanoun mia metavliti count h opoia tha einai koini se ola. Otan i 		timi tou counter ftasei mia sygkekrimeni timi (12) tote to 3o thread tha 
	prepei na enimerwnetai gia ayti tin allagi kai na diplasiazei tin metavliti 
	count. To programma prepei na trexei mexris otou ta threads pou ayksanoun 		tin count kanoun 24 epanalipseis.
	Aparaitites synartiseis tis pthread.h vivliothikis:
	int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
		void *(*start_routine)(void*), void *arg);
	void pthread_exit(void *value_ptr);
	int pthread_join(pthread_t thread, void **value_ptr);
	int pthread_mutex_init(pthread_mutex_t *mutex, 
    	const pthread_mutexattr_t *attr);
	int pthread_mutex_destroy(pthread_mutex_t *mutex);
	int pthread_mutex_lock(pthread_mutex_t *mutex);
	int pthread_mutex_unlock(pthread_mutex_t *mutex);
	int pthread_cond_init(pthread_cond_t *cond,
    	const pthread_condattr_t *attr);
	int pthread_cond_destroy(pthread_cond_t *cond);
	int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
	int pthread_cond_signal(pthread_cond_t *cond);
	int pthread_cond_broadcast(pthread_cond_t *cond);
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS  3
#define TCOUNT 24
#define COUNT_LIMIT 12

int count = 0;
int signalSent = 0;
int isDoubleCounterThreadFinished = 0;
int isDoubleCounterThreadStarted = 0;
pthread_mutex_t countMutex;
pthread_cond_t countThresholdCondition;

void *increaseCount(void *t) {
	int i;
	int *threadId = (int *)t;
	int rc;

	rc = pthread_mutex_lock(&countMutex);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}		

	//an to thread pou tha diplasiazei ton counter den exei ksekinisei perimene mexris 
	//otou se eidopoiisei ((xrisi while gia apofygi Spurious Wakeup http://en.wikipedia.org/wiki/Spurious_wakeup))
	while (isDoubleCounterThreadStarted == 0) {
		printf("increaseCount(): thread %d, the thread that will double the counter has not started about to wait...\n", *threadId);		
		rc = pthread_cond_wait(&countThresholdCondition, &countMutex);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}

		printf("increaseCount(): thread %d, the thread that will double the counter has started.\n",
            	*threadId);
	}

	rc = pthread_mutex_unlock(&countMutex);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}	

	for (i=0; i < TCOUNT; i++) {
		//lock to mutex gia na mporesei na allaksei xwris provlima twn count.
 		rc = pthread_mutex_lock(&countMutex);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			pthread_exit(&rc);
		}		

		//molis ftaseis to katallilo limit steile sima gia na ksypnisei i methodos
		//pou tha diplasiasei ton counter.
		if (count == COUNT_LIMIT) {
      			printf("increaseCount(): thread %d, count = %d  Threshold reached.\n", *threadId, count);

			//ena mono thread prpepei na eidopoiisei to thread tou 
			//diplasiasmou.
			if (signalSent == 0) {
      				rc = pthread_cond_signal(&countThresholdCondition);
				if (rc != 0) {	
					printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
					pthread_exit(&rc);
				}	

      				printf("increaseCount(): thread %d just sent signal to the doubleCounter thread.\n", *threadId);
				signalSent = 1;
			}
		
			//perimene mexri na ginei o diplasiasmos (xrisi while gia 
			//apofygi Spurious Wakeup
			while (isDoubleCounterThreadFinished == 0) {
				printf("increaseCount(): thread %d, waiting for signal.\n", *threadId);
				rc = pthread_cond_wait(&countThresholdCondition, &countMutex);
				if (rc != 0) {	
					printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
					pthread_exit(&rc);
				}
			}
      		}
    	
		count++;

		printf("increaseCount(): thread %d, count = %d, unlocking mutex\n", 
	   		*threadId, count);
    		rc = pthread_mutex_unlock(&countMutex);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(&rc);
		}
    	}
  	
	pthread_exit(t);
}

/**
 * H synartisi diplasiasmou tou counter.
*/
void *doubleCountVariable(void *t) {
	int *threadId = (int *) t;
	printf("doubleCountVariable(): thread %d started.\n", *threadId);
	int rc;
	rc = pthread_mutex_lock(&countMutex);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}

	isDoubleCounterThreadStarted = 1;
	printf("doubleCountVariable(): thread %d about to inform all other threads.\n", *threadId);
	
	//eidopoiei ta alla threads oti exei ksekinisei.
	rc = pthread_cond_broadcast(&countThresholdCondition);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
		pthread_exit(&rc);
	}

	printf("doubleCountVariable(): thread %d informed the other threads\n", *threadId);
	//oso den ikanopoieitai i synthiki perimene (xrisi while gia apofygi Spurious Wakeup http://en.wikipedia.org/wiki/Spurious_wakeup)
	while (count < COUNT_LIMIT) {
		printf("doubleCountVariable(): thread %d going into wait...\n", *threadId);
    		rc = pthread_cond_wait(&countThresholdCondition, &countMutex);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
		
		printf("doubleCountVariable(): thread %d Condition signal received.\n", *threadId);
	}		    	
	
	count *= 2;
    	printf("doubleCountVariable(): thread %d count now = %d.\n", *threadId, count);
	isDoubleCounterThreadFinished = 1;
	rc = pthread_cond_broadcast(&countThresholdCondition);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_broadcast() is %d\n", rc);
		pthread_exit(&rc);
	}
	sleep(3);

	printf("doubleCountVariable(): thread %d sent signal to increaseCount threads.\n", *threadId);
  	
	rc = pthread_mutex_unlock(&countMutex);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}

	pthread_exit(t);
}

int main(int argc, char *argv[]) {
	int i, rc; 
	int t1 = 1, t2 = 2, t3 = 3;
	pthread_t threads[3];

  	/*arxikopoiisi tou mutex kai tou condition*/
  	rc = pthread_mutex_init(&countMutex, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}

  	rc = pthread_cond_init(&countThresholdCondition, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       		exit(-1);
	}

	//arxikopoiisi olwn twn threads
  	rc = pthread_create(&threads[0], NULL, doubleCountVariable, &t1);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
	}
	
  	rc = pthread_create(&threads[1], NULL, increaseCount, &t2);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
	}

  	rc = pthread_create(&threads[2], NULL, increaseCount, &t3);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
	}
	
	void *status;
	/*join gia ola ta threads.*/
  	for (i = 0; i < NUM_THREADS; i++) {
    		rc = pthread_join(threads[i], &status);
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}

		printf("Main(): Thread %d terminated successfully.\n", *(int *) status);
  	}

	//ektypwsi tis telikis timis tou count
  	printf ("Main(): Waited for %d threads to finish. Final value of count = %d. Done.\n", NUM_THREADS, count);

  	/*"katastrofi" mutex kai condition*/
  	rc = pthread_mutex_destroy(&countMutex);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

 	rc = pthread_cond_destroy(&countThresholdCondition);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}

	return 1;
}
