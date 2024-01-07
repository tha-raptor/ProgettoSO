#include "definizioni.h"

int term = 0; //nessuna condizione di terminazione
int flag = 1; //true
struct stat_inibitore *inibitore = NULL; //globale perchè l'handler deve vedere shm
char _fork[2] = {'/', '\0'};
char _assorb[2] = {'*', '\0'};
extern char **environ;

void print_stats(struct stat_scissione *scissioni, int semid_isimulaz){
    printf("\n-------\n");
    printf("Inib: %d", inibitore->flag_inib);
    printf("\n-------");
    printf("\n---STATS RELATIVE---\n");
    printf("attivazioni: %d\n", scissioni[1].attivazioni);
    printf("scorie: %d\n", scissioni[1].scorie);
    printf("energia prodotta: %d\n", scissioni[1].energia_prodotta);
    printf("energia consumata: %d\n", scissioni[1].energia_consumata);
    printf("scissioni: %d\n", scissioni[1].scissioni);
    printf("energia assorbita: %.2f\n", scissioni[1].energia_assorbita);
    printf("[ ");
    if(inibitore->num_operazioni_fork > 0 && inibitore->num_operazioni_assorb > 0){
        inibitore->num_operazioni_fork = 1;
        inibitore->num_operazioni_assorb = 1;
        printf("%s, %s", _fork, _assorb);
    }
    else if(inibitore->num_operazioni_fork > 0){
        inibitore->num_operazioni_fork = 1;
        inibitore->num_operazioni_assorb = 0;
        printf("%s", _fork);
    }
    else if(inibitore->num_operazioni_assorb > 0){
        inibitore->num_operazioni_fork = 0;
        inibitore->num_operazioni_assorb = 1;
        printf("%s", _assorb);
    }
    else{
        inibitore->num_operazioni_fork = 0;
        inibitore->num_operazioni_assorb = 0;
    }
    printf(" ]\n");

    //calcolo assolute
    scissioni[0].attivazioni += scissioni[1].attivazioni;
    scissioni[0].scorie += scissioni[1].scorie;
    scissioni[0].scissioni += scissioni[1].scissioni;
    scissioni[0].energia_prodotta += scissioni[1].energia_prodotta;
    scissioni[0].energia_consumata += scissioni[1].energia_consumata;
    scissioni[0].energia_assorbita += scissioni[1].energia_assorbita;

    int i = inibitore->num_operazioni_tot; //vecchie assolute
    int operazioni_rel = (inibitore->num_operazioni_fork + inibitore->num_operazioni_assorb); //operazioni relative (fork e assorbimento) a questo giro
    inibitore->num_operazioni_tot += operazioni_rel; //nuove operazioni assolute

    //calcolo operazioni assolute
    if(operazioni_rel >= 1){ //se inibitore è occorso almeno una volta
        scissioni[0].log_inibitore = (char **)realloc(scissioni[0].log_inibitore, inibitore->num_operazioni_tot * sizeof(char *));
        while( (i - 1) < (inibitore->num_operazioni_tot - 1)){ //entry array log_inibitore [0, operazioni - 1]
            if(inibitore->num_operazioni_fork && inibitore->num_operazioni_assorb){ //se sono occorse entrambi i tipi
                scissioni[0].log_inibitore[i] = _fork;
                i++; //qui avrò già allocato due posizioni
                scissioni[0].log_inibitore[i] = _assorb;
            }
            else if(inibitore->num_operazioni_fork){
                scissioni[0].log_inibitore[i] = _fork;
            }
            else{ //assorbimento
                scissioni[0].log_inibitore[i] = _assorb;
            }
            i++;
        }
    }
    //azzero relative
    //stats generali
    scissioni[1].attivazioni = 0;
    scissioni[1].energia_consumata = 0;
    scissioni[1].energia_prodotta = 0;
    scissioni[1].scorie = 0;
    scissioni[1].scissioni = 0;
    scissioni[1].energia_assorbita = 0;
    //stats inibitore
    inibitore->num_operazioni_fork = 0;
    inibitore->num_operazioni_assorb = 0;

    printf("\n---STATS ASSOLUTE---\n");
    printf("attivazioni: %d\n", scissioni[0].attivazioni);
    printf("scorie: %d\n", scissioni[0].scorie);
    printf("energia prodotta: %d\n", scissioni[0].energia_prodotta);
    printf("energia consumata: %d\n", scissioni[0].energia_consumata);
    printf("scissioni: %d\n", scissioni[0].scissioni);
    printf("energia assorbita: %.2f\n", scissioni[0].energia_assorbita);
    printf("[ ");
    i = 0;
    while(i < inibitore->num_operazioni_tot){ //stampo storico operazioni
        printf("%s, ", scissioni[0].log_inibitore[i]);
        i++;
    }
    printf(" ]");
    printf("\n--------------------\n");
}

