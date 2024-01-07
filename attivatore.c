#include "definizioni.h"

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

struct stat_scissione *scissioni = NULL;
struct stat_inibitore *inibitore = NULL;
int msgid;

extern char **environ;

void forka_atomi(){
    struct msgbuf lettura_identificazione_handl;

    char *MSG_SIZE_IDENT_ENV = getenv("MSG_SIZE_IDENT");
        if (MSG_SIZE_IDENT_ENV == NULL)
            err_exit("getenv MSG_SIZE_IDENT");

    while(msgrcv(msgid, &lettura_identificazione_handl, atoi(MSG_SIZE_IDENT_ENV), 1, MSG_NOERROR | IPC_NOWAIT) != -1){
        scissioni[1].attivazioni += 1;
        kill(atoi(lettura_identificazione_handl.mtext), SIGUSR1);
    }
}

void forka_atomi_scoria(){
    struct msgbuf lettura_identificazione_handl;

    char *MSG_SIZE_IDENT_ENV = getenv("MSG_SIZE_IDENT");
        if (MSG_SIZE_IDENT_ENV == NULL)
            err_exit("getenv MSG_SIZE_IDENT");
    int i = 0;
    while(msgrcv(msgid, &lettura_identificazione_handl, atoi(MSG_SIZE_IDENT_ENV), 1, MSG_NOERROR | IPC_NOWAIT) != -1){
        if(i % 2 == 0){
            scissioni[1].attivazioni += 1;
            kill(atoi(lettura_identificazione_handl.mtext), SIGUSR1);
        }
        else{
            scissioni[1].scorie++;
            kill(atoi(lettura_identificazione_handl.mtext), SIGTERM);
        }
        i++;
    }
}

void handler_sigurs_uno(){
    forka_atomi();
}

void handler_sigurs_due(){
    forka_atomi_scoria();
}

void handler_FlagInibitore(){
}

void handle_sig(){
    sigset_t new, old;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);
    sigprocmask(SIG_BLOCK, &new, &old);
    struct sigaction sa_sigurs, sa_sigurs_due;
    sa_sigurs.sa_handler = &handler_sigurs_uno;
    sa_sigurs_due.sa_handler = &handler_sigurs_due;
    sa_sigurs.sa_flags = 0;
    sa_sigurs_due.sa_flags = 0;

    sigset_t mask_sigurs_uno, mask_sigurs_due;
    if(sigfillset(&mask_sigurs_uno) == -1)
        err_exit("sigemptyset su mask_sigurs_uno");
    if(sigdelset(&mask_sigurs_uno, SIGTERM) == -1)
        err_exit("sigemptyset su mask_sigurs_uno");
    if(sigfillset(&mask_sigurs_due) == -1)
        err_exit("sigemptyset su mask_sigurs_due");
    if(sigdelset(&mask_sigurs_due, SIGTERM) == -1)
        err_exit("sigemptyset su mask_sigurs_due");

    sa_sigurs.sa_mask = mask_sigurs_uno;
    sa_sigurs_due.sa_mask = mask_sigurs_due;

    if(sigaction(SIGUSR1, &sa_sigurs, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
    if(sigaction(SIGUSR2, &sa_sigurs_due, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS2\n");
}


int main(int agrc, char **argv, char **envp){

    int semid = atoi(argv[0]);
    msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);
    struct msgbuf lettura_identificazione, notifica_master, msg_shared_inib, msg_fork;
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

    char *MSG_SIZE_IDENT_ENV = getenv("MSG_SIZE_IDENT");
        if (MSG_SIZE_IDENT_ENV == NULL)
            err_exit("getenv MSG_SIZE_IDENT");
    
    if(msgrcv(msgid, &msg_shared_inib, atoi(MSG_SIZE_IDENT_ENV), 11, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> attivatore");

    //CONTROLLARE SE POSTO GIUSTO
    int shmid_inib = atoi(msg_shared_inib.mtext);
    inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_attivatore = getpid();

    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione attivatore\n");
    //printf("[attivatore] inizio anche io simulazione\n");

    char *STEP_ATTIVATORE_ENV = getenv("STEP_ATTIVATORE");
    if (STEP_ATTIVATORE_ENV == NULL)
        err_exit("getenv STEP_ATTIVATORE");
    int STEP_ATTIVATORE_INT = atoi(STEP_ATTIVATORE_ENV);
    for(; ;){
        usleep(STEP_ATTIVATORE_INT);
            kill(inibitore->pid_inibitore, SIGUSR1); //notifica inibitore che sta per forkare
            pause();
    }
    exit(EXIT_SUCCESS);
}