# Astra - Demand Paging on Didactic Kernel

*Read this in other languages: [English](#english) | [Italiano](#italiano)*

---

<a name="english"></a>
## 🇬🇧 English

**Astra** is an extension of the `nucleo-8.3` didactic kernel (developed at the University of Pisa) designed to implement a real virtual memory mechanism: **Demand Paging**.

The project addresses and solves a practical issue: avoiding the static allocation of the entire stack for user processes upon their creation, by delaying the physical frame allocation until it is actually needed (i.e., at the first page access).

### Architecture and Implemented Features

The system is structured in two main components: an active one (the actual demand paging) and an architectural one (the swap-out pipeline).

#### 1. Real Demand Paging
A user process starts with a valid virtual base address for its stack, but no physical memory is associated with it. The first `push` or stack access generates a *Page Fault* (Exception 14).

The kernel intercepts this specific exception, allocates the physical frame, and maps it on-the-fly:

```cpp
// Inside gestore_page_fault(vaddr fault_addr)
vaddr page_base = fault_addr & ~(DIM_PAGINA - 1);
paddr new_frame = alloca_frame();

memset(voidptr_cast(new_frame), 0, DIM_PAGINA); // Security zero-fill

// On-demand mapping in the Page Table
vaddr mapped_until = map(esecuzione->cr3, page_base, page_base + DIM_PAGINA, BIT_US | BIT_RW, get_frame);

marca_frame(new_frame, FRAME_PAGINA_UTENTE, esecuzione, page_base);
invalida_entrata_TLB(page_base);
```

#### 2. Diagnostic Swap Pipeline (OOM Management)
If the physical memory (RAM) is exhausted, Astra does not crash uncontrollably. Instead, it triggers a *Swap-Out* pipeline designed to keep the separation between the System module and the I/O driver module clean:

- **Reverse Mapping and Metadata:** Every physical frame knows which process it belongs to and at which virtual address it is mapped.
- **Victim Selection:** A circular scan of replaceable user frames is performed to find a page to evict.
- **Non-Resident PTE:** A custom format for the Page Table Entry (PTE) where the present bit is 0 (`P=0`), a custom software bit marks the swapped page nature, and the physical address space holds the logical swap sector.
- **I/O Contract:** Assembly of an `astra_swap_io_request` struct ready to be dispatched to a block driver module. (The actual disk write step is currently deliberately omitted to maintain focus on MMU management).

### Design Choices and Intentional Incompleteness

An earlier experimental version of Astra attempted to implement the full disk swap by adding block device I/O code directly into `sistema.cpp`. 
This approach was eventually discarded because it broke the modularity of the kernel, mixing system responsibilities with I/O module duties. 

Therefore, **Astra is intentionally incomplete regarding physical disk writes**. The project aims to demonstrate a solid understanding of memory management (MMU, Page Faults, TLB, PTE) without compromising the architectural cleanliness of the OS. The `astra_swap_io_request` serves as a bridge, ready to be integrated when a proper inter-process communication (IPC) or queuing mechanism with the I/O driver is implemented in future iterations.

### Navigating the Code
If you want to read the source code and distinguish Astra's modifications from the vanilla `nucleo-8.3` kernel, you can simply search for the `[ASTRA]` tag. Every custom implementation is explicitly enclosed in comment blocks like this:

```cpp
// !!! [ASTRA] 
// Astra-specific code and comments here...
// [ASTRA_END]
```

### How to Run Tests
Astra includes a memory pressure test to trigger Out-Of-Memory (OOM) conditions.

1. Compile the kernel:
   ```bash
   cd nucleo-8.3
   make
   ```
2. Run QEMU:
   ```bash
   AUTOCORR=1 ../libce-4.3/scripts/boot -n
   ```

*(The memory pressure test code is located at `nucleo-8.3/utente/examples/astra-pressione.cpp`)*

---

<a name="italiano"></a>
## 🇮🇹 Italiano

**Astra** è un'estensione del kernel didattico `nucleo-8.3` (sviluppato presso l'Università di Pisa) pensata per implementare un meccanismo reale di memoria virtuale: il **Demand Paging**. 

