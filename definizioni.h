#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

// TERMINAZIONE
#define SIM_DURATION 10
//----------------------
// INIZIALIZZAZIONE
#define N_ATOMI_INIT 3
#define N_ATOM_MAX 30
//----------------------
//GENERALI
#define ENERGY_DEMAND 15
#define STEP_NANO 900000000 //0.1 secondi
#define MIN_N_ATOMICO 10
#define MSG_SIZE_IDENT 128
#define N_NUOVI_ATOMI 1


//Dichiarazione semun MacOS-VM
#if defined(__linux__)
union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    #if defined(__linux__)
    struct seminfo *__buf;
    #endif
}; 
#endif

struct msgbuf{
    long mtype;
    char mtext[MSG_SIZE_IDENT];
};

struct stat_scissione{
    int attivazioni;
    int scissioni;
    int energia_prodotta;
    int energia_consumata;
    int scorie;
    //int energia_assorbita; //da inibitore
    //int log_inibitore; //da inibitore
};
//----------------------
