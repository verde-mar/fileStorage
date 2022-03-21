BEGIN {
    count_accept = 0
    count_create = 0
    count_create_lock = 0
    count_open_lock = 0
    count_open = 0
    count_unlock = 0
    count_close = 0
    count_open = 0
    count_lock = 0
    count_delete = 0
    max_size_reached = 0
    max_files_reached = 0
    replaced = 0
    bytes_read = 0
    count_read = 0
    count_write = 0
    bytes_write = 0
    avg_write = 0
    avg_read = 0
}

$1 == "Accept" { count_accept++ }
$1 == "Create" { count_create++ }
$1 == "Create_Lock" {count_create_lock++}
$1 == "Open_Lock" { count_open_lock++ }
$1 == "Open" { count_open++ }
$1 == "Unlock" {count_unlock++}
$1 == "Close" {count_close++}
$1 == "Open" {count_open++}
$1 == "Lock" {count_lock++}
$1 == "Delete" {count_delete++}
$1 == "Max_size_reached:"  {max_size_reached = $2}
$1 == "Max_files_reached:" {max_files_reached = $2}
$1 == "Replaced:" {replaced = $2}

$1 == "Read" {
    bytes_read += $2
    count_read++
}

$1 == "Write" {
    bytes_write += $2
    count_write++
}

END {
    print "Sono state accettate " count_accept " connessioni"
    printf "Sono state eseguite %d create_lock.\n", count_create_lock
    printf "Sono state eseguite %d unlock.\n", count_unlock
    printf "Sono state eseguite %d close.\n", count_close
    printf "Sono state eseguite %d open.\n", count_open
    printf "Sono state eseguite %d lock.\n", count_lock
    printf "Sono state eseguite %d delete.\n", count_delete
    printf "La massima size raggiunta dal file storage e' %d\n", max_size_reached
    printf "Il numero massimo di file raggiunto e' %d\n", max_files_reached
    printf "L'algoritmo di rimpiazzamento e' stato chiamato %d volte.\n", replaced

    if (bytes_write != 0)
        avg_write = (bytes_write)/count_write
    else 
        avg_write = 0;

    if (bytes_read != 0)
        avg_read = (bytes_read)/count_read
    else 
        avg_read = 0;

    printf "Sono state eseguite %d read, con numero medio di byte letti %d\n", count_read, avg_read
    printf "Sono state eseguite %d write, con numero medio di byte scritti %d\n", count_write, avg_write
}