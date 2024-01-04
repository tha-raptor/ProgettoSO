#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "libscissione.h"

// TERMINAZIONE
#define SIM_DURATION 15
#define ENERGY_EXPLODE_THRESHOLD 15000
//----------------------
// INIZIALIZZAZIONE
#define N_ATOMI_INIT 150
#define N_ATOM_MAX 30
//----------------------
//GENERALI
#define ENERGY_DEMAND 10
#define STEP_ALIMENTAZIONE 900000000 //0.9
#define STEP_ATTIVATORE 100000
#define MIN_N_ATOMICO 10
#define MSG_SIZE_IDENT 128
#define N_NUOVI_ATOMI 10

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
    char **log_inibitore;
    float energia_assorbita; 
};

struct stat_inibitore{
    int flag_inib;
    int num_operazioni_fork;
    int num_operazioni_assorb;
    int num_operazioni_tot;
    int pid_attivatore;
    int pid_inibitore;
};
