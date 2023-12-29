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

//printf("sono pazzo\n");
}


void handler_sigterm(){
    shmdt(scissioni);
    exit(EXIT_SUCCESS);
}

void handler_FlagInibitore(){

}

void print_message(struct msgbuf *message){
    printf("m-text: %s\n", message->mtext);
}

void handle_sig(){
    struct sigaction sa_sigurs, sa_sigterm,sa_SIGINT;
    sa_sigurs.sa_handler = &handler_fork;
    sa_sigurs.sa_flags = 0;
    sa_sigterm.sa_handler = &handler_sigterm;
    sa_sigterm.sa_flags = 0;
    sa_SIGINT.sa_handler = &handler_FlagInibitore;
    sa_SIGINT.sa_flags = 0;

    sigset_t mask_sigurs, mask_sigterm,mask_SIGINT;
    if(sigemptyset(&mask_sigurs) == -1)
        err_exit("sigemptyset su mask_sigurs");
    if(sigemptyset(&mask_sigterm) == -1)
        err_exit("sigemptyset su mask_sigterm");
         if(sigemptyset(&mask_SIGINT) == -1)
        err_exit("sigemptyset su mask_SIGINT");
    sa_sigurs.sa_mask = mask_sigurs;
    sa_sigterm.sa_mask = mask_sigterm;
    sa_SIGINT.sa_mask = mask_SIGINT;

    if(sigaction(SIGINT, &sa_SIGINT, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");    

    if(sigaction(SIGUSR1, &sa_sigurs, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
    
    if(sigaction(SIGTERM, &sa_sigterm, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
}

void identificazione(int msgid){
    //bloccare sigint
    struct msgbuf msg_identificazione;
    sprintf(msg_identificazione.mtext, "%d", getpid());
    msg_identificazione.mtype = 1; //tutti gli atomi mtype = 1, mtext = getpid()

    if(msgsnd(msgid, &msg_identificazione,  sizeof(msg_identificazione), 0) == -1){
        perror("Atomo");
    }
    //risbloccare sigint
}

int main(int argc, char **argv){
    struct msgbuf message;
    int error_msgrcv;
    int NUM_ATOMICO = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    struct msgbuf msg_shared_inib;
    //printf("[atomo %d] NUM_ATOMICO: %d\n", getpid(), NUM_ATOMICO);
    scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0); //scissioni[0] = assoluto, scissioni[1] = relativo
    
    handle_sig();
    
    if(argv[3] != NULL){ //init del master
        int semid = atoi(argv[3]);
        if(releaseSem(semid, 0, 1, 2) == -1)
            err_exit("releaseSem inizializzazione atomo\n");
        //printf("[atomo] ho inizializzato, aspetto...\n");
        if(reserveSem(semid, 1, 1, 2) == -1)
            err_exit("reserveSem simulazione atomo\n");
        //printf("[atomo] inizio anche io simulazione\n");
    }

    identificazione(msgid); //capire perchè non posso spostarla
    //printf("[atomo %d] NUM_ATOMICO: %d\n", getpid(), NUM_ATOMICO);
    pause();
   
    if(NUM_ATOMICO > MIN_N_ATOMICO){
        struct msgbuf msg_energy;
        msg_energy.mtype = 30;
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

        char **argv_figlio_1 = (char**)malloc(sizeof(char*) * 4);
        char **argv_figlio_2 = (char**)malloc(sizeof(char*) * 4);
        argv_figlio_1[0] = num_a1_char;
        argv_figlio_1[1] = msgid_char;
        argv_figlio_1[2] = shmid_char;
        argv_figlio_1[3] = NULL;
        argv_figlio_2[0] = num_a2_char;
        argv_figlio_2[1] = msgid_char;
        argv_figlio_2[2] = shmid_char;
        argv_figlio_2[3] = NULL;

    char *envp[1] = {NULL};
        switch(fork()){
            case -1:
                error_msgrcv = msgrcv(msgid, &message, MSG_SIZE_IDENT, 2, MSG_NOERROR);
                //print_message(&message);
                if(error_msgrcv == -1){ //se ha dato errore la msgrcv
                    if(errno != ENOMSG) //se l'errore è diverso da "non ci sono più messaggi"
                        err_exit("failure msgrcv"); //esci
                }
                print_protagonista_term("atomo -> atomo", getpid());
                if(kill(atoi(message.mtext), SIGUSR2 ) == -1)
                    err_exit("kill verso master\n");
                //err_exit("fork atomo\n");
                exit(EXIT_SUCCESS);
            case 0:
                execve("./atomo", argv_figlio_1, envp);
                err_exit("Errore execve atomo figlio 1\n");
                break;
            default:
                scissioni[1].scissioni++;
                int energia_prodotta_rel = energy(num_atomico_figlio_1, num_atomico_figlio_2);
                scissioni[1].energia_prodotta += energia_prodotta_rel;
                execve("./atomo", argv_figlio_2, envp);
                err_exit("Errore execve atomo figlio 2\n");
        }
    }
    else{
        scissioni[1].scorie++;
    }
    exit(EXIT_SUCCESS);
}