#include "definizioni.h"
#include "libscissione.h"
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
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

int main(int agrc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);
    struct msgbuf lettura_identificazione, notifica_master, msg_shared_inib;
    int error_msgrcv;

    handle_sig();

    notifica_master.mtype = 3;
    sprintf(notifica_master.mtext, "%d", getpid());
    if(msgsnd(msgid, &notifica_master, sizeof(notifica_master), 0) == -1)
        err_exit("Msg snd identificazione attivatore\n");
    //printf("[messaggio mandato da attivatore]\n");
    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione attivatore\n");
    //printf("[attivatore %d] ho inizializzato, aspetto...\n", getpid());

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 11, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> attivatore");

    //CONTROLLARE SE POSTO GIUSTO
    int shmid_inib = atoi(msg_shared_inib.mtext);
    struct stat_inibitore *inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_attivatore = getpid();

    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione attivatore\n");
    //printf("[attivatore] inizio anche io simulazione\n");
    
    for(; ;){
        if(inibitore->flag_inib == 0){
            usleep(STEP_ATTIVATORE);
            while( (error_msgrcv = msgrcv(msgid, &lettura_identificazione, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT)) != -1){
                //print_message(&lettura_identificazione);
                //printf("Manderò SIGUR A %d\n", atoi(lettura_identificazione.mtext));
                scissioni[1].attivazioni += 1;
                kill(atoi(lettura_identificazione.mtext), SIGUSR1);
            }
        }
        else{ //flag_inib = 1
            kill(inibitore->pid_inibitore, SIGUSR1); //notifica inibitore che sta per forkare
            usleep(STEP_ATTIVATORE);
            while( (error_msgrcv = msgrcv(msgid, &lettura_identificazione, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT)) != -1){
                //print_message(&lettura_identificazione);
                //printf("Manderò SIGUR A %d\n", atoi(lettura_identificazione.mtext));
                scissioni[1].attivazioni += 1;
                kill(atoi(lettura_identificazione.mtext), SIGUSR1);
            }
        }
        //pause(); //provvisorio
    }
    exit(EXIT_SUCCESS);
}