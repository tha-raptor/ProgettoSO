#include "definizioni.h"
#include "libscissione.h"
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>
#include <string.h>

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

void handler_sigurs_uno(){
}

void handle_sig(){
    struct sigaction sa_sigurs;
    sa_sigurs.sa_handler = &handler_sigurs_uno;
    sa_sigurs.sa_flags = 0;

    sigset_t mask_sigurs_uno;
    if(sigemptyset(&mask_sigurs_uno) == -1)
        err_exit("sigemptyset su mask_sigurs");

    sa_sigurs.sa_mask = mask_sigurs_uno;

    if(sigaction(SIGUSR1, &sa_sigurs, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
}

int main(int argc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    struct msgbuf msg_shared_inib, msg_fork, msg_energy;
    msg_fork.mtype = 20;
    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);

    handle_sig();

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 10, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> inibitore");

    int shmid_inib = atoi(msg_shared_inib.mtext);
    struct stat_inibitore *inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_inibitore = getpid();

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione inibitore\n");
    //printf("[inibitore %d] ho inizializzato, aspetto...\n", getpid());
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione inibitore");
    //printf("[inibitore] inizio anche io simulazione\n");

    float livello_energia, livello_scissioni;
    float soglia_massima = 0.40;
    while(1){
        if(inibitore->flag_inib){ //ogni 10 attivazioni guarda se fare tutto
            pause();
            livello_energia = ((float)scissioni[0].energia_prodotta / ENERGY_EXPLODE_THRESHOLD);
            livello_scissioni = ((float)scissioni[0].scissioni / 5000);
            //printf("livello_energia: %f\n", livello_energia);
            //printf("livello_scissioni: %f\n", livello_scissioni);
            if(livello_energia > soglia_massima || livello_scissioni > soglia_massima){
                sprintf(msg_fork.mtext, "%d", 0); //non forkare -> situazione pericolosa
                if(msgsnd(msgid, &msg_fork, sizeof(msg_fork), 0) == -1){
                    perror("");
                    err_exit("msgsnd inibitore -> attivatore\n");
                }
                   
            }
            else{
                sprintf(msg_fork.mtext, "%d", 1); //continua a forkare -> situazione non pericolosa
                if(msgsnd(msgid, &msg_fork, sizeof(msg_fork), 0) == -1){
                    perror("");
                    err_exit("msgsnd inibitore -> attivatore\n");
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}