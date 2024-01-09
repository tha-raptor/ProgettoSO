#include "definizioni.h"

extern char **environ;

void handler_sigurs_uno(){
}

void handler_sigterm(){ 
    exit(EXIT_SUCCESS);
}

void handle_sig(){
    struct sigaction sa_sigusr, sa_sigint, sa_sigterm;
    sa_sigusr.sa_handler = &handler_sigurs_uno;
    sa_sigusr.sa_flags = 0;
    sa_sigint.sa_handler = (void *)&handler_ignora_sigint;
    sa_sigint.sa_flags = 0;
    sa_sigterm.sa_handler = &handler_sigterm;
    sa_sigterm.sa_flags = 0;  

    //blocco tutti i segnali durante gli handler, tranne per SIGTERM
    sigset_t mask_sigurs_uno, mask_sigint, mask_sigterm;
    if(sigfillset(&mask_sigurs_uno) == -1)
        err_exit("sigfillset su mask_sigurs");
    if(sigdelset(&mask_sigurs_uno, SIGTERM) == -1)
        err_exit("sigdelset su mask_sigurs");
    if(sigfillset(&mask_sigint) == -1)
        err_exit("sigfillyset su mask_sigurs");
    if(sigdelset(&mask_sigint, SIGTERM) == -1)
        err_exit("sigdelset su mask_sigurs");
    if(sigemptyset(&mask_sigterm) == -1)
        err_exit("sigemptyset su mask_sigterm");

    sa_sigint.sa_mask = mask_sigint;
    sa_sigusr.sa_mask = mask_sigurs_uno;
    sa_sigterm.sa_mask = mask_sigterm;

    if(sigaction(SIGUSR1, &sa_sigusr, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");
    if(sigaction(SIGTERM, &sa_sigterm, NULL) == -1)
        err_exit("sigaction per SIGTERM");
    if(sigaction(SIGINT, &sa_sigint, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");  
}

int main(int argc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    struct my_msgbuf msg_shared_inib;
    struct my_msgbuf lettura_identificazione_handl;
    int ENERGY_EXPLODE_THRESHOLD = atoi(getenv("ENERGY_EXPLODE_THRESHOLD"));

    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);

    handle_sig();

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 10, MSG_NOERROR) == -1) //shmid di inibitore da master
        err_exit("Msgrcv master -> inibitore");

    int shmid_inib = atoi(msg_shared_inib.mtext);
    struct stat_inibitore *inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_inibitore = getpid(); //si identifica sulla memoria condivisa

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione inibitore\n");
    //fine inizializzazione
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione inibitore");
    //inizio simulazione
    
    float livello_energia;
    float soglia_massima = ((float) SOGLIA_PERICOLOSA / 100);
    struct my_msgbuf msg_energy; //messaggio che ospiterà l'energia assorbita

    sigset_t new, old;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);
    sigprocmask(SIG_BLOCK, &new, &old); //blocco SIGINT per evitare
    
    while(1){
        pause();
        if(inibitore->flag_inib){ //se inib acceso
            livello_energia = ((float)(scissioni[0].energia_prodotta - scissioni[0].energia_consumata - scissioni[0].energia_assorbita) / ENERGY_EXPLODE_THRESHOLD);
            if(livello_energia < soglia_massima){ //livello sotto controllo
                inibitore->num_operazioni_assorb++;
                while(msgrcv(msgid, &msg_energy, MSG_SIZE_IDENT, 30, MSG_NOERROR | IPC_NOWAIT) != -1){ //assorbi energia (messaggi con mtype = 30)
                    scissioni[1].energia_assorbita += 0.2 * atoi(msg_energy.mtext);
                }
                kill(inibitore->pid_attivatore, SIGUSR1); //segnale -> attivatore per far forkare
            }
            else{
                inibitore->num_operazioni_fork++;
                kill(inibitore->pid_attivatore, SIGUSR2); //segnale -> attivatore per non far forkare
            }
        }
        else{ //se inib spento
            while(msgrcv(msgid, &msg_energy, MSG_SIZE_IDENT, 30, MSG_NOERROR | IPC_NOWAIT) != -1); //svuota la coda di messaggi perchè non assorbe energia
            kill(inibitore->pid_attivatore, SIGUSR1); //segnale -> attivatore per far forkare
        }
    }
    exit(EXIT_SUCCESS);
}