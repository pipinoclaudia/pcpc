# N-Body Simulation
Project of Parallel and Concurrent Programming on the Cloud course. Professor: Vittorio Scarano.
I test verranno eseguiti su macchine in cloud Amazon di tipo m4.large con le seguenti specifiche

Modello | vCPU | Mem (GiB) | Storage SSD (GB) | 
--------- | ---------- | ---------- | ---------- | 
m4.large | 2 | 8 | Solo EBS | 

## Index
1. Problema
2. Soluzione
3. Risultati

## 1 - Problema

1. Parallelizzare l'algoritmo N-Body. Si tratta di una simulazione di un sistema dinamico di particelle, di solito sotto l'influenza delle forze fisiche, come la gravità.

2. Partizionare il carico di lavoro originale dell'algoritmo sequenziale a più core per ridurre il lavoro di ciascun processore.

## 2 - Soluzione

Per parallelizzare l'algoritmo di simulazione del problema n-body, sono state utilizzare le librerie MPI standard.

Il carico di lavoro è stato bilanciato per tutti i processi per evitare che un processo facesse più lavoro degli altri 
Sono state modificate parti dell'algoritmo originale, nello specifico:
- E' stato creato il tipo di dato MPI_Body che rende possibile l'invio e la ricezione della struttura Body (formata da 6 variabili "float" di cui tre formano le coordinate della aprticella nello spazio e cioè x y e z, mentre le altre 3 specificano la velocità dello spostamento in una determinata coordinata quale vx vy e vz). [Code]( https://drive.google.com/open?id=1jZHNXodcIoTrdjJgXuIOjCqWdDUT7hTE)
- Viene inizializzato il numero di particelle, non c'è inserimento da linea di comando (https://drive.google.com/open?id=1hBimBemzQurq8YEb2Eo96bfetYIkMRHm)
- E' stato utilizzato il tipo MPI_Type_Contiguous per la comunicazione tra i processori andando a costruire datatype contigui, in questo caso MPIBody(evita problemi per quanto riguarda la gestione della memoria che in MPI non è contigua). (https://drive.google.com/open?id=134JcAcShaS3U-9Sb5ZhU3DIn5BQotPYC) 
-I bodies vengono generati in maniera random (https://drive.google.com/open?id=1_kculCComPWpdrR2YuBB0bU_dolup080)
- Ogni processo andrà a calcolare la velocità di spostamento di una parte delle particelle, attraverso la funzione bodyForce. (https://drive.google.com/open?id=1eWuJOIcEmCY0462Fkl7iNsfuLgkn4XPZ)
- Ogni processo comunicherà con gli altri in questo modo: 
a) inizializzo un flag che sarà uguale al numero di particelle % numero di processi e se il flag è uguale a 0 non ho problemi perchè posso iniviare a tutti i processori la stessa porzione di bodies(https://drive.google.com/open?id=1cZVUcpMdvO_2Ax4iNmz5eYgkkAIGA9ue)
c)altrimenti, poichè vi è il resto,  devo effettuare le chiamate di MPI_Send e MPI_Recv perchè i processi devono comunicare porzioni più grandi di bodies. Quindi succede che ll master inizializza k e start. Se k è minore di Aflag invia una porzione di bodies grande che risulta dal modulo firstPos al processore i, incrementa k e aggiorna start; altrimenti se k è maggiore o uguale a aFlag vuol dire che il resto è stato eliminato quindi inviamo una parte di bodies grande partition e aggiorniamo start. In fine il master chiama il suo bodyForce (https://drive.google.com/open?id=1fuauo0omsgCggQXOWjvaO_j5TZbUuB0P)
-Per quanto riguarda gli SLAVE, QUESTI dovranno ricevere ALCUNI un bodies firstPos, altri dovranno ricevere un bodies partition(dipende dalla spiegazione del punto precedente).
Se process, ovvero il "rank" del processo è minore del flag allora vuol dire che devo ricevere un bodies firstPos e sulla porzione ricevuta chiamerò bodyForce. Altrimenti devo ricevere un bodies partition e chiamare bodyForce(https://drive.google.com/open?id=1fuauo0omsgCggQXOWjvaO_j5TZbUuB0P)
-MPI_Barrier per aspettare che tutti i processori finiscano la computazione, quinid per la sincronizzazione
-Il processo master stamperà il numero di Bodies computato e il tempo di esecuzione (https://drive.google.com/open?id=1yN5Kf6gLwrF_qi44z3qgqIAfpN_sbIyE)

-Note: per fare il run in weak scalability(per aumentare la taglia dell'input) bisogna modificare il parametro "int particles=30000;" a linea 32 del codice.

## 3 - Risultati

Per verificare l'efficienza dell'algoritmo proposto, sono stati fatti i test di scalabilità Strong e Weak. Per effettuare una camputazione di più di 3 minuti è stato inizializzato il numero totale di Bodies a 30 000.

#### Strong Scalability

Per quanto riguarda il test di Strong Scalability sono stati effettuati aumentando volta per volta il numero di processi lasciando la taglia dell'input inalterato(https://drive.google.com/open?id=1D-vC8JUlFlBF6DNerSOWknGkFWtN1qB0).

Il grafico dimostra che aumentando il numero di processi e rimanendo inalterato il numero delle particelle in input, diminuisce il tempo di esecuzione. La linea tratteggiata mostra l'andamento ideale avuto dividendo il tempo dell'esecuzione sequenziale per il numero di processi di ogni iterazione.
Di seguito viene mostrata la tabella, la prima riga mostra l'esecuzione dell'algoritmo con un singolo processore.

Instances   | Processors | Time in sec| 
---------   | ---------- | ---------- | 
1   |	1	 |   471,19  |   
1   |	2	 |   420,77  |    
2   |	4	 |   209.37  |    
3   |	6	 |   139.21  |    
4   |	8	 |   105.31  |    
5	|	10	 |   83.33   |    
6   |	12	 |   69.43   |    
7   |	14	 |   59.54   |    
8   |	16	 |   52.06   |    




#### Weak Scalability

I test di scalabilità debole invece sono stati effettuati aumentando la taglia dell'input in maniera uniforme rispetto al numero di processi.

Il grafico seguente, mostra l'andamento della scalabilità debole.
Si è diviso il numero di particelle(30 000) per il numero massimo di processi(16), in modo tale da incrementare in maniera costante il numero di particelle per il numero di processi, dando quindi ad ogni processo lo stesso numero di particelle da processare.
(https://drive.google.com/open?id=1EHayzNEmspampCGu_j9jTQDswiOWArSo)

Di seguito viene mostrata la tabella, la prima riga mostra l'esecuzione dell'algoritmo con un singolo processore.

#Instances  | #Processors | #Bodies | Real time(sec) | 
----------- | ----------- | ------- | --------- | 
1 |	1	  |  1875   |  1,84	    |	
1 |	4	  |  3750   |  6,57 	|	
2 |	8	  |  7500   |  13,06	|	
3 |	12	  |  11250  |  19.55	|	
4 |	16	  |  15000  |  26,04    |	
5 |	20	  |  18750  |  32,57    |	
6 |	24	  |  22500  |  39,05	|	
7 |	28	  |  26250  |  45,68	|	
8 |	32	  |  30000  |  52,06	|	



### Considerazioni

Come possiamo notare dalle tabelle e dai grafici, vi è un miglioramento in termini di tempo dell'esecuzione parallela rispetto all'esecuzione sequenziale. 