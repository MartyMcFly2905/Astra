#pragma once

/// Operazione di swap richiesta al modulo I/O.
enum astra_swap_operazione_io {
	ASTRA_IO_SWAP_OUT,
	ASTRA_IO_SWAP_IN
};

/// Richiesta minima che il modulo sistema prepara per il modulo I/O.
struct astra_swap_io_request {
	astra_swap_operazione_io operazione;
	vaddr ind_virtuale;
	paddr frame_fisico;
	natq slot;
	natl primo_settore;
	natb quanti_settori;
};