Il progetto affronta e risolve un problema concreto: evitare di allocare staticamente l'intera pila (stack) per i processi utente alla loro creazione, ritardando l'allocazione del frame fisico solo al momento dell'effettivo bisogno (ovvero al primo accesso alla pagina).

### Architettura e Funzionalità Implementate

Il sistema è strutturato in due componenti principali: una attiva (il demand paging vero e proprio) e una architetturale (la pipeline di swap out).

#### 1. Demand Paging Reale
Il processo utente parte con un indirizzo di base per la pila valido a livello virtuale, ma senza memoria fisica associata. Il primo `push` o accesso genera un *Page Fault* (eccezione 14).

Il kernel intercetta questa specifica eccezione, alloca il frame e lo mappa on-the-fly:

```cpp
// All'interno di gestore_page_fault(vaddr fault_addr)
vaddr page_base = fault_addr & ~(DIM_PAGINA - 1);
paddr new_frame = alloca_frame();

memset(voidptr_cast(new_frame), 0, DIM_PAGINA); // Sicurezza

// Mappatura on-demand nella Page Table
vaddr mapped_until = map(esecuzione->cr3, page_base, page_base + DIM_PAGINA, BIT_US | BIT_RW, get_frame);

marca_frame(new_frame, FRAME_PAGINA_UTENTE, esecuzione, page_base);
invalida_entrata_TLB(page_base);
```

#### 2. Pipeline Diagnostica di Swap (OOM Management)
Qualora la memoria fisica (RAM) si esaurisse, Astra non va in blocco in modo incontrollato, ma innesca una pipeline di *Swap-Out* progettata per non "sporcare" la separazione tra modulo sistema e driver I/O del disco:

- **Reverse Mapping e Metadati:** Ogni frame fisico sa a quale processo appartiene e in quale indirizzo virtuale si trova.
- **Scelta della Vittima:** Scansione circolare dei frame utente rimpiazzabili per trovare una pagina da espellere.
- **PTE Non Residente:** Formato custom per la Page Table Entry (PTE) dove il bit di presenza è a 0 (`P=0`), un bit software segna la natura della pagina swappata, e lo spazio dell'indirizzo fisico contiene il Logical Slot Sector del disco.
- **Contratto I/O:** Compilazione della richiesta `astra_swap_io_request` inviabile a un modulo driver a blocchi. (Attualmente lo step di invio reale su I/O non è implementato per mantenere il focus sulla gestione MMU).

### Scelte Progettuali e Incompletezza Intenzionale

Una precedente versione sperimentale di Astra tentava di implementare lo swap completo scrivendo il codice di I/O del disco direttamente all'interno di `sistema.cpp`. 
Questo approccio è stato successivamente abbandonato (tramite rollback) perché rompeva la modularità del kernel, mischiando le responsabilità del sistema con i compiti del modulo I/O.

Per questo motivo, **Astra è volutamente incompleto per quanto riguarda la scrittura fisica su disco**. L'obiettivo del progetto è dimostrare una solida comprensione della gestione della memoria (MMU, Page Fault, TLB, PTE) senza compromettere la pulizia architetturale del Sistema Operativo. Il contratto `astra_swap_io_request` funge da ponte, pronto per essere integrato quando verrà sviluppato un adeguato meccanismo di code o IPC (Inter-Process Communication) con il driver I/O in sviluppi futuri.

### Esplorare il Codice
Se desideri leggere il codice sorgente e distinguere agevolmente le modifiche di Astra dal kernel `nucleo-8.3` originale (vanilla), puoi semplicemente cercare il tag `[ASTRA]`. Ogni nuova implementazione è racchiusa esplicitamente in blocchi di commento come questo:

```cpp
// !!! [ASTRA] 
// Codice e commenti specifici di Astra qui...
// [ASTRA_END]
```

### Come Eseguire i Test

Astra include un test di stress per innescare l'esaurimento memoria (OOM).

1. Compilare il kernel:
   ```bash
   cd nucleo-8.3
   make
   ```
2. Lanciare QEMU:
   ```bash
   AUTOCORR=1 ../libce-4.3/scripts/boot -n
   ```

*(Il codice del memory pressure test è consultabile sotto `nucleo-8.3/utente/examples/astra-pressione.cpp`)*
