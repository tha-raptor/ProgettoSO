#include "definizioni.h"
#include "libscissione.h"
#include <sys/msg.h>
#include <signal.h>
#include <string.h>

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

void print_message(struct msgbuf *message){
    printf("m-text: %s\n", message->mtext);
}

int main(int agrc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem\n");
    //printf("[attivatore %d] ho inizializzato, aspetto...\n", getpid());
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione\n");
    //printf("[attivatore] inizio anche io simulazione\n");

    struct msgbuf lettura_identificazione;
    int error_msgrcv;

    struct timespec sleep_time;
    sleep_time.tv_sec = 0;          // secondi
    sleep_time.tv_nsec = STEP_NANO; // nanosecondi

    for(; ;){
        nanosleep(&sleep_time, NULL);
        while( (error_msgrcv = msgrcv(msgid, &lettura_identificazione, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT)) != -1){
            //print_message(&lettura_identificazione);
            //printf("Manderò SIGUR A %d\n", atoi(lettura_identificazione.mtext));
            kill(atoi(lettura_identificazione.mtext), SIGUSR1);
        }
        if(error_msgrcv == -1){ //se ha dato errore la msgrcv
            if(errno != ENOMSG) //se l'errore è diverso da "non ci sono più messaggi"
                err_exit("failure msgrcv"); //esci
        }
    }

    exit(EXIT_SUCCESS);
}