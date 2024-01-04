#include "definizioni.h"

void handler_FlagInibitore(){ 
    
}

void handle_sig(){
    //sengale Cambiamento Inibitore

    struct sigaction sa_SIGINT;
    sa_SIGINT.sa_handler = &handler_FlagInibitore;
    sa_SIGINT.sa_flags = 0;
    sigset_t mask_SIGINT;
    if(sigemptyset(&mask_SIGINT) == -1) //Segnale in input
        err_exit("sigemptyset su mask_SIGINT");
    sa_SIGINT.sa_mask = mask_SIGINT;
    if(sigaction(SIGINT, &sa_SIGINT, NULL) == -1)
        err_exit("sigaction per SIGINT");     
}

int main(int argc, char **argv, char **envp){
    extern char **environ;

    int error_msgrcv;
    struct msgbuf message;
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);

    //char *envp[] = {NULL};
    char **argvAtomo = (char **)malloc(sizeof(char*) * 4); 
    char *argv_msgid = (char*)malloc(sizeof(char) * 20);
    sprintf(argv_msgid, "%d", msgid);
    char *argv_shmid = (char*)malloc(sizeof(char) * 20);
    sprintf(argv_shmid, "%d", shmid);
    char *NUM_ATOMICO = (char*)malloc(sizeof(char) * 7);
    pid_t parent_pid = getpid();
    pid_t child_pid;
    int counter_creazione = 0;  

    handle_sig();

    //manuale linux
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;          // secondi
    char *STEP_ALIMENTAZIONE_ENV = getenv("STEP_ALIMENTAZIONE");
    if (STEP_ALIMENTAZIONE_ENV == NULL)
        err_exit("getenv STEP_ALIMENTAZIONE");
    sleep_time.tv_nsec = atoi(STEP_ALIMENTAZIONE_ENV); // nanosecondi

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione alimentazione\n");
    //printf("[alimentazione %d] ho inizializzato, aspetto...\n", getpid());
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione alimentazione\n");
    //printf("[alimentazione] inizio anche io simulazione\n");

    char *N_ATOM_MAX_ENV = getenv("N_ATOM_MAX");
    if (N_ATOM_MAX_ENV == NULL)
        err_exit("getenv N_ATOM_MAX");
    int N_ATOM_MAX_INT = atoi(N_ATOM_MAX_ENV);

    char *N_NUOVI_ATOMI_ENV = getenv("N_NUOVI_ATOMI");
    if (N_NUOVI_ATOMI_ENV == NULL)
        err_exit("getenv N_NUOVI_ATOMI");
    int N_NUOVI_ATOMI_INT = atoi(N_NUOVI_ATOMI_ENV);

    char *MSG_SIZE_IDENT_ENV = getenv("MSG_SIZE_IDENT");
    if (MSG_SIZE_IDENT_ENV == NULL)
        err_exit("getenv MSG_SIZE_IDENT");
    int MSG_SIZE_IDENT_INT = atoi(MSG_SIZE_IDENT_ENV);
    
    for (; ;) {
        nanosleep(&sleep_time, NULL);  //ogni STEP_NANO, check params
        while(counter_creazione < N_NUOVI_ATOMI_INT){ //fino a quando non raggiungo N_NUOVI_ATOMI
            //printf("\n[alimentazione %d] creo un atomo\n", getpid());
            srand((unsigned int) counter_creazione + 1); // setto il seed
            if(getpid() == parent_pid)
                child_pid = fork();  
            switch(child_pid){
                case -1:
                    error_msgrcv = msgrcv(msgid, &message, MSG_SIZE_IDENT_INT, 2, MSG_NOERROR);
                    if(error_msgrcv == -1){ //se ha dato errore la msgrcv
                        if(errno != ENOMSG) //se l'errore è diverso da "non ci sono più messaggi"
                            err_exit("failure msgrcv"); //esci
                    }
                    print_protagonista_term("alimentazione -> atomo", getpid());
                    if(kill(atoi(message.mtext), SIGUSR2 ) == -1)
                        err_exit("kill verso master\n");
                    exit(EXIT_SUCCESS);
                case 0:  
                    sprintf(NUM_ATOMICO, "%d", (rand() % N_ATOM_MAX_INT)+1); //random tra 1 e N_ATOM_MAX
                    argvAtomo[0] = NUM_ATOMICO;
                    argvAtomo[1] = argv_msgid;
                    argvAtomo[2] = argv_shmid;
                    argvAtomo[3]= NULL;
                    execve("./atomo", argvAtomo, environ); //argv[0] = NUM_ATOMICO, argv[1] = msgid, environ = array di variabili d'ambiente
                    err_exit("Exceve atomo");
                default:
                     counter_creazione++;
            }
        }
        counter_creazione = 0; //azzero -> pronto a ricreare altri N_NUOVI_ATOMI
    }
    
    exit(EXIT_SUCCESS);
}