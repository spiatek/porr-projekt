#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "read_network.c"

#include "queue.c"
#include "list.c"

#include <omp.h>

#define INF 99999

#define OMP 0
#define SEQ 1
#define SSE 2
	
int check_s_t(int source, int tail, int *P, int nodes)
{	
	/* sprawdzenie czy source jest jednoczesnie tail */
	if(source == tail) {
		printf("s = t, brak sciezki\n");
		return 1;
	}
	
	/* sprawdzenie czy t jest poprawne */
	if(tail > 0 && tail <= nodes) {
		P[tail] = 0.0;
	}
	else {
		printf("Blad, t musi byc z zakresu 1..nodes\n");
		printf("t nodes %d %d\n", tail, nodes);
		return 1;
	}

	/* sprawdzenie czy s jest poprawne */
	if(source <= 0 || source > nodes) {
		printf("Blad, s musi byc z zakresu 1..nodes\n");
		printf("%d\n", source);
		return 1;
	}

	return 0;
}

int omp_single_auction_search(int *pr, int *P, int (*a)[2], int (*ai)[2], int *fpr, /*omp_lock_t *pmux, */omp_lock_t *fmux, omp_lock_t *qmux, Queue *queue, int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, m, l;
	int la, maxla, argmaxla, cost, path_cost = 0;
	int cost_tab[nodes+1];
	
	int index = -1;
	int finalIndex = -1;
	int lockResult = 0;
	int value = 0;
	int finalValue = 0;
	List list = createList();
			
	/* wypelnienie wartosciami poczatkowymi tablicy ze sciezka, tablicy z cenami wezlow */
	/* oraz tablicy z kosztami dojscia do wezlow w sciezce */
	/* sciezka - dla poszczegolnych wezlow wartosc to pozycja w sciezce */
	for(i = 0; i <= nodes; i++) {
		P[i] = INF;
		pr[i] = 0;
		cost_tab[i] = 0;
	}

	/* sprawdzenie poprawnosci source i tail */
	if(check_s_t(s, t, P, nodes) != 0) {
		return 1;
	}
	
	/*lockResult = omp_test_lock(&(pmux[j]));
	printf("1XXXlockResult=%d for j=%d\n", lockResult, j);
	if (lockResult == 0)
	{			
		return -2;
	}
	else if (lockResult == 1)
	{
		printf("dodaje do listy wezel j=%d path_cost=%d\n", j, cost);
		//omp_set_lock(&(pmux[t]));*/
	
	list.add(&list, t, 0);			
	
	//}
	//printf("start=%d, tail=%d\n", s, t);

	while(P[s] == INF) {
	
		/* czesc identyczna jak w sekwencyjnym algorytmie aukcyjnym */
		maxla = 0 - INF;
		argmaxla = -1;
		k = -1;
		m = -1;

		//printf("j = %d\n", j);

		/* wyszukanie krawedzi wychodzacych z j w tablicy a */
		for(i = 0; i < nodes; i++) {
			if(ai[i][0] == j) {
				k = ai[i][1];		//k - indeks startowy krawedzi wychodzacych z j
				//printf("i = %d ", i);
				if(i < nodes - 1) {
					m = ai[i+1][1];
				}
				else {
					m = arcs;
				}
			}
		}

		//printf("k = %d, m = %d\n", k, m);

		/* wybor optymalnej krawedzi */
		if(k != -1) {
			for(i = k; i < m; i++) {

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

		/* sprawdzanie wynikow z innych watkow */
		if(list.isEmpty(&list) == 0)
		{
			list.setCurrToHead(&list);
			omp_set_lock(fmux);
			do
			{
				index = list.getCurr(&list);
				value = list.getCurrValue(&list);
				if (fpr[index] != INF)
				{	
					finalIndex = index;
					finalValue = value;
					//printf("Wartosc juz obliczono w innym watku dla index=%d wartosc=%d\n", finalIndex, fpr[index]);
					/* fpr[index] - koszt sciezki do i-tego wezla */
					/* list.getCurrValue(&list) - policzona w innym watku sciezka z i do s */	
					omp_set_lock(qmux);
					if (list.isEmpty(&list) == 0)
					{
						list.setCurrToHead(&list);
						do
						{
							index = list.getCurr(&list);
							value = path_cost - list.getCurrValue(&list);
							if (fpr[index] == INF)
							{
								fpr[index] = value;
							}
							//printf("before remove fpr[%d]=%d\n", index, fpr[index]);
							queue->queue_remove(queue, index);
							//omp_unset_lock(&(pmux[index]));
							//printf("lock unset for %d\n", index);
						} while (list.getNext(&list) != -1);
					}
					
					list.clear(&list);

					omp_unset_lock(qmux);
					omp_unset_lock(fmux);
					
					return fpr[finalIndex] + finalValue;
				}
			} while (list.getNext(&list) != -1);
			omp_unset_lock(fmux);
								
		}

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
	//			printf("usuwam z listy wezel j=%d\n", j);
				//int lsize = list.size(&list);
				//do {		
				//omp_set_lock(&lmux);
				list.setCurrToHead(&list);
				list.remove_(&list, j);
				//omp_unset_lock(&lmux);
				//} while(lsize == list.size(&list) + 1);

///				omp_unset_lock(&(pmux[j]));
	//			printf("lock unset for %d\n", j);				

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
			if (j != s)
			{
			/*	lockResult = omp_test_lock(&(pmux[j]));
				printf("lockResult=%d for j=%d\n", lockResult, j);
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
							printf("XXXlock unset for %d\n", index);
						} while (list.getNext(&list) != -1);
					}
					//czyszczenie listy
					printf("CCCCCCCCCCCCCCCZZZZZZZZZZZZZZZYYYYYYYYYYYYYSSSSSSSSSSSSSZZZZZZZZZ %d\n", s);
					list.clear(&list);
	
					return -2;
				}
				else if (lockResult == 1)
				{
					printf("dodaje do listy wezel j=%d path_cost=%d\n", j, path_cost);
					//omp_set_lock(&(pmux[j]));*/
			
				list.add(&list, j, path_cost);
	
			/*	}*/
			}

			/* sciezka doszla do wierzcholka startowego => koniec */
			if(argmaxla == s)
			{
				omp_set_lock(fmux);
				omp_set_lock(qmux);
				
				if (list.isEmpty(&list) == 0)
				{
					list.setCurrToHead(&list);
					do
					{
						index = list.getCurr(&list);
						value = path_cost - list.getCurrValue(&list);
						fpr[index] = value;
						//printf("before remove fpr[%d]=%d\n", index, fpr[index]);
						queue->queue_remove(queue, index);
	//					printf("SIZE::::::: %d\n", queue->size);
						//omp_unset_lock(&(pmux[index]));
						//printf("lock unset for %d\n", index);
					} while (list.getNext(&list) != -1);
///					omp_unset_lock(&(pmux[t]));
	//				printf("lock unset for %d\n", t);
				}
				/* wyczyszczenie listy */
				list.clear(&list);
	//				printf("before RETURN path_cost=%d\n", path_cost);
				
				omp_unset_lock(qmux);
				omp_unset_lock(fmux);
								
				//printf("before RETURN path_cost=%d\n", path_cost - 1);
				return path_cost;
			}
		}	
	}
	return 0;
	
}

int auction_search(int *pr, int *P, int (*a)[2], int (*ai)[2], int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, m, l;
	int la, maxla, argmaxla, cost, path_cost = 0;
	int cost_tab[nodes+1];
	
	/* wypelnienie wartosciami poczatkowymi tablicy ze sciezka, tablicy z cenami wezlow */
	/* oraz tablicy z kosztami dojscia do wezlow w sciezce */
	/* sciezka - dla poszczegolnych wezlow wartosc to pozycja w sciezce */
	for(i = 0; i <= nodes; i++) {
		P[i] = INF;
		pr[i] = 0;
		cost_tab[i] = 0;
	}

	/* sprawdzenie poprawnosci source i tail */
	if(check_s_t(s, t, P, nodes) != 0) {
		return 1;
	}

	while(P[s] == INF) {
		maxla = 0 - INF;
		argmaxla = -1;
		k = -1;
		m = -1;

		//printf("j = %d %d %d %d %d\n", j, ai[1][0], ai[1][1], a[1][0], a[1][1]);
	//	printf("j = %d\n", j);

		/* wyszukanie krawedzi wychodzacych z j w tabeli a */
		for(i = 0; i < nodes; i++) {
			if(ai[i][0] == j) {
				k = ai[i][1];		//k - indeks startowy krawedzi wychodzacych z j
				//printf("i = %d ", i);
				if(i < nodes - 1) {
					m = ai[i+1][1];
				}
				else {
					m = arcs;
				}
			}
		}

//		printf("k=%d m=%d\n", k, m);

		/* wybor optymalnej krawedzi */
		if(k != -1) {
			for(i = k; i < m; i++) {
//				printf("for loop i=%d\n",i);

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
	//	printf("pr[j] = %d, maxla = %d, argmaxla = %d\n", pr[j], maxla, argmaxla);

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

			/* sciezka doszla do wierzcholka startowego => koniec */
			if(argmaxla == s)
			{
				printf("result= %d\n", path_cost);
				return 0;
				
			}
		}
	}
	printf("no result\n");
	return 0;
}


int auction_omp_search(int *pr, int *P, int (*a)[2], int(*ai)[2], int nodes, int arcs, int s, int t)
{
	int result;
	int item, i, endloop = -1;
	int fpr[nodes];
	//omp_lock_t *pmux;
	omp_lock_t qmux;
	omp_lock_t fmux;
	Queue queue = createQueue();
	int finalResult = -1;

	omp_init_lock(&qmux);
	omp_init_lock(&fmux);

	//pmux = (omp_lock_t*) malloc(nodes*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem

	//printf("P1\n");

	//inicjalizacja fpr
	for(i = 0; i < nodes; ++i)
	{
		fpr[i] = INF;
	}

	//inicjalizacja semaforow
	for(i = 0; i < nodes; ++i)
	{
		//omp_init_lock(&(pmux[i]));
//		queue.push(&queue, i);
	}
	
	//inicjalizacja kolejki
	
	for(i = nodes - 2; i > 0; --i)
	{
		queue.push(&queue, i);
	}

//	queue.push(&queue, 243);
//	queue.push(&queue, 1);
	
	//printf("P2 %d\n", nodes);

	#pragma omp parallel firstprivate(result, item, pr, P, a, ai, nodes, arcs, s) shared(/*pmux,*/queue, fpr, qmux, fmux)
	{
		do
		{
			//printf("P3 %d %d %d %d thread=%d\n", nodes, arcs, s, t, omp_get_thread_num());
			omp_set_lock(&qmux);
			item = queue.pop(&queue);
			printf("<<<<<<<<<<<>>>>>>>>>>>>>ITEM=%d %d\n", item, omp_get_thread_num());
			if (item >= 0)
			{
				omp_unset_lock(&qmux);
				//printf("Auction_search for tail=%d\n", item);
				//queue.display(&queue);
				result = omp_single_auction_search(pr, P, a, ai, fpr, /*pmux,*/&fmux, &qmux, &queue, nodes, arcs, s, item);
				printf("############################################################################ Result: %d\n", result);
				if (result == -2)
				{
					//wstaw na koniec kolejki gdy jest blokada
					//printf("NIE MOZE WYLICZYC\n");
					omp_set_lock(&qmux);
					queue.push(&queue, item);
					omp_unset_lock(&qmux);
				}
				else if (result == -1)
				{
					//usun z kolejki, usuwamy z kolejki robiac pop wiec nic tutaj nie trzeba robic
				}
				else if (result == 0)
				{
					//s==t czyli usuwam z kolejki, usuwamy z kolejki robiac pop wiec nic tutaj nie trzeba robic
				}
				else
				{
					//wstaw wynik do tablicy fpr
					fpr[item] = result;
					for(i = 0; i < nodes; i++) {
						if(fpr[i] != INF) {
							printf("FFFF %i %d ", i, fpr[i]);
						}
					}
					/*if (item == t)
					{
						omp_set_lock(&qmux);
						queue.size = 0;
						omp_unset_lock(&qmux);
						finalResult = result;
						//printf("before break Thread = %d\n", omp_get_thread_num());
						break;
					}*/
				}
			
			}
			else
			{
				omp_unset_lock(&qmux);
			}
			printf("&&& end while for Thread = %d %d\n", omp_get_thread_num(), fpr[t]);
			#pragma omp barrier
			printf("@@@!!! after barrier for Thread = %d\n", omp_get_thread_num());
		}
		while(queue.size != 0);
	}
	//printf("Usuwanie zasobow for Thread = %d\n", omp_get_thread_num());
	
	/*for(i = 0; i < nodes; ++i)
	{
		omp_destroy_lock(&(pmux[i]));
	}*/

	omp_destroy_lock(&qmux);
	omp_destroy_lock(&fmux);
	queue.queue_clear(&queue);
	
	printf("Final result: %d\n", fpr[t]);	
		
	for(i = 0; i < nodes; i++) {
		if(fpr[i] != INF) {
			printf("%i %d; ", i, fpr[i]);
		}
	}
	return finalResult;
}


int main(int argc, char* argv[])
{
	double time;
	int *prices, *P;
	int i, task, source, tail, nodes, arcs, threads;
	int (*network)[2], (*network_i)[2];
	float *a0, *a1, *ai0, *ai1, *Psse, *prsse;
	clock_t start, end;
	char *filename;

	printf("Hello\n");

	task = atoi(argv[1]);
	arcs = atoi(argv[2]);
	nodes = atoi(argv[3]);
	filename = argv[4];
	threads = atoi(argv[5]);

	printf("P-1\n");

	network = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	network_i = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));
	read_network(filename, &source, &tail, &nodes, &arcs, network, network_i);

	printf("%d %d %d %d\n", network_i[0][0], network_i[0][1], network[0][0], network[0][1]);

	prices = (int*)malloc((nodes+1)*sizeof(int));
	P = (int*)malloc((nodes+1)*sizeof(int));
	printf("P0\n");

	start = clock();

	if(task == OMP) {
		omp_set_num_threads(threads);
		auction_omp_search(prices, P, network, network_i, nodes, arcs, source, tail);
	}
	else if(task == SEQ) {
		//auction_search(prices, P, network, network_i, nodes, arcs, source, tail);
		//for(i = 1; i < 398; i++)
		auction_search(prices, P, network, network_i, nodes, arcs, source, 243);
		printf("after seq\n");
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
	free(network);
	free(network_i);
	free(prices);
	free(P);

	return 0;
}

