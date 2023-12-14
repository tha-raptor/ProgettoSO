#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/shm.h>
#include "libscissione.h"
#include "definizioni.h"

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);
//extern struct stat_scissione;

void init_processi(pid_t parent_pid, pid_t child_pid, int semid, int msgid, int shmid){
    char *argv_semid = (char *)malloc(sizeof(char) * 11); //inizializzazione malloc(?)
    sprintf(argv_semid, "%d", semid);
    char *argv_msgid = (char*)malloc(sizeof(char) * 20); 
    sprintf(argv_msgid, "%d", msgid);
    char *argv_shmid = (char*)malloc(sizeof(char) * 20); 
    sprintf(argv_shmid, "%d", shmid);
    char *envp[] = {NULL};
    char *argv_processi[4] = {argv_semid, argv_msgid, argv_shmid, NULL};
     switch(fork()){ //creazione attivatore
        case -1:
            err_exit("Fork attivatore");
        case 0:
            execve("./attivatore", argv_processi, envp);
            err_exit("Exceve attivatore");
    }
    
    switch(fork()){ //creazione alimentazione
        case -1:
            err_exit("Fork attivatore");
        case 0:
            execve("./alimentazione", argv_processi, envp);
            err_exit("Exceve alimentazione");
    }

    switch(fork()){ //creazione inibitore
        case -1:
            err_exit("Fork attivatore");
        case 0:
            execve("./inibitore", argv_processi, envp);
            err_exit("Exceve inibitore");
    }
    int init = 0;
    char *argv_inizializzazione = (char*)malloc(sizeof(char) * 10); 
    sprintf(argv_inizializzazione, "%d", init);
    char **argvAtomo = (char **)malloc(sizeof(char*) * 5); 
    char *NUM_ATOMICO = (char*)malloc(sizeof(char) * 7);
    for(int i = 0; i < N_ATOMI_INIT; i++){ //creazione N_ATOMI_INIT processi atomo
        srand((unsigned int) i + 1); // setto il seed
        if(getpid() == parent_pid)
            child_pid = fork();
        
        switch(child_pid){
            case -1:
                err_exit("Fork atomo");
            case 0:  
                sprintf(NUM_ATOMICO, "%d", (rand() % N_ATOM_MAX)+1); //random tra 1 e N_ATOM_MAX
                argvAtomo[0] = NUM_ATOMICO;
                argvAtomo[1] = argv_msgid;
                argvAtomo[2] = argv_shmid;
                argvAtomo[3] = argv_semid;
                argvAtomo[4] = argv_inizializzazione;
                argvAtomo[5]= NULL;
                execve("./atomo", argvAtomo, envp); //argv = NUM_ATOMICO, envp = NULL
                err_exit("Exceve atomo\n");
                
        }
    }
    free(argv_semid);
    free(argv_msgid);
    free(NUM_ATOMICO);
    free(argvAtomo);
}

void print_stats(struct stat_scissione *scissioni, int semid_isimulaz){

    printf("\n---STATS RELATIVE---\n");
    printf("attivazioni: %d\n", scissioni[1].attivazioni);
    printf("scorie: %d\n", scissioni[1].scorie);
    printf("energia prodotta: %d\n", scissioni[1].energia_prodotta);
    printf("energia consumata: %d\n", scissioni[1].energia_consumata);
    printf("scissioni: %d\n", scissioni[1].scissioni);
    
    scissioni[0].attivazioni += scissioni[1].attivazioni;
    scissioni[0].scorie += scissioni[1].scorie;
    scissioni[0].scissioni += scissioni[1].scissioni;
    scissioni[0].energia_prodotta += scissioni[1].energia_prodotta;
    scissioni[0].energia_consumata += scissioni[1].energia_consumata;

    printf("\n---STATS ASSOLUTE---\n");
    printf("attivazioni: %d\n", scissioni[0].attivazioni);
    printf("scorie: %d\n", scissioni[0].scorie);
    printf("energia prodotta: %d\n", scissioni[0].energia_prodotta);
    printf("energia consumata: %d\n", scissioni[0].energia_consumata);
    printf("scissioni: %d\n", scissioni[0].scissioni);
    
    //azzero le stats relative
    scissioni[1].attivazioni = 0;
    scissioni[1].energia_consumata = 0;
    scissioni[1].energia_prodotta = 0;
    scissioni[1].scorie = 0;
    scissioni[1].scissioni = 0;
}

int main(){
    pid_t master_pid = getpid(), child_pid;
    int semid_isimulaz, msgid, shmid; //semid semafoto master
    union semun arg_simulazione, arg_inizializzazione,  arg_stats;
    arg_inizializzazione.val = 0; //inizializzo semaforo inizializzazione a 0
    arg_simulazione.val = 0;
    arg_stats.val = 1;
    if((semid_isimulaz = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666 )) == -1)
        err_exit("Semget\n");
    
    //printf("[master] semid: %d", semid_isimulaz);

    if(semctl(semid_isimulaz, 0, SETVAL, arg_inizializzazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");
    
    if(semctl(semid_isimulaz, 1, SETVAL, arg_simulazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");

    if((shmid = shmget(IPC_PRIVATE, sizeof(struct stat_scissione) * 2, IPC_CREAT | 0666)) == -1)
        err_exit("shmget");

    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);
    scissioni[0].attivazioni = 0;
    scissioni[0].energia_consumata = 0;
    scissioni[0].energia_prodotta = 0;
    scissioni[0].scissioni = 0;
    scissioni[0].scorie = 0;
    scissioni[1].attivazioni = 0;
    scissioni[1].energia_consumata = 0;
    scissioni[1].energia_prodotta = 0;
    scissioni[1].scissioni = 0;
    scissioni[1].scorie = 0;

    if((msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666 )) == -1)
        err_exit("Msgget\n");

    init_processi(master_pid, child_pid, semid_isimulaz, msgid, shmid); //inizializza tutti i processi

    int semaph_operation = N_ATOMI_INIT + 3;
    if(reserveSem(semid_isimulaz, 0, semaph_operation, 2) == -1)
        err_exit("reserveSem\n");

    printf("\n[master %d] fine inizializzazione, inizia la simulazione\n", getpid());
    //inizio simulzione
    if(releaseSem(semid_isimulaz, 1, semaph_operation, 2) == -1)
        err_exit("releaseSem simulazione\n");

    /*for(int i = 0;i < SIM_DURATION ;i++){
        print_stats(scissioni, semid_isimulaz);
        sleep(1);
    }*/
    
    sleep(10);
    printf("[master] aspetto e dealloco tutto\n");

    //printf("[master] SIM_DURATION iteraz, aspetto...\n");
    
    if(semctl(semid_isimulaz, 0, IPC_RMID, NULL) == -1) //elimino il semaforo
        err_exit("remove semid_isimulaz_inizializzazione con IPC_RMID\n");

    if(msgctl(msgid, IPC_RMID, NULL) == -1) //elimino la coda di messaggi
        err_exit("remove msg_identificazine con IPC_RMID\n");

    if(shmctl(shmid, IPC_RMID, 0) == -1) //elimino il semaforo
        err_exit("remove shared memory scissioni con IPC_RMID\n");
    exit(EXIT_SUCCESS);
}