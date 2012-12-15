#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "read_network.c"

#include "queue.c"
#include "list.c"

#include <omp.h>

#define INF 9999999

#define OMP 0
#define SEQ 1
#define SSE 2
	
int auction_search(int type, int *pr, int *P, int (*a)[2], int *fpr, omp_lock_t* pmux, int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, l;
	int la, maxla, argmaxla, cost, path_cost;
	int cost_tab[nodes+1];

	/* tylko Open MP */
	int index = -1;
	int lockResult = 0;
	List list = createList();

	/* sprawdzenie czy source jest jednoczesnie tail */
	if(s == t) {
		printf("s = t, brak sciezki\n");
		return 0;
	}

	/* wypelnienie wartosciami poczatkowymi tablicy ze sciezka, tablicy z cenami wezlow */
	/* oraz tablicy z kosztami dojscia do wezlow w sciezce */
	/* sciezka - dla poszczegolnych wezlow wartosc to pozycja w sciezce */
	for(i = 0; i <= nodes; i++) {
		P[i] = INF;
		pr[i] = 0;
		cost_tab[i] = 0;
	}

	/* sprawdzenie czy t jest poprawne */
	printf("%d!!!!!!!!!!!!!!!!!!!1\n", t);
	if(t > 0 && t <= nodes) {
		P[t] = 0;
	}
	else {
		printf("Blad, t musi byc z zakresu 1..nodes\n");
		printf("t nodes %d %d\n", t, nodes);
		return 0;
	}

	/* sprawdzenie czy s jest poprawne */
	if(s <= 0 || s > nodes) {
		printf("Blad, s musi byc z zakresu 1..nodes\n");
		printf("%d\n", s);
		return 0;
	}

	/* dodanie head do listy */
	if(type == OMP) {
		list.add(&list, t, path_cost);
	}

	while(P[s] == INF) {
		/* sprawdzanie wynikow z innych watkow */
		if(type == OMP && list.isEmpty(&list) == 0)
		{
			list.setCurrToHead(&list);
			do
			{
				index = list.getCurr(&list);
				if (fpr[index] != INF)
				{
					/* fpr[index] - koszt sciezki do i-tego wezla */
					/* list.getCurrValue(&list) - policzona w innym watku sciezka z i do s */
					return fpr[index] + list.getCurrValue(&list);
				}
			} while (list.getNext(&list) != -1);
		}

		maxla = 0 - INF;
		argmaxla = -1;
		k = -1;

		printf("j = %d\n", j);

		/* wyszukanie krawedzi wychodzacych z j w tabeli a */
		for(i = 0; i < nodes+arcs; i++) {
			if(a[i][0] == 0 && a[i][1] == j) {
				k = i;
				//printf("i = %d ", i);
				break;
			}
		}

		/* wybor optymalnej krawedzi */
		if(k != -1) {		//TODO: jesli k=-1, to trzeba koniecznie sie cofnac!!!
			for(i = k + 1; i < nodes+arcs; i++) {

				/* czy przejrzano juz wszystkie krawedzie */
				if(a[i][0] == 0) {
					break;
				}

				/* l - aktualnie przetwarzany potecjalny nastepny wierzcholek */
				l = a[i][0];

				/* la - cena tego wierzcholka minus koszt dotarcia do niego */
				la = pr[l] - a[i][1];
				//printf("la: %d %d %d\n", i, a[i][1], la);

				/* wierzcholki w sciezce nie moga sie powtarzac */
				if(la > maxla && P[l] == INF) {
					//printf("nowy max: %d %d\n", l, a[i][1]);
					maxla = la;		//nowy maksymalny la
					argmaxla = l;		//numer wezla
					cost = a[i][1];		//koszt potenjalnie dodawanej krawedzi
				}
			}
		}
		//printf("pr[j] = %d, maxla = %d, argmaxla = %d\n", pr[j], maxla, argmaxla);

		/* skrocenie sciezki */
		if(k == 1 || pr[j] > maxla || maxla == -INF) {
			
			/* uaktualnienie ceny */
			pr[j] = maxla;

			/* sciezka jednoelementowa nie jest skracana */
			if(j != t) {

				/* uaktualnienie sciezki */
				P[j] = INF;
				length = length - 1;
				path_cost = path_cost - cost_tab[length];
				cost_tab[length] = 0;
				
				/* usuwanie wezla i zdejmowanie lock */
				if(type == OMP) {
					list.remove_(&list, j);
					omp_unset_lock(&(pmux[j]));
				}
				
				/* powrot do poprzedniego wierzcholka w sciezce (j), k - odcinany */
				k = j;
				for(i = 0; i < nodes; i++) {
					if(P[i] == length - 1) {
						j = i;
						break;
					}
				}
			}
		}
		/* przedluzenie sciezki */
		else {
			P[argmaxla] = length;
			j = argmaxla;
			path_cost = path_cost + cost;
			cost_tab[length] = cost;
			length = length + 1;
	
			/* dodawanie wezla do list i ustawianie lock, 0 gdy juz ktos blokuje */
			if(type == OMP) {
				lockResult = omp_test_lock(&(pmux[j]));
				if (lockResult == 0)
				{
					//odlokowanie blokowanych wezlow
					if (list.isEmpty(&list) == 0)
					{
						list.setCurrToHead(&list);
						do
						{
							index = list.getCurr(&list);
							omp_unset_lock(&(pmux[index]));
						} while (list.getNext(&list) != -1);
					}

					//czyszczenie listy
					list.clear(&list);

					return -2;
				}
				else if (lockResult == 1)
				{
					list.add(&list, j, path_cost);
				}
			}

			/* sciezka doszla do wierzcholka startowego => koniec */
			if(argmaxla == s)
			{
				if(type == SEQ) {
					return 0;
				}
				else if(type == OMP) {
					
					/* odlobkowanie wszystkich blokad */
					if (list.isEmpty(&list) == 0)
					{
						list.setCurrToHead(&list);
						do
						{
							index = list.getCurr(&list);
							omp_unset_lock(&(pmux[index]));
						} while (list.getNext(&list) != -1);
					}

					/* wyczyszczenie listy */
					list.clear(&list);

					return path_cost;
				}
			}
		}
	}
	return 0;
}


