#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include "definizioni.h"
#include "libscissione.h"
#include <stdio.h>

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

struct stat_scissione *scissioni = NULL;

void handler_fork(){
    scissioni[1].attivazioni += 1;
}

int main(int argc, char **argv){
    int NUM_ATOMICO = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    int semid = atoi(argv[3]);
    int init = atoi(argv[4]);
    struct msgbuf msg_identificazione;
    scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0); //scissioni[0] = assoluto, scissioni[1] = relativo
    struct sigaction sa;
    sa.sa_handler = &handler_fork;
    sa.sa_flags = 0;
    
    //printf("[atomo %d] gpid: %d\n", getpid(), getpgid(getpid())); //GROUP PID: getpgid(getpid()) check gruppo 

    sigset_t my_mask;
    if(sigemptyset(&my_mask) == -1)
        err_exit("sigemptyset su my_mask");
    sa.sa_mask = my_mask;

    if(sigaction(SIGUSR1, &sa, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");

    if(init == 0){ //init del master
        if(releaseSem(semid, 0, 1, 2) == -1)
            err_exit("releaseSem inizializzazione\n");
        //fine inizializzazione
        //printf("[atomo] ho inizializzato, aspetto...\n");
        //inizio simulazione
        if(reserveSem(semid, 1, 1, 2) == -1){
            perror("reserveSem: \n");
            err_exit("reserveSem simulazione\n");
        }
        //printf("[atomo] inizio anche io simulazione\n");
    }
    
    //printf("[pid(%d), NUM_ATOMICO: %d] [SHMID: %d]\n", getpid(), NUM_ATOMICO, shmid);
   
    sprintf(msg_identificazione.mtext, "%d", getpid());
    msg_identificazione.mtype = 1; //tutti gli atomi mtype = 1, mtext = getpid()

    if(msgsnd(msgid, &msg_identificazione,  sizeof(msg_identificazione), 0) == -1){
        perror("msg: ");
        err_exit("Msg snd identificazione\n");
    }
    
    pause();
    //printf("[atomo svegliato dal segnale]\n");
    if(NUM_ATOMICO > MIN_N_ATOMICO){
        int num_atomico_figlio_1 = NUM_ATOMICO * 0.5;
        int num_atomico_figlio_2 =  NUM_ATOMICO - num_atomico_figlio_1;
        char *num_a1_char = (char*)malloc(sizeof(char) * 20); //non so bene perchè 20, mi gustava
        sprintf(num_a1_char, "%d", num_atomico_figlio_1);
        char *num_a2_char = (char*)malloc(sizeof(char) * 20); //non so bene perchè 20, mi gustava
        sprintf(num_a2_char, "%d", num_atomico_figlio_2);
        char *msgid_char = (char*)malloc(sizeof(char) * 20); //non so bene perchè 20, mi gustava
        sprintf(msgid_char, "%d", msgid);
        char *shmid_char = (char*)malloc(sizeof(char) * 20); //non so bene perchè 20, mi gustava
        sprintf(shmid_char, "%d", shmid);
        char *semid_char = (char*)malloc(sizeof(char) * 20); //non so bene perchè 20, mi gustava
        sprintf(semid_char, "%d", semid);
        int init_uno = 1;
        char *init_char = (char*)malloc(sizeof(char) * 10); 
        sprintf(init_char, "%d", init_uno);
        char **argv_figlio_1 = (char**)malloc(sizeof(char*) * 5);
        char **argv_figlio_2 = (char**)malloc(sizeof(char*) * 5);
        argv_figlio_1[0] = num_a1_char;
        argv_figlio_1[1] = msgid_char;
        argv_figlio_1[2] = shmid_char;
        argv_figlio_1[3] = semid_char;
        argv_figlio_1[4] = init_char;
        argv_figlio_1[5] = NULL;
        argv_figlio_2[0] = num_a2_char;
        argv_figlio_2[1] = msgid_char;
        argv_figlio_2[2] = shmid_char;
        argv_figlio_2[3] = semid_char;
        argv_figlio_2[4] = init_char;
        argv_figlio_2[5] = NULL;

    char *envp[1] = {NULL};
        switch(fork()){
            case -1:
                err_exit("fork atomo\n");
            case 0:
                execve("./atomo", argv_figlio_1, envp);
                err_exit("Errore execve atomo figlio 1\n");
            default:
                scissioni[1].scissioni++;
                scissioni[1].energia_prodotta += energy(num_atomico_figlio_1, num_atomico_figlio_2);
                execve("./atomo", argv_figlio_2, envp);
                err_exit("Errore execve atomo figlio 2\n");
        }
    }
    else{
        scissioni[1].scorie++;
        //printf("[atomo non più forkabile %d] NUM_ATOMICO: %d\n", getpid(), NUM_ATOMICO);
    }
    exit(EXIT_SUCCESS);
}