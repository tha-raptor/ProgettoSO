#include "definizioni.h"

//utili per handler fork (SIGUSR_UNO)
struct stat_scissione *scissioni = NULL;
struct stat_inibitore *inibitore = NULL;
int msgid;
int flag_buf;
extern char **environ;

void forka_atomi(){
    struct my_msgbuf lettura_identificazione_handl;
    if(flag_buf){ //codice normale - livello non pericoloso
        while(msgrcv(msgid, &lettura_identificazione_handl, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT) != -1){
            scissioni[1].attivazioni += 1;
            kill(atoi(lettura_identificazione_handl.mtext), SIGUSR1);
        }
    }
    else{ //primo giro dopo non aver forkato (bisogna limitare le attivazioni)
        while(msgrcv(msgid, &lettura_identificazione_handl, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT) != -1){
            scissioni[1].attivazioni += 1;
            if(atoi(lettura_identificazione_handl.mtext) % 2 == 0)
                kill(atoi(lettura_identificazione_handl.mtext), SIGUSR1);
        }
    }
}

void handler_sigurs_uno(){
    forka_atomi();
}

void handler_sigurs_due(){
    flag_buf = 0;
}

void handler_flag_inibitore(){
}

void handle_sig(){
    struct sigaction sa_sigusr, sa_sigusr_due, sa_sigint;
    sa_sigusr.sa_handler = &handler_sigurs_uno;
    sa_sigusr_due.sa_handler = &handler_sigurs_due;
    sa_sigint.sa_handler = &handler_flag_inibitore;
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
    struct my_msgbuf lettura_identificazione, notifica_master, msg_shared_inib, msg_fork;
    flag_buf = 1;

    handle_sig();

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione attivatore\n");

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 11, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> attivatore");

    //CONTROLLARE SE POSTO GIUSTO
    int shmid_inib = atoi(msg_shared_inib.mtext);
    inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_attivatore = getpid();

    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione attivatore\n");

    for(; ;){
        usleep(STEP_ATTIVATORE);
        kill(inibitore->pid_inibitore, SIGUSR1);
        pause();
    }
    
    //non arriverà mai qui
    exit(EXIT_SUCCESS);
}