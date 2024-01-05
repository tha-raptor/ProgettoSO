#include "definizioni.h"

struct stat_scissione *scissioni = NULL;
struct stat_inibitore *inibitore = NULL;
int msgid;

void forka_atomi(){
    struct msgbuf lettura_identificazione_handl;
    while(msgrcv(msgid, &lettura_identificazione_handl, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT) != -1){
        //printf("Manderò SIGUR A %d\n", atoi(lettura_identificazione.mtext));
        scissioni[1].attivazioni += 1;
        kill(atoi(lettura_identificazione_handl.mtext), SIGUSR1);
    }
}

void handler_sigurs_uno(){
    forka_atomi();
}

void handler_sigurs_due(){
}

void handler_FlagInibitore(){
}

void handle_sig(){
    struct sigaction sa_sigurs, sa_sigurs_due,sa_SIGINT;
    sa_sigurs.sa_handler = &handler_sigurs_uno;
    sa_sigurs_due.sa_handler = &handler_sigurs_due;
    sa_SIGINT.sa_handler = &handler_FlagInibitore;
    sa_sigurs.sa_flags = 0;
    sa_sigurs_due.sa_flags = 0;
    sa_SIGINT.sa_flags = 0;

    sigset_t mask_sigurs_uno, mask_sigurs_due, mask_SIGINT;
    if(sigfillset(&mask_sigurs_uno) == -1)
        err_exit("sigfillset su mask_sigurs_uno");
    if(sigdelset(&mask_sigurs_uno, SIGTERM) == -1)
        err_exit("sigdelset su mask_sigurs_uno");
    if(sigfillset(&mask_sigurs_due) == -1)
        err_exit("sigemptyset su mask_sigurs_due");
    if(sigdelset(&mask_sigurs_due, SIGTERM))
        err_exit("sigdelset su mask_sigurs_due");
    if(sigfillset(&mask_SIGINT) == -1)
        err_exit("sigemptyset su mask_SIGINT");
    if(sigdelset(&mask_SIGINT, SIGTERM))
        err_exit("sigdelset su mask_SIGINT");

    sa_sigurs.sa_mask = mask_sigurs_uno;
    sa_sigurs_due.sa_mask = mask_sigurs_due;
    sa_SIGINT.sa_mask = mask_SIGINT;

    if(sigaction(SIGUSR1, &sa_sigurs, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
    if(sigaction(SIGUSR2, &sa_sigurs_due, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS2\n");
    if(sigaction(SIGINT, &sa_SIGINT, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGINT\n");
}


int main(int agrc, char **argv){
    int semid = atoi(argv[0]);
    msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);
    struct msgbuf lettura_identificazione, notifica_master, msg_shared_inib, msg_fork;
    int error_msgrcv;

    handle_sig();

    //printf("[messaggio mandato da attivatore]\n");
    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione attivatore\n");
    //printf("[attivatore %d] ho inizializzato, aspetto...\n", getpid());

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 11, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> attivatore");

    //CONTROLLARE SE POSTO GIUSTO
    int shmid_inib = atoi(msg_shared_inib.mtext);
    inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_attivatore = getpid();

    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione attivatore\n");
    //printf("[attivatore] inizio anche io simulazione\n");

    for(; ;){
        usleep(STEP_ATTIVATORE);
        /*if(inibitore->flag_inib == 0){
            //stoppare segnali
            forka_atomi();
        }
        else{
            kill(inibitore->pid_inibitore, SIGUSR1); //notifica inibitore che sta per forkare
            pause();
        }*/
        kill(inibitore->pid_inibitore, SIGUSR1);
        pause();
        
    }
    exit(EXIT_SUCCESS);
}