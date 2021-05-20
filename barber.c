#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

void * afeitar();
void * client_fuction();
void timeClient();
void * doClient();
    
pthread_mutex_t clientServed;

int totalClients;
int Chairs;
int chairsAvalable;
int clientLeave = 0;
time_t wait;


sem_t barber; 
sem_t client;
sem_t stateChairs;

void * afeitar(){    
    int counter = 0;
    while(1){
        sem_wait(&client);  //funcion de la libreria semaphore, donde se bloquea el semaforo hata que el hilo sea diferente de 0 este caso para el cliente
        sem_wait(&stateChairs); //funcion de la libreria semaphore, donde se bloquea el semaforo hata que el hilo sea diferente de 0 este caso para el barbero
        chairsAvalable++; 

        sem_post(&stateChairs);  //Se desbloquea el semaforo, devuelve un 0, si no un -1 lo que indicaria un error
        sem_post(&barber);        

        pthread_mutex_lock(&clientServed); //Bloqueo del objeto del cliente atendido

        timeClient();    
        pthread_mutex_unlock(&clientServed); //Liberacion del objeto 
        
        printf("El cliente fue atendido 😊\n");
        counter++; 
        if(counter == (totalClients - clientLeave))
            break;
    }
    pthread_exit(NULL);    
}

void * client_fuction(){  
    struct timeval begin, end;  //medicion del tiempo 
     
    sem_wait(&stateChairs); 

    if(chairsAvalable >= 1){
        chairsAvalable--; //Decremento de sillas ocupadas
        printf("El cliente %lu esta esperando\n", pthread_self());
        printf("Sillas disponibles actualmente: %d\n", chairsAvalable);
        
        gettimeofday(&begin, NULL);

        sem_post(&client);          //la funcion sem_post() desbloquea el semaforo y realiza dicha operacion de desbloquea, retorna un cero.
        sem_post(&stateChairs);         
        sem_wait(&barber); 

        gettimeofday(&end, NULL);
        
        double sec = (double)(end.tv_usec - begin.tv_usec) / 1000000 + (double)(end.tv_sec - begin.tv_sec);
        
        wait += 1000 * sec;
        printf("El cliente %ld fue atendido. \n", pthread_self());        
    }else{
        sem_post(&stateChairs);
        clientLeave++;
        printf("El cliente %ld se fue. \n",pthread_self());
    }    
    pthread_exit(NULL);
}

void timeClient(){
    int s = rand() % 501+500; 
    s = s * 1000; 
    usleep(s);
}

void * doClient(){
    int tmp;   
    int counter = 0;
    while(counter < totalClients){
        pthread_t thClient;
        tmp = pthread_create(&thClient, NULL, (void *)client_fuction, NULL);
        counter++;
        usleep(20000);
    }
}

int main(){

    srand(time(NULL));   

    pthread_t thBarber;  //Hilo del barbero
    pthread_t thClient;  //Hilo de clientes
    int tmp;

    pthread_mutex_init(&clientServed, NULL);   //Incio del mutex

    sem_init(&client, 0, 0);
    sem_init(&barber, 0, 0);                    //Declaracion de los semaforos, para llamadas a los metodos sem_wait(), sem_post()
    sem_init(&stateChairs, 0, 1);
    
    printf("Ingrese el numero de sillas: \n");
    scanf("%d", &Chairs);
    
    printf("Ingrese el numero de clientes: \n");
    scanf("%d", &totalClients);
    
    chairsAvalable = Chairs; 
    
    tmp = pthread_create(&thBarber, NULL, (void *)afeitar, NULL);  //Hilo del barbero
    tmp = pthread_create(&thClient, NULL, (void *)doClient, NULL);  //Hilo del cliente

    pthread_join(thBarber, NULL);
    pthread_join(thClient, NULL);
        
    printf("\n~~~~~~~~~\n");
    printf("Sillas disponibles en la barberia: %d\n", Chairs);
    printf("Clientes que entraron a la barberia: %d\n", totalClients);
     printf("\n~~~~~~~~~\n");
    printf("Personas que no fueron atendidas: %d\n", clientLeave);    	
    printf("Tiempo final de ejecucion: %f s.\n", (wait / (double) (totalClients - clientLeave))/100);
}