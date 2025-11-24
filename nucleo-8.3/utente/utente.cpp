#include <all.h>

void main() {
    // La semplice entrata e uscita da una funzione (come main)
    // richiede l'uso implicito della pila, che causerà il Page Fault.
    
    // Possiamo aggiungere un log esplicito per la verifica.
    printf("Astra: main_utente avviato.\n"); 

    terminate_p(); // Chiude il processo
}