int check_terminazioni(struct stat_scissione *scissioni){
    char *ENERGY_EXPLODE_THRESHOLD_ENV = getenv("ENERGY_EXPLODE_THRESHOLD");
    int energia_disponibile = scissioni[0].energia_prodotta - scissioni[0].energia_consumata - scissioni[0].energia_assorbita;
    if(energia_disponibile < 0)
        return -1; //blackout
    if(energia_disponibile > atoi(ENERGY_EXPLODE_THRESHOLD_ENV))
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

void uccidi_processi(){
    if (kill(-getpgid(getpid()), SIGTERM)==-1)
        err_exit("kill group_pid\n");
}

void crea_processi(pid_t parent_pid, int semid, int msgid, int shmid, int N_ATOMI_INIT){
    char *argv_semid = (char *)malloc(sizeof(char) * 11);
    sprintf(argv_semid, "%d", semid);
    char *argv_msgid = (char*)malloc(sizeof(char) * 11); 
    sprintf(argv_msgid, "%d", msgid);
    char *argv_shmid = (char*)malloc(sizeof(char) * 11); 
    sprintf(argv_shmid, "%d", shmid);
    char *argv_processi[4] = {argv_semid, argv_msgid, argv_shmid, NULL};

    switch(fork()){ //creazione attivatore
        case -1:
            uccidi_processi();
            print_protagonista_term("master -> attivatore", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./attivatore", argv_processi, environ);
            err_exit("Exceve attivatore");
    }
    
    switch(fork()){ //creazione alimentazione
        case -1:
            uccidi_processi();
            print_protagonista_term("master -> alimentazione", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./alimentazione", argv_processi, environ);
            err_exit("Exceve alimentazione");
    }

    switch(fork()){ //creazione inibitore
        case -1:
            uccidi_processi();
            print_protagonista_term("master -> inibitore", getpid());
            exit(EXIT_SUCCESS);
        case 0:
            execve("./inibitore", argv_processi, environ);
            err_exit("Exceve inibitore");
    }

    pid_t child_pid;
    char **argvAtomo = (char **)malloc(sizeof(char*) * 5); 
    char *NUM_ATOMICO = (char*)malloc(sizeof(char) * 3);

    for(int i = 0; i < N_ATOMI_INIT; i++){ //creazione N_ATOMI_INIT processi atomo
        srand((unsigned int) i + 1); // setto il seed
        if(getpid() == parent_pid)
            child_pid = fork();
        
        switch(child_pid){
            case -1:
                uccidi_processi();
                print_protagonista_term("master -> atomo_init", getpid());
                exit(EXIT_SUCCESS);
            case 0:  
                sprintf(NUM_ATOMICO, "%d", (rand() % N_ATOM_MAX)+1); //random tra 1 e N_ATOM_MAX
                argvAtomo[0] = NUM_ATOMICO;
                argvAtomo[1] = argv_msgid;
                argvAtomo[2] = argv_shmid;
                argvAtomo[3] = argv_semid;
                argvAtomo[4]= NULL;
                execve("./atomo", argvAtomo, environ); //argv = NUM_ATOMICO, envp = environ
                err_exit("Exceve atomo\n");
        }
    }
    //dealloco spazio in RAM
    free(argv_semid);
    free(argv_msgid);
    free(argv_shmid);
    free(argvAtomo);
    free(NUM_ATOMICO);
}

void handle_sig(){ //funzione per dichiarare e gestire tutti i segnali del master
    struct sigaction sa_sigint;
    sigset_t mask_sigint;
    sa_sigint.sa_handler = &handler_flag_inibitore;
    sa_sigint.sa_flags = 0;
    if(sigfillset(&mask_sigint) == -1)
        printf("error sigfillset su mask_sigint");
    if(sigdelset(&mask_sigint, SIGTERM) == -1)
         printf("error sigdelset su mask_sigint");
    sa_sigint.sa_mask = mask_sigint;

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
    if(sigaction(SIGINT, &sa_sigint, NULL) == -1)
        err_exit("sigaction per SIGINT");
}

void dealloca_risorse(int semid_isimulaz, int msgid, int shmid, int shmid_inib){
     if(semctl(semid_isimulaz, 0, IPC_RMID, NULL) == -1) //elimino il semaforo
        err_exit("remove semid_isimulaz_inizializzazione con IPC_RMID\n");

    if(msgctl(msgid, IPC_RMID, NULL) == -1) //elimino la coda di messaggi
        err_exit("remove msg_identificazine con IPC_RMID\n");

    if(shmctl(shmid, IPC_RMID, 0) == -1) //elimino shared memory stats generali
        err_exit("remove shared memory stats con IPC_RMID\n");

    if(shmctl(shmid_inib, IPC_RMID, 0) == -1) //elimino shared memory inib
        err_exit("remove shared memory inibitore con IPC_RMID\n");
}

int main(){
    struct my_msgbuf message_tutti,
    msg_inib_inib, msg_inib_attiv;
    pid_t master_pid = getpid();
    pid_t group_pid = getpgid(master_pid); //prendo il pid del gruppo
    int semid_isimulaz, msgid, shmid_stats, shmid_inib; //semid semafoto master
    union semun arg_simulazione, arg_inizializzazione;
    arg_inizializzazione.val = 0; //inizializzo semaforo inizializzazione a 0
    arg_simulazione.val = 0; //inizializzo semaforo simulazione a 0

    FILE *config_file = fopen("config.txt", "r");
    if(config_file == NULL)
        err_exit("fopen config.txt");
    
    char line[256];
    while (fgets(line, sizeof(line), config_file)!=NULL){ //leggo ogni riga del file di configurazione
        if(line[strlen(line)-1] == '\n')    //rimuovo il carattere \n sostituendolo con \0
            line[strlen(line)-1] = '\0';
    
        char *separator = strchr(line, '='); //cerco il carattere '='
        if(separator != NULL){      //divido la stringa in name e value
            *separator = '\0';       //in modo che contenga due stringhe
            char *name = line;
            char *value = separator + 1;
            //sovrascrivo le variabili d'ambiente con i valori letti dal file di configurazione
            if(setenv(name, value, 0) == -1){
                err_exit("error setenv");
            }
        }
    }
    fclose(config_file);

    printf("[master %d]\n", getpid());

    printf("\n---- LEGENDA INIB ----\n");
    printf("*: Assorbe energia\n");
    printf("/: Non fa forkare Attivatore\n");
    printf("----------------------\n");

    handle_sig(); //gestione segnali
    
    //SEMAFORI
    if((semid_isimulaz = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666 )) == -1)
        err_exit("Semget\n");
    if(semctl(semid_isimulaz, 0, SETVAL, arg_inizializzazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");
    if(semctl(semid_isimulaz, 1, SETVAL, arg_simulazione) == -1) //inizializzo semaforo inizializzazzione a 0
        err_exit("semctl con SETVAL su semid_isimulaz\n");
        
    //SHM
    if((shmid_stats = shmget(IPC_PRIVATE, sizeof(struct stat_scissione) * 2, IPC_CREAT | 0666)) == -1){
        uccidi_processi();
        dealloca_risorse(semid_isimulaz, msgid, shmid_stats, shmid_inib);
        err_exit("shmget stats");
    }
    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid_stats, NULL, 0);
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
    if((shmid_inib = shmget(IPC_PRIVATE, sizeof(struct stat_inibitore), IPC_CREAT | 0666)) == -1){
        uccidi_processi();
        dealloca_risorse(semid_isimulaz, msgid, shmid_stats, shmid_inib);
        err_exit("shmget inib");
    }

    inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->flag_inib = 0;
    inibitore->num_operazioni_fork = 0;
    inibitore->num_operazioni_assorb = 0;

    //MSG_QUEUE
    if((msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666 )) == -1){
        uccidi_processi();
        dealloca_risorse(semid_isimulaz, msgid, shmid_stats, shmid_inib);
        err_exit("Msgget\n");
    }
    char *N_ATOMI_INIT_ENV = getenv("N_ATOMI_INIT");
    if(N_ATOMI_INIT_ENV == NULL)
        err_exit("getenv N_ATOMI_INIT");
    int N_ATOMI_INIT = atoi(N_ATOMI_INIT_ENV);
    crea_processi(master_pid, semid_isimulaz, msgid, shmid_stats, N_ATOMI_INIT); //lancia tutti i processi

    //SI NOTIFICA SULLA CODA
    //IL PRIMO CHE RILEVA UNA MANCATA FORK, 
    //PRENDE IL MESSAGGIO E LO COMUNICA CON UN SEGNALE A MASTER CHE DEALLOCA TUTTO
    message_tutti.mtype = 2;
    sprintf(message_tutti.mtext, "%d", getpid());

    if(msgsnd(msgid, &message_tutti, sizeof(message_tutti), 0) == -1){
        uccidi_processi();
        dealloca_risorse(semid_isimulaz, msgid, shmid_stats, shmid_inib);
        err_exit("Msg snd identificazione master\n");
    }

    msg_inib_inib.mtype = 10; //messaggio ad inibitore con id shm inib
    sprintf(msg_inib_inib.mtext, "%d", shmid_inib);
    if(msgsnd(msgid, &msg_inib_inib, sizeof(msg_inib_inib), 0) == -1 ){
        uccidi_processi();
        dealloca_risorse(semid_isimulaz, msgid, shmid_stats, shmid_inib);
        err_exit("Msg snd master -> inib\n");
    }

    msg_inib_attiv.mtype = 11; //messaggio ad attivatore con id shm inib
    sprintf(msg_inib_attiv.mtext, "%d", shmid_inib);
    if(msgsnd(msgid, &msg_inib_attiv, sizeof(msg_inib_attiv), 0) == -1){
        uccidi_processi();
        dealloca_risorse(semid_isimulaz, msgid, shmid_stats, shmid_inib);
        err_exit("Msg snd master -> attivatore\n");
    }
    
    //SEMAFORO FINE INIT 
    int semaph_operation = N_ATOMI_INIT + 3;
    if(reserveSem(semid_isimulaz, 0, semaph_operation, 2) == -1)
        err_exit("reserveSem inizializzazione master\n");

    char si_no;
    printf("\nInibitore (y/n):");
    do{
        scanf("%c", &si_no);
    }while(si_no != 'y' && si_no != 'n');
    inibitore->flag_inib = si_no == 'y' ? 1: 0; //imposto flag variabile condivisa

    //SEMAFORO INIZIO SIMULAZ (RILASCIA TUTTI I PROCESSI)
    if(releaseSem(semid_isimulaz, 1, semaph_operation, 2) == -1)
        err_exit("releaseSem simulazione master\n");

    char *SIM_DURATION_ENV = getenv("SIM_DURATION");
    if(SIM_DURATION_ENV == NULL)
        err_exit("getenv SIM_DURATION");
    char *ENERGY_DEMAND = getenv("ENERGY_DEMAND");
    if(ENERGY_DEMAND == NULL)
        err_exit("getenv ENERGY_DEMAND");

    sigset_t new, old;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);

    //INIZIO SIMULAZIONE
    for(int i = 0; i < atoi(SIM_DURATION_ENV) && flag; i++){
        sigprocmask(SIG_BLOCK, &new, &old);  //blocco SIGINT solo quando deve stampare
        sleep(1);
        term = check_terminazioni(scissioni);
        if(term == 0 && flag != 0){ //la simulazione prosegue
            scissioni[1].energia_consumata = atoi(ENERGY_DEMAND); //preleva energia
            print_stats(scissioni, semid_isimulaz); //stampa statistiche
            sigprocmask(SIG_SETMASK, &old, NULL); //sblocco SIGINT
        }
        else {//blackout o explode
            flag = 0;
            sigprocmask(SIG_SETMASK, &old, NULL); //sblocco SIGINT
        }
    }

    //stampa terminazione
    if(flag == 1)
        printf("[master dealloco] terminazione: timeout\n");
    else if(term == -1)
        printf("[master dealloco] terminazione: blackout\n");
    else if(term == -2)
        printf("[master dealloco] terminazione: explode\n");
    else
        printf("[master dealloco] terminazione: meltdown\n");

    uccidi_processi(); //manda SIGTERM a tutti i suoi figli

    while (waitpid(-group_pid, NULL, 0)>0); //si assicura che tutti i figli (stesso group_pid) terminino

    dealloca_risorse(semid_isimulaz, msgid, shmid_stats, shmid_inib); //dealloca le risorse IPC

    exit(EXIT_SUCCESS);
}