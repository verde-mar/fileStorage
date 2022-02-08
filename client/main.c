#include <stdio.h>
#include <check_errors.h>
#include <dispatcher.h>

int main(int argc, char *argv[]) {
    int err_dispatcher = dispatcher(argc, argv);

    /* Se c'e' stato un errore nella gestione delle richieste termina restituendo -1 */
    CHECK_OPERATION(err_dispatcher == -1, return -1);

    return 0;
}
