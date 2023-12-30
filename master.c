#include "definizioni.h"

int term = 0; //nessuna condizione di terminazione
int flag = 1; //true
struct stat_inibitore *inibitore = NULL; //globale perchè l'handler lo deve vedere
int operation_assolute = 0;

void print_stats(struct stat_scissione *scissioni, int semid_isimulaz){
    printf("\n----\n");
    printf("Inib: %d", inibitore->flag_inib);
    printf("\n----\n");

    printf("\n---STATS RELATIVE---\n");
    printf("attivazioni: %d\n", scissioni[1].attivazioni);
    printf("scorie: %d\n", scissioni[1].scorie);
    printf("energia prodotta: %d\n", scissioni[1].energia_prodotta);
    printf("energia consumata: %d\n", scissioni[1].energia_consumata);
    printf("scissioni: %d\n", scissioni[1].scissioni);
    printf("energia assorbita: %d\n", scissioni[1].energia_assorbita);
    printf("[ ");
    if(inibitore->num_operazioni > 0){
        operation_assolute++;
        printf("operazione");
    } 
    printf(" ]\n");
    
    //calcolo assolute
    scissioni[0].attivazioni += scissioni[1].attivazioni;
    scissioni[0].scorie += scissioni[1].scorie;
    scissioni[0].scissioni += scissioni[1].scissioni;
    scissioni[0].energia_prodotta += scissioni[1].energia_prodotta;
    scissioni[0].energia_consumata += scissioni[1].energia_consumata;
    scissioni[0].energia_assorbita += scissioni[1].energia_assorbita;

    //azzero relative
    scissioni[1].attivazioni = 0;
    scissioni[1].energia_consumata = 0;
    scissioni[1].energia_prodotta = 0;
    scissioni[1].scorie = 0;
    scissioni[1].scissioni = 0;
    scissioni[1].energia_assorbita = 0;
    inibitore->num_operazioni = 0;

    int temp = operation_assolute;
    printf("\n---STATS ASSOLUTE---\n");
    printf("attivazioni: %d\n", scissioni[0].attivazioni);
    printf("scorie: %d\n", scissioni[0].scorie);
    printf("energia prodotta: %d\n", scissioni[0].energia_prodotta);
    printf("energia consumata: %d\n", scissioni[0].energia_consumata);
    printf("scissioni: %d\n", scissioni[0].scissioni);
    printf("energia assorbita: %d\n", scissioni[0].energia_assorbita);
    printf("[ ");
    while(temp > 0){
        printf("operazione, ");
        temp--;
    }
    printf(" ]\n");
    printf("\n--------------------\n");
}

int check_terminazioni(struct stat_scissione *scissioni){
    int energia_disponibile = scissioni[0].energia_prodotta - scissioni[0].energia_consumata;
    if(energia_disponibile < 0)
        return -1; //blackout
    if(energia_disponibile > ENERGY_EXPLODE_THRESHOLD)
        return -2; //explode
    return 0; //si prosegue con la simulazione
}

void ignora_sigterm(){
}

void handler_sigursdue(){
    flag = 0;
    if(kill(-getpgid(getpid()), SIGTERM) == -1)
        err_exit("kill group_pid\n");
}

void handler_flag_inibitore(){ //attivazione e spegnimento inibitore
    inibitore->flag_inib = !(inibitore->flag_inib);
}

void handler_neg(){
    printf("\n! Non puoi modificare inibitore !\n");
}

void uccidi_processi(int semid, int msgid, int shmid){
    if (kill(-getpgid(getpid()), SIGTERM)==-1)
        err_exit("kill group_pid\n");
}

