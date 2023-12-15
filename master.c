#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/shm.h>
#include "libscissione.h"
#include "definizioni.h"

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

int term = 0; //nessuna condizione di terminazione
int flag = 1; //true

void print_stats(struct stat_scissione *scissioni, int semid_isimulaz){
    printf("\n---STATS RELATIVE---\n");
    printf("attivazioni: %d\n", scissioni[1].attivazioni);
    printf("scorie: %d\n", scissioni[1].scorie);
    printf("energia prodotta: %d\n", scissioni[1].energia_prodotta);
    printf("energia consumata: %d\n", scissioni[1].energia_consumata);
    printf("scissioni: %d\n", scissioni[1].scissioni);
    
    //calcolo assolute
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
     printf("\n--------------------\n");
    //azzero le stats relative
    scissioni[1].attivazioni = 0;
    scissioni[1].energia_consumata = 0;
    scissioni[1].energia_prodotta = 0;
    scissioni[1].scorie = 0;
    scissioni[1].scissioni = 0;
}

int check_terminazioni(struct stat_scissione *scissioni){
    int energia_disponibile = scissioni[0].energia_prodotta - scissioni[0].energia_consumata;
    if(energia_disponibile < 0)
        return -1; //blackout
    if(scissioni[0].energia_prodotta > ENERGY_EXPLODE_THRESHOLD)
        return -2; //explode
    return 0; //si prosegue con la simulazione
}

void ignora_sigterm(){
}

void handler_sigursdue(){
    flag = 0;
    if(kill(-getpgid(getpid()), SIGTERM) == -1)
        err_exit("kill group_pid\n");
}

void uccidi_processi(int semid, int msgid, int shmid){
    if (kill(-getpgid(getpid()), SIGTERM)==-1)
        err_exit("kill group_pid\n");
}

void init_processi(pid_t parent_pid, int semid, int msgid, int shmid){
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
            uccidi_processi(semid, msgid, shmid);
            //err_exit("Fork attivatore");
            print_protagonista_term("master -> attivatore", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./attivatore", argv_processi, envp);
            err_exit("Exceve attivatore");
    }
    
    switch(fork()){ //creazione alimentazione
        case -1:
            uccidi_processi(semid, msgid, shmid);
            //err_exit("Fork alimentazione");
            print_protagonista_term("master -> alimentazione", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./alimentazione", argv_processi, envp);
            err_exit("Exceve alimentazione");
    }

    switch(fork()){ //creazione inibitore
        case -1:
            uccidi_processi(semid, msgid, shmid);
            //err_exit("Fork inibitore");
            print_protagonista_term("master -> inibitore", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./inibitore", argv_processi, envp);
            err_exit("Exceve inibitore");
    }
    int init = 0;
    pid_t child_pid;
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
                uccidi_processi(semid, msgid, shmid);
                //err_exit("Fork atomo");
                print_protagonista_term("master -> atomo_init", getpid());
                exit(EXIT_SUCCESS);
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

int main(){
    struct msgbuf message;
    pid_t master_pid = getpid();
    pid_t group_pid = getpgid(master_pid); //prendo il pid del gruppo
    int semid_isimulaz, msgid, shmid; //semid semafoto master
    union semun arg_simulazione, arg_inizializzazione,  arg_sinc;
    arg_inizializzazione.val = 0; //inizializzo semaforo inizializzazione a 0
    arg_simulazione.val = 0;
    arg_sinc.val = 1;

    struct sigaction sa_sigterm, sa_sigursdue;
    sa_sigterm.sa_handler = &ignora_sigterm;
    sa_sigterm.sa_flags = 0;
    sa_sigursdue.sa_handler = &handler_sigursdue;
    sa_sigursdue.sa_flags = 0;
    sigset_t mask_sigterm, mask_sigursdue;
    if(sigemptyset(&mask_sigterm) == -1)
        err_exit("sigemptyset su mask_sigterm");
    sa_sigterm.sa_mask = mask_sigterm;
    if(sigemptyset(&mask_sigursdue) == -1)
        err_exit("sigemptyset su mask_sigursdue");
     sa_sigursdue.sa_mask = mask_sigursdue;

    if(sigaction(SIGTERM, &sa_sigterm, NULL) == -1)
        err_exit("sigaction per SIGTERM");
    
    if(sigaction(SIGUSR2, &sa_sigursdue, NULL) == -1)
        err_exit("sigaction per SIGURS2");

    if((semid_isimulaz = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666 )) == -1)
        err_exit("Semget\n");

    if(semctl(semid_isimulaz, 0, SETVAL, arg_inizializzazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");
    
    if(semctl(semid_isimulaz, 1, SETVAL, arg_simulazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");

    /*if(semctl(semid_isimulaz, 2, SETVAL, arg_sinc) == -1) //inizializzo semaforo stats a 1, provare a fermare l'attivatore?
        err_exit("semctl con SETVAL su semid_isimulaz\n");*/

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

    message.mtype = 2;
    sprintf(message.mtext, "%d", getpid());

    if(msgsnd(msgid, &message, sizeof(message), 0) == -1) //master si identifica con mtype = 2
        err_exit("Msg snd identificazione\n");
    
    //printf("[master] inizio inizializzazione");

    init_processi(master_pid, semid_isimulaz, msgid, shmid); //inizializza tutti i processi

    int semaph_operation = N_ATOMI_INIT + 3;
    if(reserveSem(semid_isimulaz, 0, semaph_operation, 2) == -1)
        err_exit("reserveSem\n");

    //printf("\n[master grouppid: %d] fine inizializzazione, inizia la simulazione\n", group_pid);
    //inizio simulzione
    if(releaseSem(semid_isimulaz, 1, semaph_operation, 2) == -1)
        err_exit("releaseSem simulazione\n");

    for(int i = 0; i < SIM_DURATION && flag; i++){
        sleep(1);
        term = check_terminazioni(scissioni); //attenzione al primo giro andrà sempre in blackout (da risolvere)
        if(term == 0 && flag != 0){ //si prosegue
            scissioni[1].energia_consumata = ENERGY_DEMAND;
            print_stats(scissioni, semid_isimulaz);
        }
        else //blackout o explode
            flag = 0;
    }
    if(flag == 1)
        printf("[master dealloco] terminazione: timeout\n");
    else if(term == -1)
        printf("[master dealloco] terminazione: blackout\n");
    else if(term == -2)
        printf("[master dealloco] terminazione: explode\n");
    else
        printf("[master dealloco] terminazione: meltdown\n");

    uccidi_processi(semid_isimulaz, msgid, shmid);

    pid_t pid_prova;
    while ((pid_prova = waitpid(-group_pid, NULL, 0))>0);

    if(semctl(semid_isimulaz, 0, IPC_RMID, NULL) == -1) //elimino il semaforo
        err_exit("remove semid_isimulaz_inizializzazione con IPC_RMID\n");

    if(msgctl(msgid, IPC_RMID, NULL) == -1) //elimino la coda di messaggi
        err_exit("remove msg_identificazine con IPC_RMID\n");

    if(shmctl(shmid, IPC_RMID, 0) == -1) //elimino il semaforo
        err_exit("remove shared memory scissioni con IPC_RMID\n");

     exit(EXIT_SUCCESS);
}