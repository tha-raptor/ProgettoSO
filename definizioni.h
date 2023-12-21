#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

// TERMINAZIONE
#define SIM_DURATION 10
#define ENERGY_EXPLODE_THRESHOLD 10000
//----------------------
// INIZIALIZZAZIONE
#define N_ATOMI_INIT 10
#define N_ATOM_MAX 30
//----------------------
//GENERALI
#define ENERGY_DEMAND 10
#define STEP_NANO 500000000 //0.1 secondi
#define MIN_N_ATOMICO 10
#define MSG_SIZE_IDENT 128
#define N_NUOVI_ATOMI 10

//N_INIT = 2 -> 8, 15

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
