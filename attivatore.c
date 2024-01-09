#include "definizioni.h"

//utili per handler (SIGUSR1)
struct stat_scissione *scissioni = NULL;
struct stat_inibitore *inibitore = NULL;
int msgid;
extern char **environ;

void forka_atomi(){
    struct my_msgbuf lettura_identificazione_handl;
    while(msgrcv(msgid, &lettura_identificazione_handl, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT) != -1){ //prendi i messaggi sulla coda
        scissioni[1].attivazioni += 1; //aggiorna attivazioni
        kill(atoi(lettura_identificazione_handl.mtext), SIGUSR1); //manda segnale di forkare
    }
}

void handler_sigurs_uno(){
    forka_atomi();
}

void handler_sigurs_due(){
    //svegliati e non fare nulla
}

void handle_sig(){
    struct sigaction sa_sigusr, sa_sigusr_due, sa_sigint;
    sa_sigusr.sa_handler = &handler_sigurs_uno;
    sa_sigusr_due.sa_handler = &handler_sigurs_due;
    sa_sigint.sa_handler = (void *)&handler_ignora_sigint;
    sa_sigusr.sa_flags = 0;
    sa_sigusr_due.sa_flags = 0;
    sa_sigint.sa_flags = 0;

    sigset_t mask_sigusr_uno, mask_sigusr_due, mask_sigint;
    if(sigfillset(&mask_sigusr_uno) == -1)
        err_exit("sigfillset su mask_sigusr_uno");
    if(sigdelset(&mask_sigusr_uno, SIGTERM) == -1)
        err_exit("sigdelset su mask_sigusr_uno");
    if(sigfillset(&mask_sigusr_due) == -1)
        err_exit("sigemptyset su mask_sigusr_due");
    if(sigdelset(&mask_sigusr_due, SIGTERM))
        err_exit("sigdelset su mask_sigusr_due");
    if(sigfillset(&mask_sigint) == -1)
        err_exit("sigemptyset su mask_sigint");
    if(sigdelset(&mask_sigint, SIGTERM))
        err_exit("sigdelset su mask_sigint");

    sa_sigusr.sa_mask = mask_sigusr_uno;
    sa_sigusr_due.sa_mask = mask_sigusr_due;
    sa_sigint.sa_mask = mask_sigint;

    if(sigaction(SIGUSR1, &sa_sigusr, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
    if(sigaction(SIGUSR2, &sa_sigusr_due, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS2\n");
    if(sigaction(SIGINT, &sa_sigint, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGINT\n");
}

int main(int agrc, char **argv){
    int semid = atoi(argv[0]);
    msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);
    struct my_msgbuf msg_shared_inib;

    handle_sig();
    sigset_t new, old;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);
    sigprocmask(SIG_BLOCK, &new, &old); //blocco SIGINT per evitare che SIGINT faccia terminare

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 11, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> attivatore");

    int shmid_inib = atoi(msg_shared_inib.mtext);
    inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_attivatore = getpid();

    //manuale linux
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;                   // secondi
    sleep_time.tv_nsec = STEP_ATTIVATORE; // nanosecondi

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione attivatore\n");
    //fine inizializzazione
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione attivatore\n");
    //inizio simulazione

    for(; ;){
        nanosleep(&sleep_time, NULL);
        kill(inibitore->pid_inibitore, SIGUSR1); //inizio handshake con inbiitore
        pause(); //aspetta che inibitore monitori il livello di energia
    }
    
    //non arriverà mai qui
    exit(EXIT_SUCCESS);
}