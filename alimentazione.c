#include "definizioni.h"

char **environ;

void handler_FlagInibitore(){ 
}

void handle_sig(){
    struct sigaction sa_SIGINT;
    sa_SIGINT.sa_handler = &handler_FlagInibitore;
    sa_SIGINT.sa_flags = 0;
    sigset_t mask_SIGINT;
    if(sigfillset(&mask_SIGINT) == -1) //Segnale in input
        err_exit("sigemptyset su mask_SIGINT");
    if(sigdelset(&mask_SIGINT, SIGTERM) == -1) //Segnale in input
        err_exit("sigemptyset su mask_SIGINT");
    sa_SIGINT.sa_mask = mask_SIGINT;
    if(sigaction(SIGINT, &sa_SIGINT, NULL) == -1)
        err_exit("sigaction per SIGINT");     
}

int main(int argc, char **argv){
    int error_msgrcv;
    struct msgbuf message;
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);

    //char *envp[] = {NULL};
    char **argvAtomo = (char **)malloc(sizeof(char*) * 4); 
    char *argv_msgid = (char*)malloc(sizeof(char) * 11);
    sprintf(argv_msgid, "%d", msgid);
    char *argv_shmid = (char*)malloc(sizeof(char) * 11);
    sprintf(argv_shmid, "%d", shmid);
    char *NUM_ATOMICO = (char*)malloc(sizeof(char) * 3);
    pid_t parent_pid = getpid();
    pid_t child_pid;
    int counter_creazione = 0;  

    handle_sig();

    char *N_NUOVI_ATOMI_ENV = getenv("N_NUOVI_ATOMI");
    if (N_NUOVI_ATOMI_ENV == NULL)
        err_exit("getenv N_NUOVI_ATOMI");
    int N_NUOVI_ATOMI = atoi(N_NUOVI_ATOMI_ENV);
    //manuale linux
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;          // secondi
    sleep_time.tv_nsec = STEP_ALIMENTAZIONE; // nanosecondi

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione alimentazione\n");
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione alimentazione\n");

    for (; ;) {
        nanosleep(&sleep_time, NULL);  //ogni STEP_NANO, check params
        while(counter_creazione < N_NUOVI_ATOMI){ //fino a quando non raggiungo N_NUOVI_ATOMI
            srand((unsigned int) counter_creazione + 1); // setto il seed
            if(getpid() == parent_pid)
                child_pid = fork();  
            switch(child_pid){
                case -1:
                    error_msgrcv = msgrcv(msgid, &message, MSG_SIZE_IDENT, 2, MSG_NOERROR);
                    if(error_msgrcv == -1){ //se ha dato errore la msgrcv
                        if(errno != ENOMSG) //se l'errore è diverso da "non ci sono più messaggi"
                            err_exit("failure msgrcv");
                    }
                    print_protagonista_term("alimentazione -> atomo", getpid());
                    if(kill(atoi(message.mtext), SIGUSR2 ) == -1)
                        err_exit("kill verso master\n");
                    exit(EXIT_SUCCESS);
                case 0:  
                    sprintf(NUM_ATOMICO, "%d", (rand() % N_ATOM_MAX)+1); //random tra 1 e N_ATOM_MAX
                    argvAtomo[0] = NUM_ATOMICO;
                    argvAtomo[1] = argv_msgid;
                    argvAtomo[2] = argv_shmid;
                    argvAtomo[3]= NULL;
                    execve("./atomo", argvAtomo, environ); //argv[0] = NUM_ATOMICO, argv[1] = msgid, envp = environ
                    err_exit("Exceve atomo");
                default:
                     counter_creazione++;
            }
        }
        counter_creazione = 0; //azzero -> pronto a ricreare altri N_NUOVI_ATOMI
    }
    
    exit(EXIT_SUCCESS);
}