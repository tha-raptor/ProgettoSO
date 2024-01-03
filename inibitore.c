#include "definizioni.h"

struct stat_inibitore *inibitore = NULL;

void handler_sigurs_uno(){
}

void handler_flag_inibitore(){
}

void handle_sig(){
    struct sigaction sa_sigurs,sa_sigint;
    sa_sigurs.sa_handler = &handler_sigurs_uno;
    sa_sigurs.sa_flags = 0;
    sa_sigint.sa_handler = &handler_flag_inibitore;
    sa_sigint.sa_flags = 0;

    sigset_t mask_sigurs_uno, mask_sigint;
    if(sigemptyset(&mask_sigurs_uno) == -1)
        err_exit("sigemptyset su mask_sigurs");

    sa_sigurs.sa_mask = mask_sigurs_uno;

    if(sigaction(SIGUSR1, &sa_sigurs, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");

    if(sigemptyset(&mask_sigint) == -1)
        err_exit("sigemptyset su mask_sigurs");

    sa_sigint.sa_mask = mask_sigint;

    if(sigaction(SIGINT, &sa_sigint, NULL) == -1)  //imposto un handler da svolgere all'arrivo di SIGINT da parte di attivatore
        err_exit("sigaction su SIGURS1\n");    
}

int main(int argc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    struct msgbuf msg_shared_inib, msg_fork;
    msg_fork.mtype = 20;
    struct msgbuf lettura_identificazione_handl;
   
    struct stat_scissione *scissioni = (struct stat_scissione *)shmat(shmid, NULL, 0);

    handle_sig();

    if(msgrcv(msgid, &msg_shared_inib, MSG_SIZE_IDENT, 10, MSG_NOERROR) == -1)
        err_exit("Msgrcv master -> inibitore");

    int shmid_inib = atoi(msg_shared_inib.mtext);
    inibitore = (struct stat_inibitore *)shmat(shmid_inib, NULL, 0);
    inibitore->pid_inibitore = getpid();

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem inizializzazione inibitore\n");
    //printf("[inibitore %d] ho inizializzato, aspetto...\n", getpid());
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione inibitore");
    //printf("[inibitore] inizio anche io simulazione\n");

    char *operazione = malloc(strlen("operazione") + 1); // +1 per il terminatore nullo
    strcpy(operazione, "operazione");

    float livello_energia;
    float soglia_massima = 0.75;
    struct msgbuf msg_energy;
    
    while(1){
        if(inibitore->flag_inib){
            pause();
            livello_energia = ((float)(scissioni[0].energia_prodotta - scissioni[0].energia_consumata) / ENERGY_EXPLODE_THRESHOLD);
            //printf("livello_energia: %f\n", livello_energia);
            if(livello_energia < soglia_massima){ //tutto a posto
                while(msgrcv(msgid, &msg_energy, MSG_SIZE_IDENT, 30, MSG_NOERROR | IPC_NOWAIT) != -1){
                    scissioni[1].energia_assorbita += 0.2 * atoi(msg_energy.mtext);
                }
                kill(inibitore->pid_attivatore, SIGUSR1);
            }
            else{
                //atomo deve dirgli quanta energia ha prodotto nella scissione
                while(msgrcv(msgid, &lettura_identificazione_handl, MSG_SIZE_IDENT, 1, MSG_NOERROR | IPC_NOWAIT) != -1){
                    scissioni[1].attivazioni += 1;
                    scissioni[1].scorie += 1;
                }
                kill(inibitore->pid_attivatore, SIGUSR2); //situazione pericolosa
                inibitore->num_operazioni++; //un'operazione in più
            }
        }
        else{
            //o così oppure atomo non manda quando inib è 'n'
            while(msgrcv(msgid, &msg_energy, MSG_SIZE_IDENT, 30, MSG_NOERROR | IPC_NOWAIT) != -1);
        }
    }
    exit(EXIT_SUCCESS);
}