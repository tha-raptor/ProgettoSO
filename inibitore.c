#include "definizioni.h"
#include "libscissione.h"
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

int main(int argc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    struct msgbuf msg_shared_inib;

    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 10, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> inibitore");

    int shmid_inib = atoi(msg_shared_inib.mtext);
    struct stat_inibitore *inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione inibitore\n");
    //printf("[inibitore %d] ho inizializzato, aspetto...\n", getpid());
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione inibitore");
    //printf("[inibitore] inizio anche io simulazione\n");

    int livello_energia;
    int soglia_massima = 40;
    int soglia_minima = 30;

    for(; ;){
        livello_energia = scissioni[0].energia_prodotta / ENERGY_EXPLODE_THRESHOLD * 100;
    }

    exit(EXIT_SUCCESS);
}