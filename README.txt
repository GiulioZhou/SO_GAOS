----------------------------------------------------
I) Composizione del Gruppo
----------------------------------------------------
* Olga Becci
* Giacomo Rizzi
* Giulio Zhou
* Sara Brolli


Altri membri che non lavorano più la progetto
* Andrian Leah
* Matteo Barato
* Mattia Biondi
* Alessio Tosto


----------------------------------------------------
II) Compilazione ed esecuzione del codice sorgente
----------------------------------------------------
Il codice sorgente può essere compilato tramite il comando make.
L'unica dipendenza è la libreria uArm.
Una volta eseguita la compilazione, si otterranno i file
- kernel.elf.core.uarm
- kernel.elf.stab.uarm


III) Istruzioni al codice sorgente
----------------------------------------------------
Si è cercato di uniformare lo stile di codice per mantenere la leggibilità,
le scelte principali sono: utilizzo di 4 spazi per l'indentazione, parentesi di
apertura delle funzioni nella stessa riga della dichiarazione, commenti di
descrizione uniformi prima di ogni funzione/macro, utilizzo di parole chiave
come TODO.

Guideline per il modulo clist:
http://www.cs.unibo.it/~renzo/so/jaeos16/phase0/clist_template.h

Panoramica dei moduli pcb e asl:
http://www.cs.unibo.it/~renzo/so/jaeos16/phase1.pdf

IV) Note e considerazioni

Macro clist_pop:
Questa macro è stata ridefinita per comodità, e
semplicemente salva la testa in elem e la rimuove dalla
lista (utilizzata nelle allocazioni)

Macro clist_foreach:
- Inizializzazione:
a) tmp è la coda della lista (NULL se la lista è vuota)
- Condizioni pre-ciclo:
a) tmp non deve essere NULL (fallisce se la lista è vuota,
   o se viene forzata l'uscita nella modifica b)
b) scan, che ad ogni ciclo diventa tmp->next, non deve
   essere NULL (condizione sempre vera, ma necessaria
  per effettuare l'assegnamento)
- Modifiche post-ciclo:
a) tmp diventa tmp->next
b) se la lista ha un solo elemento (tmp == tmp->next),
   tmp diventa NULL per uscire dal ciclo (condizione a)

Modulo util
È stato creato il modulo util per raccogliere funzioni di generica utilità,
ad esempio un implementazione della memset e delle funzioni per assistere il
debug.
