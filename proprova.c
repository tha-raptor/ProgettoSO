#include "definizioni.h"

void handler(){
    int i = 0;
    while(1){
        printf(".\n");
    }
}

int main(){
    struct sigaction sa_sigint;
    sigset_t mask_sigint;

    sigset_t new, old;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);
    sigprocmask(SIG_BLOCK, &new, &old);

    sleep(5);

    printf("fine\n");
    sigprocmask(SIG_SETMASK, &old, NULL);

    printf("aooo\n");
    sleep(5);
    exit(EXIT_SUCCESS);
}