int auction_omp_search(int *pr, int *P, int (*a)[2], int nodes, int arcs, int s, int t)
{
	int result;
	int item, i, endloop = -1;
	int fpr[nodes];
	omp_lock_t pmux[nodes];
	omp_lock_t qmux;
	Queue queue = createQueue();
	int finalResult = -1;

	omp_init_lock(&qmux);

	printf("P1\n");

	//#pragma omp parallel for
	for(i = 0; i < nodes; ++i)
	{
		fpr[i] = INF;
	}

	//inicjalizacja kolejki
	for(i = 0; i < nodes; ++i)
	{
		omp_init_lock(&pmux[i]);
		queue.push(&queue, i);
	}

	printf("P2 %d\n", nodes);

	#pragma omp parallel firstprivate(result, item, pr, P, a, nodes, arcs, s) shared(pmux, fpr, qmux)
	{
		do
		{
			printf("P3 %d %d %d %d\n", nodes, arcs, s, t);
			/*omp_set_lock(&pmux);
			printf("queue_size: %d\n", queue.size);
			if(queue.size == 0) {
				omp_unset_lock(&pmux);
				endloop = i;			
				break;
			}
			omp_unset_lock(&pmux);
			*/
			//dodac synchronizacje kolejki
			omp_set_lock(&qmux);
			item = queue.pop(&queue);
			omp_unset_lock(&qmux);

			printf("Auction_search\n");
			result = auction_search(OMP, pr, P, a, fpr, pmux, nodes, arcs, s, item);
			printf("Result: %d\n", result);			

			if (result == -2)
			{
				//wstaw na koniec kolejki gdy jest blokada
				omp_set_lock(&qmux);
				queue.push(&queue, item);
				omp_unset_lock(&qmux);
			}
			else if (result == -1)
			{
				//usun z kolejki, usuwamy z kolejki robiac pop wiec nic tutaj nie trzeba robic
				//nothing to write
			}
			else if (result == 0)
			{
				//s==t czyli usuwam z kolejki, usuwamy z kolejki robiac pop wiec nic tutaj nie trzeba robic
				//nothing to write
			}
			else
			{
				//wstaw wynik do tablicy fpr
				fpr[item] = result;
			
				if (item == t)
				{
					omp_set_lock(&qmux);
					queue.size = 0;
					omp_unset_lock(&qmux);
					finalResult = result;
					break;
				}
			}
		}
		while(queue.size != 0);
 	}

	for(i = 0; i < nodes; ++i)
	{
		omp_destroy_lock(&pmux[i]);
	}

	omp_destroy_lock(&qmux);
	queue.queue_clear(&queue);

	return finalResult;
}


int main(int argc, char* argv[])
{
	double time;
	int *prices, *P;
	int task, source, tail, nodes, arcs;
	int (*network)[2];
	clock_t start, end;
	char *filename;

	printf("Hello\n");

	task = atoi(argv[1]);
	arcs = atoi(argv[2]);
	nodes = atoi(argv[3]);
	filename = argv[4];

	printf("P-1\n");

	network = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	read_network(filename, &source, &tail, &nodes, &arcs, network);

	prices = (int*)malloc((nodes+1)*sizeof(int));
	P = (int*)malloc((nodes+1)*sizeof(int));
	printf("P0\n");

	start = clock();

	if(task == OMP) {
		omp_set_num_threads(1);
	//auction_search(prices, P, network, nodes, arcs, source, tail);
		auction_omp_search(prices, P, network, nodes, arcs, source, tail);
	}
	else if(task == SEQ) {
		auction_search(SEQ, prices, P, network, NULL, NULL, nodes, arcs, source, tail);
	}
	else {
		printf("Nieprawidlowy typ zadania\n");
		return 1;
	}
	
	end = clock();

	time = ((double) (end - start)) / CLOCKS_PER_SEC;

	printf("Czas wykonania programu: %5.1f [ms]\n", time*1000);
	//for(i = 0; i < nodes; i++) {
	//	printf("%d ", P[i]);
	//}
	//printf("\n");
	return 0;
}

