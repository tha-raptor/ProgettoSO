# progettoSO

**Atomo:**

argv[0] = NUM_ATOMICO

argv[1] = msgid

argv[2] = shmid

argv[3] = semid (opzionale)

**Inibitore:**

argv[0] = NUM_ATOMICO

**Attivatore:**

argv[0] = semid

**Alimentazione:**

argv[0] = semid

argv[1] = msgid (deve poi passarlo agli atomi così come master)

**IDEA CODA DI MESSAGGI ATOMO**

L'idea è che gli atomi alla loro creazione si identificano in una coda di messaggi dove l'attivatore dirà agli atomi presenti in coda di dividersi. Prendendo i messaggi dalla coda attivatore la **svuoterà anche immediatamente**.

**MEMCONDIVISA SCISSIONI**

scissioni[0] = assoluto

scissioni[1] = relativo
