#include "definizioni.h"
#include "libscissione.h"
#include <string.h>

extern int releaseSem(int, int, int, int);
extern int reserveSem(int, int, int, int);
extern void err_exit(char *);

int main(int argc, char **argv){
    int semid = atoi(argv[0]);
    int msgid = atoi(argv[1]);

    //manuale linux
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;          // secondi
    sleep_time.tv_nsec = STEP_NANO; // nanosecondi

    if(releaseSem(semid, 0, 1, 2) == -1)
        err_exit("releaseSem\n");
     printf("[alimentazione %d] ho inizializzato, aspetto...\n", getpid());
    if(reserveSem(semid, 1, 1, 2) == -1)
        err_exit("reserveSem simulazione\n");
    printf("[alimentazione] inizio anche io simulazione\n");

    char *envp[] = {NULL};
    char **argvAtomo = (char **)malloc(sizeof(char*) * 3); 
    char *argv_semid = (char*)malloc(sizeof(char) * 20);
    sprintf(argv_semid, "%d", semid);
    char *argv_msgid = (char*)malloc(sizeof(char) * 20);
    sprintf(argv_msgid, "%d", msgid);
    char *NUM_ATOMICO = (char*)malloc(sizeof(char) * 7);

    pid_t parent_pid = getpid();
    pid_t child_pid;
    int counter_creazione = 0;

    for (; ;)
    {
        nanosleep(&sleep_time, NULL);  //ogni STEP_NANO
        while(counter_creazione < N_NUOVI_ATOMI){ //fino a quando non raggiungo N_NUOVI_ATOMI
            printf("\n[alimentazione %d] creo un atomo\n", getpid());
            srand((unsigned int) counter_creazione + 1); // setto il seed
            if(getpid() == parent_pid)
                child_pid = fork();
            switch(child_pid){
                case -1:
                    err_exit("Fork atomo");
                case 0:  
                    sprintf(NUM_ATOMICO, "%d", (rand() % N_ATOM_MAX)+1); //random tra 1 e N_ATOM_MAX
                    argvAtomo[0] = NUM_ATOMICO;
                    argvAtomo[1] = argv_msgid;
                    argvAtomo[2] = (char *)NULL;
                    execve("./atomo", argvAtomo, envp); //argv[0] = NUM_ATOMICO, argv[1] = msgid, envp = NULL
                    err_exit("Exceve atomo");
            }
        }
        counter_creazione = 0; //azzero -> pronto a ricreare altri N_NUOVI_ATOMI
    }
    exit(EXIT_SUCCESS);
}