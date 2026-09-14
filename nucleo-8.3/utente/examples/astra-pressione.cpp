#include <all.h>

const natq DIM_BLOCCO_STACK = 48 * KiB;

void processo_memoria(natq a)
{
	volatile char area[DIM_BLOCCO_STACK];
	volatile natq controllo = 0;

	// Tocchiamo una pagina alla volta: ogni accesso forza il demand paging
	// della pila utente, senza usare heap o primitive nuove.
	for (natq i = 0; i < DIM_BLOCCO_STACK; i += DIM_PAGINA) {
		area[i] = static_cast<char>(a);
		controllo += area[i];
	}

	meminfo m = getmeminfo();
	printf("proc %lu: frame liberi %u, controllo %lu\n",
			a, m.num_frame_liberi, controllo);

	delay(200);
	terminate_p();
}

extern "C" void main()
{
	for (natq i = 0; i < 800; i++) { //  Questo dovrebbe richiedere circa 9600 frame per i soli stack 
									 //  superando gli 8192(MEM_TOT=32MiB) disponibili
  									 //  costringendo il nucleo a gestire l'esaurimento della memoria.
		natl id = activate_p(processo_memoria, i, 4, LIV_UTENTE);

		if (id == 0xFFFFFFFF) {
			meminfo m = getmeminfo();
			printf("activate_p fallita dopo %lu processi, frame liberi %u\n",
					i, m.num_frame_liberi);
			break;
		}
	}

	terminate_p();
}
