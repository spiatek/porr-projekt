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
#include "list.c"

#define INF 999999
 
 struct t_result {
	 int path_cost;
	 int t;
 };
 
typedef struct t_result Result;
 

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

int single_auction_search(int *pr, int *P, int *cost_tab, int (*a)[2], int (*ai)[2], int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, m, l;
	int la, maxla, argmaxla, cost, path_cost = 0;

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
				//printf("result= %d\n", path_cost);
				return path_cost;

			}
		}
	}
	//printf("no result\n");
	return -1;
}


int main_single_auction_search(int *pr, int *P, int (*a)[2], int (*ai)[2], int *fpr, int *dest_ptr, int *src_ptr, MPI_Request *request, int workers, int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, m, l;
	int la, maxla, argmaxla, cost, path_cost = 0;
	int cost_tab[nodes+1];
	
	int tag = 200;
	int index = -1;
	int finalIndex = -1;
	int value = 0;
	int finalValue = 0;
	int dest, src;
	int flag = 0;
	int finish[workers];
	int node_count = 1;
	Result result;
	MPI_Status status[workers];
	
	
	List list = createList();
	
	
	for(i = 0; i < workers; i++) {
		finish[i] = 0;
	}
	
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
	
	list.add(&list, t, 0);			

	while(P[s] == INF) {
	
		/* czesc identyczna jak w sekwencyjnym algorytmie aukcyjnym */
		maxla = 0 - INF;
		argmaxla = -1;
		k = -1;
		m = -1;

						/* odbieranie danych od worker*/
						for(j = 0; j < workers; j++) 
						{
							MPI_Iprobe(j + 1, tag + j + 1, MPI_COMM_WORLD, &flag, &status[j]);
							if (flag != 0)
							{
								MPI_Irecv(&result, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
								printf("Communicate received %d for t=%d\n", result.path_cost, result.t);
								fpr[result.t] = result.path_cost;
								finish[j] = 1;
								//fpr[src-1] = src_ptr[0];

	//							for(k = 1; k <= nodes; k++) {
	//								if(fpr[k] != INF) {
	//									printf("FFFF %i %d ", k, fpr[k]);
	//								}
	//							}
							}
						}
		
						/* wysylanie danych do worker */
						for(j = 0; j < workers; j++) {
							
							if (finish[j] == 1)
							{
								++node_count;
								dest_ptr[0] = node_count;
								printf("Auction_search for tail=%d\n", dest_ptr[0]);
								MPI_Isend(dest_ptr, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
								//MPI_Isend(prices, nodes+1, MPI_INT, dest, tag + dest, MPI_COMM_WORLD, &request[j]);
								printf("Communicate sent\n");
								finish[j] = 0;
							}
						}



				

		/* wyszukanie krawedzi wychodzacych z j w tablicy a */
		for(i = 0; i < nodes; i++) {
			if(ai[i][0] == j) {
				k = ai[i][1];		//k - indeks startowy krawedzi wychodzacych z j
				if(i < nodes - 1) {
					m = ai[i+1][1];
				}
				else {
					m = arcs;
				}
			}
		}

		/* wybor optymalnej krawedzi */
		if(k != -1) {
			for(i = k; i < m; i++) 
			{
				/* l - aktualnie przetwarzany potecjalny nastepny wierzcholek */
				l = a[i][0];

				/* la - cena tego wierzcholka minus koszt dotarcia do niego */
				la = pr[l] - a[i][1];

				/* wierzcholki w sciezce nie moga sie powtarzac */
				if(la > maxla && P[l] == INF) {
					maxla = la;		//nowy maksymalny la
					argmaxla = l;		//numer wezla
					cost = a[i][1];		//koszt potenjalnie dodawanej krawedzi
				}
			}
		}


		/* sprawdzanie wynikow z innych procesow */
		if(list.isEmpty(&list) == 0)
		{
			list.setCurrToHead(&list);

			do
			{
				index = list.getCurr(&list);
				value = list.getCurrValue(&list);
				if (fpr[index] != INF)
				{	
					finalIndex = index;
					finalValue = value;
					/* fpr[index] - koszt sciezki do i-tego wezla */
					/* list.getCurrValue(&list) - policzona w innym watku sciezka z i do s */	
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
						} while (list.getNext(&list) != -1);
					}
					
					list.clear(&list);

								/* zakonczenie pracy workerow */
								for(j = 0; j < workers; j++) 
								{
									while (flag != 0)
									{
										MPI_Iprobe(j + 1, tag + j + 1, MPI_COMM_WORLD, &flag, &status[j]);
										
										if (flag != 0)
										{
											MPI_Irecv(&result, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
											printf("Communicate received %d for t=%d\n", result.path_cost, result.t);
											fpr[result.t] = result.path_cost;
											finish[j] = 1;
											dest_ptr[0] = -1;
											printf("Auction_search send end message %d\n", dest_ptr[0]);
											MPI_Isend(dest_ptr, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
										}
									}						
								}
					
					
					return fpr[finalIndex] + finalValue;
				}
			} while (list.getNext(&list) != -1);

								
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
				list.setCurrToHead(&list);
				list.remove_(&list, j);

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
				list.add(&list, j, path_cost);	
			}

			/* sciezka doszla do wierzcholka startowego => koniec */
			if(argmaxla == s)
			{
				/* wyczyszczenie listy */
				list.clear(&list);
			
				/* zakonczenie pracy workerow*/
						for(j = 0; j < workers; j++) 
						{
							while (flag != 0)
							{
								MPI_Iprobe(j + 1, tag + j + 1, MPI_COMM_WORLD, &flag, &status[j]);
								
								if (flag != 0)
								{
									MPI_Irecv(&result, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
									printf("Communicate received %d for t=%d\n", result.path_cost, result.t);
									fpr[result.t] = result.path_cost;
									finish[j] = 1;
									dest_ptr[0] = -1;
									printf("Auction_search send end message %d\n", dest_ptr[0]);
									MPI_Isend(dest_ptr, 1, MPI_INT, j + 1, tag + j + 1, MPI_COMM_WORLD, &request[j]);
								}
							}						
						}
			
				return path_cost;
			}
		}	
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
	int i, j, k, source=400, tail=1, nodes=400, arcs=1500;
	int dest, src;
	int *dest_ptr, *src_ptr;
	int (*network)[2], (*network_i)[2];
	clock_t start, end;

	Result result;
	int request_arg;
	int item, endloop = -1;
	int finalResult = -1;

	int length = 1, m, l, t, index;
	int la, maxla, argmaxla, cost, path_cost = 0;

	network = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	network_i = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));
	read_network(/*filename*/"outp", &source, &tail, &nodes, &arcs, network, network_i);

	  MPI_Init(&argc, &argv);
	  MPI_Comm_rank(MPI_COMM_WORLD, &rank);		//identyfikacja procesu
	  MPI_Comm_size(MPI_COMM_WORLD, &size);		//podanie liczby procesów w komunikatorze

  /* Manager */

	  printf("RANK %d SIZE %d\n", rank, size);

  if (rank == 0) {

    /* Allocate and initialize the data */

    workers = size - 1;
	dest_ptr = (int*)malloc(sizeof(int) * workers);
	src_ptr = (int*)malloc(sizeof(int));
	prices = (int*)malloc((nodes+1)*sizeof(int));
    request = malloc(sizeof(MPI_Request) * workers);	//tyle ile workersów
	fpr = (int*)malloc((nodes+1)*sizeof(int));

	start = clock();

	for(i = 0; i <= nodes; ++i)
	{
		fpr[i] = INF;
	}
	
	prices = (int*)malloc((nodes+1)*sizeof(int));
	P = (int*)malloc((nodes+1)*sizeof(int));
	cost_tab = (int*)malloc((nodes+1)*sizeof(int));

	for(i = 0; i <= nodes; i++) {
		P[i] = INF;
		prices[i] = 0;
		cost_tab[i] = 0;
	}

	path_cost =  main_single_auction_search(prices, P, network, network_i, fpr, dest_ptr, src_ptr, request, workers, nodes, arcs, source, tail);

	
	
	end = clock();

    time = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Czas wykonania programu: %5.1f [ms]\n", time*1000);
	printf("Dlugosc sciezki: %d\n", path_cost);

	
	
    free(prices);
    free(fpr);
    free(dest_ptr);
    free(src_ptr);
    free(request);
  }

  /* Workers */

  else {
		//printf("::WORKER %d STARTED::", rank);

		prices = (int*)malloc((nodes+1)*sizeof(int));
		P = (int*)malloc((nodes+1)*sizeof(int));
		cost_tab = (int*)malloc((nodes+1)*sizeof(int));

		for(i = 0; i <= nodes; i++) {
			P[i] = INF;
			prices[i] = 0;
			cost_tab[i] = 0;
		}

		t = 0;
		do
		{
			MPI_Recv(&request_arg, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (request_arg == -1)
				break;
			path_cost = single_auction_search(prices, P, cost_tab, network, network_i, nodes, arcs, source, request_arg);
			//path_cost = 1;
			result.t = request_arg;
			result.path_cost = path_cost;
			MPI_Send(&result, 1, MPI_INT, 0, tag + rank, MPI_COMM_WORLD);
		} while(result.t < nodes-size+1);

    	free(prices);
    	free(P);
    	free(cost_tab);
  }

  MPI_Finalize();
  return 0;
}
