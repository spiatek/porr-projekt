/*
 * auction_mpi.c
 *
 *  Created on: 31-01-2013
 *      Author: spiatek
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "read_network.c"
#include "queue.c"

#define INF 999999

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

int main(int argc, char *argv[])
{
  int total, workers;
  int rank, size;
  int len = 100, tag = 200;
  MPI_Request *request;

	double time;
	int *prices, *P;
	int i, task, source, tail, nodes, arcs, threads;
	int (*network)[2], (*network_i)[2];
	clock_t start, end;
	char *filename;

	task = atoi(argv[1]);
	nodes = atoi(argv[2]);
	arcs = atoi(argv[3]);
	filename = argv[4];
	threads = atoi(argv[5]);

	  MPI_Init(&argc, &argv);
	  MPI_Comm_rank(MPI_COMM_WORLD, &rank);		//identyfikacja procesu
	  MPI_Comm_size(MPI_COMM_WORLD, &size);		//podanie liczby procesów w komunikatorze

  /* Manager */

  if (rank == 0) {

    /* Allocate and initialize the data */

    workers = size - 1;
	network = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	network_i = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));
	read_network(filename, &source, &tail, &nodes, &arcs, network, network_i);

	printf("%d %d %d %d\n", network_i[0][0], network_i[0][1], network[0][0], network[0][1]);

	start = clock();

	int result;
	int item, i, endloop = -1;
	int fpr[nodes];

	Queue queue = createQueue();
	int finalResult = -1;

	for(i = 0; i < nodes; ++i)
	{
		fpr[i] = INF;
	}

	for(i = nodes - 2; i > 0; --i)
	{
		queue.push(&queue, i);		//do kolejki wrzucam kolejne numery węzłów
	}

	do
	{
		item = queue.pop(&queue);
		if (item >= 0)
		{
			printf("Auction_search for tail=%d\n", item);

			int s_tag = item % workers;

			MPI_Send(&item, 1, MPI_INT, s_tag, tag + s_tag, MPI_COMM_WORLD);
			MPI_Recv(&result, 1, MPI_INT, s_tag, tag + s_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			if (result == -2)			//jeśli result nie policzony, wrzucam dalej do kolejki
			{
				queue.push(&queue, item);
			}
			else if (result == -1) {}
			else if (result == 0) {}
			else
			{
				fpr[item] = result;
				for(i = 0; i < nodes; i++) {
					if(fpr[i] != INF) {
						printf("FFFF %i %d ", i, fpr[i]);
					}
				}
				if (item == tail)
				{
					queue.size = 0;
					finalResult = result;
					break;
				}
			}
		}
		//printf("&&& end while for Thread = %d %d\n", omp_get_thread_num(), fpr[t]);
	}
	while(queue.size != 0);

		end = clock();

    	time = ((double) (end - start)) / CLOCKS_PER_SEC;

    	printf("Czas wykonania programu: %5.1f [ms]\n", time*1000);

    	free(network);
    	free(network_i);
    	free(prices);
    	free(P);

    free(request);
  }

  /* Workers */

  else {
		int i, length = 1, k, m, l;
		int la, maxla, argmaxla, cost, path_cost = 0;
		int *cost_tab;

		prices = (int*)malloc((nodes+1)*sizeof(int));
		P = (int*)malloc((nodes+1)*sizeof(int));
		cost_tab = (int*)malloc((nodes+1)*sizeof(int));

		/* wypelnienie wartosciami poczatkowymi tablicy ze sciezka, tablicy z cenami wezlow */
		/* oraz tablicy z kosztami dojscia do wezlow w sciezce */
		/* sciezka - dla poszczegolnych wezlow wartosc to pozycja w sciezce */
		for(i = 0; i <= nodes; i++) {
			P[i] = INF;
			prices[i] = 0;
			cost_tab[i] = 0;
		}

		int t;
		int j = t;
		MPI_Recv(&t, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		/* sprawdzenie poprawnosci source i tail */
		if(check_s_t(source, t, P, nodes) != 0) {
			return 1;
		}

		while(P[source] == INF) {
			maxla = 0 - INF;
			argmaxla = -1;
			k = -1;
			m = -1;

			//printf("j = %d %d %d %d %d\n", j, ai[1][0], ai[1][1], a[1][0], a[1][1]);
		//	printf("j = %d\n", j);

			/* wyszukanie krawedzi wychodzacych z j w tabeli a */
			for(i = 0; i < nodes; i++) {
				if(network_i[i][0] == j) {
					k = network_i[i][1];		//k - indeks startowy krawedzi wychodzacych z j
					//printf("i = %d ", i);
					if(i < nodes - 1) {
						m = network_i[i+1][1];
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
					l = network[i][0];

					/* la - cena tego wierzcholka minus koszt dotarcia do niego */
					la = prices[l] - network[i][1];
					//printf("la: %d %d %d\n", i, a[i][1], la);

					/* wierzcholki w sciezce nie moga sie powtarzac */
					if(la > maxla && P[l] == INF) {
						//printf("nowy max: %d %d\n", l, a[i][1]);
						maxla = la;		//nowy maksymalny la
						argmaxla = l;		//numer wezla
						cost = network[i][1];		//koszt potenjalnie dodawanej krawedzi
					}
				}
			}
		//	printf("pr[j] = %d, maxla = %d, argmaxla = %d\n", pr[j], maxla, argmaxla);

			/* skrocenie sciezki */
			if(k == 1 || prices[j] > maxla || maxla == -INF) {

				/* uaktualnienie ceny */
				prices[j] = maxla;

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
				if(argmaxla == source)
				{
					//printf("result= %d\n", path_cost);
					MPI_Send(&path_cost, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD);
				}
			}
		}
		printf("no result\n");
		path_cost = INF;
		MPI_Send(&path_cost, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}
