#include "definizioni.h"

extern char **environ;

struct stat_scissione *scissioni = NULL;

void handler_fork(){
}

void handler_sigterm(){
    shmdt(scissioni);
    exit(EXIT_SUCCESS);
}

void handle_sig(){
    struct sigaction sa_sigusr_uno, sa_sigterm;
    sa_sigusr_uno.sa_handler = &handler_fork;
    sa_sigusr_uno.sa_flags = 0;
    sa_sigterm.sa_handler = &handler_sigterm;
    sa_sigterm.sa_flags = 0;

    sigset_t mask_sigusr_uno, mask_sigterm;
    if(sigemptyset(&mask_sigusr_uno) == -1)
        err_exit("sigemptyset su mask_sigusr_uno");
    if(sigemptyset(&mask_sigterm) == -1)
        err_exit("sigemptyset su mask_sigterm");

    sa_sigusr_uno.sa_mask = mask_sigusr_uno;
    sa_sigterm.sa_mask = mask_sigterm;

    if(sigaction(SIGUSR1, &sa_sigusr_uno, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
    
    if(sigaction(SIGTERM, &sa_sigterm, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
}

void identificazione(int msgid){
    struct my_msgbuf msg_identificazione;
    sprintf(msg_identificazione.mtext, "%d", getpid());
    msg_identificazione.mtype = 1; //tutti gli atomi mtype = 1, mtext = getpid()

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) //blocco ricezione SIGINT per l'identificazione
        perror("Impossibile bloccare SIGINT\n");
    
    if(msgsnd(msgid, &msg_identificazione, sizeof(msg_identificazione), 0) == -1){
         perror("Atomo");
         exit(EXIT_FAILURE);
    }
    if(sigprocmask(SIG_UNBLOCK, &set, NULL) == -1) //sblocco la ricezione di SIGINT
        perror("Impossibile risbloccare SIGINT\n");
}

int main(int argc, char **argv){
    sigset_t new, old;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);
    sigprocmask(SIG_BLOCK, &new, &old); //blocco SIGINT per evitare che SIGINT faccia terminare
    struct my_msgbuf message, msg_energy; //messaggio che manda atomo per inibitore
    msg_energy.mtype = 30;

    int NUM_ATOMICO = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
  
    scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0); //scissioni[0] = assoluto, scissioni[1] = relativo

    handle_sig();

    if(argv[3] != NULL){
        int semid = atoi(argv[3]);
        if(releaseSem(semid, 0, 1, 2) == -1)
            err_exit("releaseSem inizializzazione atomo\n");
        if(reserveSem(semid, 1, 1, 2) == -1)
            err_exit("reserveSem simulazione atomo\n");
    }
    identificazione(msgid);
    pause(); //aspetta segnale SIGUSR1 di attivatores

    if(NUM_ATOMICO > MIN_N_ATOMICO){
        int num_atomico_figlio_1 = NUM_ATOMICO * 0.5;
        int num_atomico_figlio_2 =  NUM_ATOMICO - num_atomico_figlio_1;
        char *num_a1_char = (char*)malloc(sizeof(char) * 3); //non so bene perchè 20, mi gustava
        sprintf(num_a1_char, "%d", num_atomico_figlio_1);
        char *num_a2_char = (char*)malloc(sizeof(char) * 3);
        sprintf(num_a2_char, "%d", num_atomico_figlio_2);
        char *msgid_char = (char*)malloc(sizeof(char) * 11); //non so bene perchè 20, mi gustava
        sprintf(msgid_char, "%d", msgid);
        char *shmid_char = (char*)malloc(sizeof(char) * 11); //non so bene perchè 20, mi gustava
        sprintf(shmid_char, "%d", shmid);

        char **argv_figlio_1 = (char**)malloc(sizeof(char*) * 4);
        char **argv_figlio_2 = (char**)malloc(sizeof(char*) * 4);
        argv_figlio_1[0] = num_a1_char;
        argv_figlio_1[1] = msgid_char;
        argv_figlio_1[2] = shmid_char;
        argv_figlio_1[3] = NULL;
        argv_figlio_2[0] = num_a2_char;
        argv_figlio_2[1] = msgid_char;
        argv_figlio_2[2] = shmid_char;
        argv_figlio_2[3] = NULL;

        int error_msgrcv;
        int energia_prodotta_rel;
        switch(fork()){
            case -1:
                if(msgrcv(msgid, &message, MSG_SIZE_IDENT, 2, MSG_NOERROR) == -1) //messaggio da master con suo PID
                    err_exit("failure msgrcv"); //esci
                print_protagonista_term("atomo -> atomo", getpid()); //
                if(kill(atoi(message.mtext), SIGUSR2) == -1) //notifica master
                    err_exit("kill verso master\n");
                exit(EXIT_SUCCESS);
            case 0:
                execve("./atomo", argv_figlio_1, environ);
                err_exit("Errore execve atomo figlio 1\n");
                break;
            default:
                energia_prodotta_rel = energy(num_atomico_figlio_1, num_atomico_figlio_2); //calcola energia prodotta
                sprintf(msg_energy.mtext, "%d", energia_prodotta_rel); //preparo messaggio
                //aggiorno mem condivisa
                scissioni[1].energia_prodotta += energia_prodotta_rel;
                scissioni[1].scissioni++;
                sigset_t new, old;
                sigfillset(&new); 
                sigdelset(&new, SIGTERM);
                sigprocmask(SIG_BLOCK, &new, &old); //blocchiamo tutti i segnali tranne sigterm (terminazione)
                if(msgsnd(msgid, &msg_energy, sizeof(msg_energy), 0) == -1)
                    err_exit("prova");
                sigprocmask(SIG_SETMASK, &old, NULL); //sblocco segnali
                execve("./atomo", argv_figlio_2, environ);
                err_exit("Errore execve atomo figlio 2\n");
        }
    }
    else{
        scissioni[1].scorie++;
    }
    exit(EXIT_SUCCESS);
}