#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int active_worker_threads;
//gcc -g -Wall -std=c99 -pthread detach.c -o detach
void *worker (void *arg){
    active_worker_threads++;
    int i;
    for(i=0;i<5;i++){
        printf("Worker thread is alive! Count=%d\n",i);
        sleep(1);
    }
    printf("Worker thread is about to terminate\n");
    active_worker_threads--;
    return NULL;

}
void printActive(){
    printf("%d\n",active_worker_threads);
}
void lastgasp(void){
    puts("We are in the exit handler!");
}
int main(int argc,char **argv){
    active_worker_threads=0;
    printActive();

    atexit(lastgasp);
    pthread_t tid[3];
    //main thread exit:terminate process , all threads gets killed
    for(int i=0;i<2;i++){
        pthread_create(&tid[i],NULL,worker,NULL);
        //pthread_detach(tid[i]);
        printf("Worker thread %d created\n",i+1);
        //pthread_join(tid[i],NULL);
    }
    //pthread_create(&tid[0],NULL,worker,NULL);
    pthread_join(tid[0],NULL);//we wait for them outside. but if we wait for the inside the loop, each will execute before another is created
    pthread_join(tid[1],NULL);
    pthread_join(tid[2],NULL);

    //sleep(1);
    
    printf("Main about to exit\n");
    pthread_exit(NULL);//terminates main thread but not the process
    return 0;
}