void init_processi(pid_t parent_pid, int semid, int msgid, int shmid){
    char *argv_semid = (char *)malloc(sizeof(char) * 11); //inizializzazione malloc(?)
    sprintf(argv_semid, "%d", semid);
    char *argv_msgid = (char*)malloc(sizeof(char) * 20); 
    sprintf(argv_msgid, "%d", msgid);
    char *argv_shmid = (char*)malloc(sizeof(char) * 20); 
    sprintf(argv_shmid, "%d", shmid);
    char *envp[] = {NULL};
    char *argv_processi[4] = {argv_semid, argv_msgid, argv_shmid, NULL};

    switch(fork()){ //creazione attivatore
        case -1:
            uccidi_processi(semid, msgid, shmid);
            //err_exit("Fork attivatore");
            print_protagonista_term("master -> attivatore", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./attivatore", argv_processi, envp);
            err_exit("Exceve attivatore");
    }
    
    switch(fork()){ //creazione alimentazione
        case -1:
            uccidi_processi(semid, msgid, shmid);
            //err_exit("Fork alimentazione");
            print_protagonista_term("master -> alimentazione", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./alimentazione", argv_processi, envp);
            err_exit("Exceve alimentazione");
    }

    switch(fork()){ //creazione inibitore
        case -1:
            uccidi_processi(semid, msgid, shmid);
            //err_exit("Fork inibitore");
            print_protagonista_term("master -> inibitore", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./inibitore", argv_processi, envp);
            err_exit("Exceve inibitore");
    }

    pid_t child_pid;
    char **argvAtomo = (char **)malloc(sizeof(char*) * 5); 
    char *NUM_ATOMICO = (char*)malloc(sizeof(char) * 7);

    for(int i = 0; i < N_ATOMI_INIT; i++){ //creazione N_ATOMI_INIT processi atomo
        srand((unsigned int) i + 1); // setto il seed
        if(getpid() == parent_pid)
            child_pid = fork();
        
        switch(child_pid){
            case -1:
                uccidi_processi(semid, msgid, shmid);
                //err_exit("Fork atomo");
                print_protagonista_term("master -> atomo_init", getpid());
                exit(EXIT_SUCCESS);
            case 0:  
                sprintf(NUM_ATOMICO, "%d", (rand() % N_ATOM_MAX)+1); //random tra 1 e N_ATOM_MAX
                argvAtomo[0] = NUM_ATOMICO;
                argvAtomo[1] = argv_msgid;
                argvAtomo[2] = argv_shmid;
                argvAtomo[3] = argv_semid;
                argvAtomo[4]= NULL;
                execve("./atomo", argvAtomo, envp); //argv = NUM_ATOMICO, envp = NULL
                err_exit("Exceve atomo\n");
        }
    }
    free(argv_semid);
    free(argv_msgid);
    free(NUM_ATOMICO);
    free(argvAtomo);
}

void handle_sig(){ //Funzione per dichiarare e gestire tutti i segnali del master
   struct sigaction sa_sigterm, sa_sigursdue;
    sa_sigterm.sa_handler = &ignora_sigterm;
    sa_sigterm.sa_flags = 0;
    sa_sigursdue.sa_handler = &handler_sigursdue;
    sa_sigursdue.sa_flags = 0;
    sigset_t mask_sigterm, mask_sigursdue;
    if(sigemptyset(&mask_sigterm) == -1)
        err_exit("sigemptyset su mask_sigterm");
    sa_sigterm.sa_mask = mask_sigterm;
    if(sigemptyset(&mask_sigursdue) == -1)
        err_exit("sigemptyset su mask_sigursdue");
    sa_sigursdue.sa_mask = mask_sigursdue;

    if(sigaction(SIGTERM, &sa_sigterm, NULL) == -1)
        err_exit("sigaction per SIGTERM");
    if(sigaction(SIGUSR2, &sa_sigursdue, NULL) == -1)
        err_exit("sigaction per SIGURS2");
}

void dealloca_risorse(int semid_isimulaz, int msgid, int shmid, int shmid_inib){
     if(semctl(semid_isimulaz, 0, IPC_RMID, NULL) == -1) //elimino il semaforo
        err_exit("remove semid_isimulaz_inizializzazione con IPC_RMID\n");

    if(msgctl(msgid, IPC_RMID, NULL) == -1) //elimino la coda di messaggi
        err_exit("remove msg_identificazine con IPC_RMID\n");

    if(shmctl(shmid, IPC_RMID, 0) == -1) //elimino shared memory stats
        err_exit("remove shared memory stats con IPC_RMID\n");

    if(shmctl(shmid_inib, IPC_RMID, 0) == -1) //elimino shared memory inib
        err_exit("remove shared memory inibitore con IPC_RMID\n");
}

int main(){
    struct msgbuf message_tutti, notifica_attivatore,
    msg_inib_inib, msg_inib_atomo, msg_inib_attiv,msg_inib_alim;
    pid_t master_pid = getpid();
    pid_t group_pid = getpgid(master_pid); //prendo il pid del gruppo
    int semid_isimulaz, msgid, shmid, shmid_inib; //semid semafoto master
    union semun arg_simulazione, arg_inizializzazione;
    arg_inizializzazione.val = 0; //inizializzo semaforo inizializzazione a 0
    arg_simulazione.val = 0;
    printf("[master %d]\n", getpid());
    handle_sig();
    

    //SEMAFORI
    if((semid_isimulaz = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666 )) == -1)
        err_exit("Semget\n");
    if(semctl(semid_isimulaz, 0, SETVAL, arg_inizializzazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");
    if(semctl(semid_isimulaz, 1, SETVAL, arg_simulazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");
        
    //SHM
    if((shmid = shmget(IPC_PRIVATE, sizeof(struct stat_scissione) * 2, IPC_CREAT | 0666)) == -1)
        err_exit("shmget stats");
    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);
    scissioni[0].attivazioni = 0;
    scissioni[0].energia_consumata = 0;
    scissioni[0].energia_prodotta = 0;
    scissioni[0].scissioni = 0;
    scissioni[0].scorie = 0;
    scissioni[0].energia_assorbita = 0;
    scissioni[0].log_inibitore = NULL;
    scissioni[1].attivazioni = 0;
    scissioni[1].energia_consumata = 0;
    scissioni[1].energia_prodotta = 0;
    scissioni[1].scissioni = 0;
    scissioni[1].scorie = 0;
    scissioni[1].energia_assorbita = 0;
    scissioni[1].log_inibitore = NULL;
   
    /*for(int i = 0; i < 10; i++){
        printf("log_inib[%d]: %s\n", i, scissioni[1].log_inibitore[i]);
    }*/

    if((shmid_inib = shmget(IPC_PRIVATE, sizeof(struct stat_inibitore), IPC_CREAT | 0666)) == -1)
        err_exit("shmget inib");
    inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->flag_inib = 0;
    inibitore->num_operazioni = 0;

    //MSG_QUEUE
    if((msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666 )) == -1)
        err_exit("Msgget\n");
    init_processi(master_pid, semid_isimulaz, msgid, shmid); //inizializza tutti i processi

    //SI NOTIFICA SULLA CODA
    //IL PRIMO CHE RILEVA UNA MANCATA FORK, 
    //PRENDE IL MESSAGGIO E LO COMUNICA CON UN SEGNALE A MASTER CHE DEALLOCA TUTTO
    message_tutti.mtype = 2;
    sprintf(message_tutti.mtext, "%d", getpid());
    if(msgsnd(msgid, &message_tutti, sizeof(message_tutti), 0) == -1) //master si identifica con mtype = 2
        err_exit("Msg snd identificazione master\n");

    msg_inib_inib.mtype = 10; //messaggio a inibitore con idshm inib
    sprintf(msg_inib_inib.mtext, "%d", shmid_inib);
    if(msgsnd(msgid, &msg_inib_inib, sizeof(msg_inib_inib), 0) == -1 )
        err_exit("Msg snd master -> inib\n");

    msg_inib_attiv.mtype = 11; //messaggio a attivatore con idshm inib
    sprintf(msg_inib_attiv.mtext, "%d", shmid_inib);
    if(msgsnd(msgid, &msg_inib_attiv, sizeof(msg_inib_attiv), 0) == -1 )
        err_exit("Msg snd master -> attivatore\n");
    
    //MASTER ASPETTA CHE ATTIVATORE SI IDENTIFICHI (MTYPE = 3)
    if(msgrcv(msgid, &notifica_attivatore, MSG_SIZE_IDENT, 3, MSG_NOERROR) == -1){
        uccidi_processi(semid_isimulaz, msgid, shmid);
        while (waitpid(-group_pid, NULL, 0)>0);
        dealloca_risorse(semid_isimulaz, msgid, shmid, shmid_inib);
        exit(EXIT_FAILURE);
    }
    int pid_attivatore = atoi(notifica_attivatore.mtext);

    //SEMAFORO FINE INIT 
    int semaph_operation = N_ATOMI_INIT + 3;
    if(reserveSem(semid_isimulaz, 0, semaph_operation, 2) == -1)
        err_exit("reserveSem inizializzazione master\n");

    //PRE-INIZIO SIMULAZ FLAG INIBITORE
    char si_no; //Esistenza inibitore
    printf("Inibitore (y/n):");
    do{
        scanf("%c", &si_no);
    }while(si_no != 'y' && si_no != 'n');
    inibitore->flag_inib = si_no == 'y' ? 1: 0;

    struct sigaction sa_sigint;
    sigset_t mask_sigint;

    if(inibitore->flag_inib) //se la prima volta risposta
        sa_sigint.sa_handler = &handler_flag_inibitore;
    else
        sa_sigint.sa_handler = &handler_neg;

    sa_sigint.sa_flags = 0;
    if(sigemptyset(&mask_sigint) == -1)
        err_exit("sigemptyset su mask_sigint");
    sa_sigint.sa_mask = mask_sigint;

    if(sigaction(SIGINT, &sa_sigint, NULL) == -1)
        err_exit("sigaction per SIGINT");

    //SEMAFORO INIZIO SIMULAZ (RILASCIA TUTTI I PROCESSI)
    if(releaseSem(semid_isimulaz, 1, semaph_operation, 2) == -1)
        err_exit("releaseSem simulazione master\n");

    //INIZIO SIMULAZIONE
    for(int i = 0; i < SIM_DURATION && flag; i++){
        sleep(1);
        term = check_terminazioni(scissioni);
        if(term == 0 && flag != 0){ //la simulazione prosegue
            scissioni[1].energia_consumata = ENERGY_DEMAND;
            print_stats(scissioni, semid_isimulaz);
        }
        else //blackout o explode
            flag = 0;
    }
    if(flag == 1)
        printf("[master dealloco] terminazione: timeout\n");
    else if(term == -1)
        printf("[master dealloco] terminazione: blackout\n");
    else if(term == -2)
        printf("[master dealloco] terminazione: explode\n");
    else
        printf("[master dealloco] terminazione: meltdown\n");

    uccidi_processi(semid_isimulaz, msgid, shmid);

    while (waitpid(-group_pid, NULL, 0)>0);

    //printf("num_operazioni: %d", inibitore->num_operazioni); va

    dealloca_risorse(semid_isimulaz, msgid, shmid, shmid_inib);

    exit(EXIT_SUCCESS);
}