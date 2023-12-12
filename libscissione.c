#include <sys/sem.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int reserveSem(int semid, int semNum, int semOperation, int numSemafori){
    struct sembuf sops[numSemafori];

   for(int i = 0; i < numSemafori; i++){
        sops[i].sem_num = i;
        sops[i].sem_op = (i == semNum) ? (semOperation*-1) : 0;
        sops[i].sem_flg = 0;
   }
    
    return semop(semid, sops, numSemafori);
}

int releaseSem(int semid, int semNum, int semOperation, int numSemafori){
    struct sembuf sops[numSemafori];

    for(int i = 0; i < numSemafori; i++){
        sops[i].sem_num = i;
        sops[i].sem_op = (i == semNum) ? semOperation : 0;
        sops[i].sem_flg = 0;
   }
    return semop(semid, sops, numSemafori);
}

void err_exit(char* s){
    printf("\nCausa errore: %s", s);
    exit(EXIT_FAILURE);
}

int energy(int n1, int n2){
    int max = n1 > n2 ? n1 : n2;
    return n1 * n2 - max;
}

void print_errshmat(){
    switch(errno){
        case EACCES:
            printf("\nEACCES :%d", errno);
            exit(EXIT_FAILURE);
        case EINVAL:
             printf("\nEINVAL: %d", errno);
            exit(EXIT_FAILURE);
        case EMFILE:
             printf("\nEMFILE: %d", errno);
            exit(EXIT_FAILURE);
        case ENOMEM:
            printf("\nENOMEM%d", errno);
            exit(EXIT_FAILURE);
    }
}