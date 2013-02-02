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
#include <math.h>

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
	int *prices, *P, *cost_tab, *fpr;
	int i, j, k, source=399, tail=1, nodes=400, arcs=1500;
	int dest, src;
	int *dest_ptr, *src_ptr;
	int (*network)[2], (*network_i)[2];
	clock_t start, end;
	//char *filename;
	int result;
	int item, endloop = -1;
	int finalResult = -1;

	int length = 1, m, l, t, index;
	int la, maxla, argmaxla, cost, path_cost = 0;

/*	nodes = atoi(argv[2]);
	arcs = atoi(argv[3]);
	filename = argv[4];
*/
	  MPI_Init(&argc, &argv);
	  MPI_Comm_rank(MPI_COMM_WORLD, &rank);		//identyfikacja procesu
	  MPI_Comm_size(MPI_COMM_WORLD, &size);		//podanie liczby procesów w komunikatorze

  /* Manager */

	  printf("RANK %d SIZE %d\n", rank, size);

  if (rank == 0) {

    /* Allocate and initialize the data */

    workers = size - 1;
	network = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	network_i = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));
	dest_ptr = (int*)malloc(sizeof(int));
	src_ptr = (int*)malloc(sizeof(int));
	prices = (int*)malloc((nodes+1)*sizeof(int));
    request = malloc(sizeof(MPI_Request) * workers);	//tyle ile workersów
	fpr = (int*)malloc((nodes+1)*sizeof(int));

	read_network(/*filename*/"outp", &source, &tail, &nodes, &arcs, network, network_i);

	printf("%d %d %d %d\n", network_i[0][0], network_i[0][1], network[0][0], network[0][1]);
	printf("size of communicator: %d\n", size);

	start = clock();

		//Queue queue = createQueue();

	for(i = 0; i <= nodes; ++i)
	{
		fpr[i] = INF;
	}
/*
	for(i = nodes - 2; i > 0; --i)
	{
		queue.push(&queue, i);		//do kolejki wrzucam kolejne numery węzłów
	}
*/

	for(i = 0; i < nodes; i += workers)
	{
		for(j = 0; j < workers; j++) {
			dest = j + i + 1;
			dest_ptr[0] = dest;
			printf("Auction_search for tail=%d\n", dest);
			MPI_Isend(dest_ptr, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
			//MPI_Isend(prices, nodes+1, MPI_INT, dest, tag + dest, MPI_COMM_WORLD, &request[j]);
			printf("Communicate sent\n");
		}

		MPI_Waitall(workers, request, MPI_STATUSES_IGNORE);

		for(j = 0; j < workers; j++) {
			src = j + i + 1;
			MPI_Irecv(fpr + src, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
			printf("Communicate received %d\n", fpr[src]);
			//fpr[src-1] = src_ptr[0];

			for(k = 1; k <= nodes; k++) {
				if(fpr[k] != INF) {
					printf("FFFF %i %d ", k, fpr[k]);
				}
			}
		}
		//printf("AFFafa\n");
		//MPI_Waitall(workers, request, MPI_STATUSES_IGNORE);
		//printf("ssafa\n");
	}

	end = clock();

    time = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Czas wykonania programu: %5.1f [ms]\n", time*1000);

    free(network);
    free(network_i);
    free(request);
  }

  /* Workers */

  else {
		//printf("::WORKER %d STARTED::", rank);

		prices = (int*)malloc((nodes+1)*sizeof(int));
		P = (int*)malloc((nodes+1)*sizeof(int));
		cost_tab = (int*)malloc((nodes+1)*sizeof(int));
		dest_ptr = (int*)malloc(sizeof(int));

		//printf("::MEMORY ALLOCATED FOR %d::", rank);
		/* wypelnienie wartosciami poczatkowymi tablicy ze sciezka, tablicy z cenami wezlow */
		/* oraz tablicy z kosztami dojscia do wezlow w sciezce */
		/* sciezka - dla poszczegolnych wezlow wartosc to pozycja w sciezce */
		for(i = 0; i <= nodes; i++) {
			P[i] = INF;
			prices[i] = 0;
			cost_tab[i] = 0;
		}

		do
		{
			MPI_Recv(&result, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			//printf("::GOT COMMUNICATE %d FROM 0::", rank);
			t = result;
			j = t;

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
						path_cost=1;
						MPI_Send(&path_cost, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD);
					}
				}
			}
			//printf("no result\n");
			//path_cost = INF;
			//path_cost=2;
			//MPI_Send(&path_cost, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD);
		}
		while(result < nodes-size+1);

    	free(prices);
    	free(P);
    	free(cost_tab);
    	free(dest_ptr);
  }

  MPI_Finalize();
  return 0;
}
