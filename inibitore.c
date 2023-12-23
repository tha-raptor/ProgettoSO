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
    struct msgbuf msg_shared_inib;
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

    int livello_energia, livello_scissioni;
    int soglia_massima = 40;
    
    for( ; ;){
        if(inibitore->flag_inib){
            livello_energia = scissioni[0].energia_prodotta / ENERGY_EXPLODE_THRESHOLD * 100;
            livello_scissioni = scissioni[0].scissioni / 5000 * 100;
            pause();
            if(livello_energia > soglia_massima){
                //assorbire energia
            }
            if(livello_scissioni > soglia_massima){
                //o non far forkare o calcolare come scorie
            }
        }
    }
    exit(EXIT_SUCCESS);
}