#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define POOL_CAP 20
#define NUM_THREADS 40

typedef struct{
	int objid;
} anObject;     

typedef struct {
	int cap;			
	int cou;		
	anObject** objects;
} objectPool;

pthread_mutex_t lock;
objectPool pool;

void object_pool_init(int cap, objectPool* pool){
	
	int i=0;
	
	pthread_mutex_init(&lock, NULL);

	pool->cap = cap;
	pool->cou = 0;
	pool->objects = (anObject**) malloc(cap * sizeof(anObject**));  
	
	for(i=0; i<cap; i++){
		anObject* obj = (anObject*) malloc(sizeof(anObject));   
		obj->objid = i;
		pool->objects[i] = obj;
		pool->cou++;        
	}
	
}

void object_pool_destroy(objectPool* pool){

	int i=0;
	
	for(i=0;i<pool->cou;i++){       
		free(pool->objects[i]);
	}
	free(pool->objects);
	pthread_mutex_destroy(&lock);   

	printf("Eeverything destroyed");
}

anObject* object_pool_getObject_printless(objectPool* pool){

	pthread_mutex_lock(&lock);      

	anObject* obj = NULL;

	if(pool->cou > 0){
		obj = pool->objects[--pool->cou];     
		pool->objects[pool->cou]=NULL;
	}

	pthread_mutex_unlock(&lock); 
	return obj;
}

anObject* object_pool_getObject(objectPool* pool){

	pthread_mutex_lock(&lock);      

	anObject* obj = NULL;

	if(pool->cou > 0){

		obj = pool->objects[--pool->cou];     
		pool->objects[pool->cou]=NULL;
		printf("got object(id = %d)\n\n",obj->objid);

	}
	else
		printf("No object to get\n\n");

	pthread_mutex_unlock(&lock); 
	return obj;
}


void object_pool_returnObject(objectPool* pool, anObject* obj){

	pthread_mutex_lock(&lock);

	if(obj != NULL){
		
		if(pool->cou >= pool->cap)
			printf("The pool is full, cant return the object(id = %d)\n\n", obj->objid);    
		else{

			pool->objects[pool->cou++] = obj;
			printf("returned object(id = %d)\n\n",obj->objid);

		}
	}
	else
		printf("Object is NULL cant return it \n\n");

	pthread_mutex_unlock(&lock);  
}

void *myThread(void *vargp){

	int *myid = (int *)vargp; 
	
	anObject* obj;

	obj=object_pool_getObject_printless(&pool);
	
	if(obj != NULL)
		printf("Thread %d got object(id = %d)\n\n", *myid, obj->objid);
	else
		printf("Thread %d didnt get object\n\n",*myid);
	                         	        
    object_pool_returnObject(&pool, obj);
	
	return NULL;
}

int main(){
	
	int i, j, num, thread_id[NUM_THREADS];
	anObject* obj2[100];
    pthread_t tid;
	   
	object_pool_init(POOL_CAP, &pool);
	
	printf("How many loads?");

	scanf("%d",&num);

	for(j=0;j<num;j++){

		printf("Test %d\n\n",j+1);

		for (i = 0; i < NUM_THREADS; i++){
			thread_id[i]=i+1;                                   
			pthread_create(&tid, NULL, myThread, &thread_id[i]);    
		}
		for (i = 0; i < NUM_THREADS; i++){                                 
			pthread_join(tid, NULL);    
		}
		sleep(1);
	}

	printf("\n\n");

	sleep(1);

	printf("\n\n");
	for(i=0;i<POOL_CAP+5;i++){
		obj2[i]=object_pool_getObject(&pool);
	}

	for(i=0;i<5;i++){
		obj2[i+POOL_CAP+5]=obj2[i];
	}

	for(i=0;i<POOL_CAP+10;i++){
		object_pool_returnObject(&pool, obj2[i]);
	}

	object_pool_destroy(&pool);
	return 0;
}