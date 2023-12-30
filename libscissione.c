#include "definizioni.h"

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

void print_protagonista_term(char *s, int pid){
    printf("Protagonista meltdown [%s] %d\n", s, pid);
}