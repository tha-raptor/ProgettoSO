#include "definizioni.h"
#include "libscissione.h"
#include <sys/msg.h>
#include <signal.h>
#include <string.h>

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

int main(int agrc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem\n");
    printf("[attivatore %d] ho inizializzato, aspetto...\n", getpid());
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione\n");
    printf("[attivatore] inizio anche io simulazione\n");

    struct msgbuf lettura_identificazione;
    int error_msgrcv;

    /*
    for(; ;){
        sleep(2);
        while( (error_msgrcv = msgrcv(msgid, &lettura_identificazione, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT)) != -1){
            printf("Manderò SIGINT A %d", atoi(lettura_identificazione.mtext));
            kill(atoi(lettura_identificazione.mtext), SIGINT);
            //print_msg_indentificazioni(&lettura_identificazione);
        }
        if(error_msgrcv == -1){ //se ha dato errore la msgrcv
            if(errno != ENOMSG) //se l'errore è diverso da "non ci sono più messaggi"
                err_exit("failure msgrcv"); //esci
        }
    }
    */

    exit(EXIT_SUCCESS);
}