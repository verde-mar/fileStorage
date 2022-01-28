#include <stdio.h>
#include <check_errors.h>
#include <dispatcher.h>

int main(int argc, char *argv[]) {
    int err_dispatcher = dispatcher(argc, argv);

    /* Se c'e' stato un qualunque errore nella gestione delle richieste termina stampando un messaggio di errore */
    CHECK_OPERATION(err_dispatcher == -1, return -1);
    /* Se tutte le richieste sono state eseguite con successo, termina stampando un messaggio */
    CHECK_OPERATION(err_dispatcher == 0, fprintf(stdout, "\nTutte le richieste sono state eseguite.\n"));

    return 0;
}
