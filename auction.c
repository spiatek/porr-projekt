#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <smmintrin.h>
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

uint32_t get_from_m128i(__m128i var, int n)
{
	uint32_t *val = (uint32_t*) &var;	
	return val[n];
}

void print128_num(__m128i var)
{
	uint32_t *val = (uint32_t*) &var;
	printf("Numerical: %i %i %i %i\n", val[0], val[1], val[2], val[3]);
}

int sse_auction_search(int *pr, int *P, int *ai0, int *ai1, int *a0, int *a1, int nodes, int arcs, int s, int t)
{
	int i __attribute__ ((aligned (16))) = 0;	
	int j __attribute__ ((aligned (16))) = t;
	int k __attribute__ ((aligned (16))) = 0;
	int m __attribute__ ((aligned (16))) = 0;	
	int maxla __attribute__ ((aligned (32))) = 0;
	int argmaxla __attribute__ ((aligned (16))) = 0;
	int cost __attribute__ ((aligned (16))) = 0;
	int length __attribute__ ((aligned (16))) = 1;
	int path_cost __attribute__ ((aligned (16))) = 0;
	
	uint32_t tmp1, tmp2;
	int cost_tab[nodes+1];

	__m128i a0sse, a1sse, ai0sse, ai1sse, ai1sse1, I, J, K, M, then;
	__m128i ARCS, MNODES, INFINITE, NEGINF, prsse, Psse, MAXLA, ARGMAXLA, LA, mask1, mask2, mask3, COST;
			
	for(i = 0; i <= nodes; i++) {
		cost_tab[i] = 0;
	}

	if(check_s_t(s, t, P, nodes) != 0) {
		return 1;
	}

	while(P[s] == INF) {
		k = -1;	
		m = -1;

		//printf("j = %d\n", j);

		J = _mm_set1_epi32(j);			//aktualna wartosc j
		K = _mm_set1_epi32(-1);			//poczatkowy indeks w tablicy z kosztami krawedzi
		M = _mm_set1_epi32(-1);			//koncowy indeks w tablicy z kosztami krawedzi
		MNODES = _mm_set1_epi32(nodes-1);	//liczba wezlow pomniejszona o 1 (do sprawdzenia czy koniec tablicy)
		ARCS = _mm_set1_epi32(arcs);		//liczba krawedzi
	
		/* wyliczenie k, m */
		for(i = 0; i < nodes; i+=4) {
			ai0sse = _mm_load_si128((__m128i*) &ai0[i]);	//ladowanie ai0 (numerow wezlow)
			ai1sse = _mm_load_si128((__m128i*) &ai1[i]);	//ladowanie ai1 (indeksow w tablicy z krawedziami)
			ai1sse1 = _mm_set_epi32(ai1[i+4],ai1[i+3],ai1[i+2],ai1[i+1]);	//ladowanie indeksow z ai1 przesunietych o 1
			mask1 = _mm_cmpeq_epi32(J, ai0sse);				//sprawdzenie warunku j == ai0[i]
			K = _mm_or_si128(_mm_and_si128(mask1,ai1sse), _mm_andnot_si128(mask1,K));	//ustalenie K
			I = _mm_set_epi32(i+3, i+2, i+1, i);						//aktualne wartosci i
			mask2 = _mm_cmplt_epi32(I, MNODES);				//sprawdzenie warunku i == nodes-1
			mask3 = _mm_and_si128(mask1,mask2);				//sprawdzenie sumy warunkow 1 i 2
			then = _mm_or_si128(_mm_and_si128(mask2,ai1sse1), _mm_andnot_si128(mask2,ARCS));	//m = ai1[i+1] lub arcs
			M = _mm_or_si128(_mm_and_si128(mask3,then), _mm_andnot_si128(mask3,M));		//ustalenie M
		}

		/* zapisanie k, m */
		for(i = 0; i < 4; i++) {
			tmp1 = get_from_m128i(K,i);
			tmp2 = get_from_m128i(M,i);
			if(tmp1 != -1) {
				k = tmp1;
			}
			if(tmp2 != -1) {
				m = tmp2;
			}
		}
		//printf("K,M: %d %d\n", k, m);

		/* wybor optymalnej krawedzi */
		if(k != -1) {		
			INFINITE = _mm_set1_epi32(INF);		//wartosc "nieskonczona"
			NEGINF = _mm_set1_epi32(0-INF);		//wartosc -INF
			COST = _mm_set1_epi32(cost);		//koszt wybranej krawedzi
			MAXLA = _mm_set1_epi32(0-INF);		//maksymalna wartosc la = pr[a0[i]] - a1[i]
			ARGMAXLA = _mm_set1_epi32(-1);		//indeks dla którego la jest najwieksza
			for(i = k; i < m; i+=4) {
				a1sse = _mm_set_epi32(a1[i],a1[i+1],a1[i+2],a1[i+3]);				//ladowanie a1
				a0sse = _mm_set_epi32(a0[i],a0[i+1],a0[i+2],a0[i+3]);				//ladowanie a0
				prsse = _mm_set_epi32(pr[a0[i]],pr[a0[i+1]],pr[a0[i+2]],pr[a0[i+3]]);		//ladowanie pr
				Psse = _mm_set_epi32(P[a0[i]],P[a0[i+1]],P[a0[i+2]],P[a0[i+3]]);		//ladowanie P
				mask1 = _mm_cmpgt_epi32(_mm_set1_epi32(m),_mm_set_epi32(i,i+1,i+2,i+3));	//czy ostatni obieg
				prsse = _mm_or_si128(_mm_and_si128(mask1,prsse), _mm_andnot_si128(mask1,NEGINF));	//obciecie cudzych lukow
				LA = _mm_sub_epi32(prsse, a1sse);		//la = pr[a0[i]] - a1[i]
				then = _mm_max_epi32(LA,MAXLA);			//maksymalna wartość la, maxla
				mask1 = _mm_cmpeq_epi32(Psse,INFINITE);		//czy P[i] == INF
				mask2 = _mm_and_si128(mask1,_mm_cmpgt_epi32(LA,MAXLA));		//czy P[i] == INF i LA > MAXLA
				MAXLA = _mm_or_si128(_mm_and_si128(mask1,then), _mm_andnot_si128(mask1,MAXLA));		//aktualizacja maxla
				ARGMAXLA = _mm_or_si128(_mm_and_si128(mask2,a0sse), _mm_andnot_si128(mask2,ARGMAXLA));	//aktualizacja argmaxla
				COST = _mm_or_si128(_mm_and_si128(mask2,a1sse), _mm_andnot_si128(mask2,COST));		//aktualizacja cost
			}
		}
	
		/* zapisanie maxla, argmaxla, cost */
		maxla = 0 - INF;
		for(i = 0; i < 4; i++) {
			tmp1 = get_from_m128i(MAXLA,i);
			if(tmp1 > maxla) {
				argmaxla = get_from_m128i(ARGMAXLA,i);
				maxla = tmp1;
				cost = get_from_m128i(COST,i);
			}
		}
		//printf("COST: %d, PATH_COST: %d\n", cost, path_cost);
		//printf("pr[j] = %d, maxla = %d, argmaxla = %d\n", pr[j], maxla, argmaxla);

		/* skrocenie sciezki */
		if(pr[j] > maxla || maxla == -INF) {
			
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
				printf("dlugosc sciezki: %d\n", path_cost);
				return 0;
			}
		}
	}
	return 0;


}

int omp_auction_search(int *pr, int *P, int (*a)[2], int (*ai)[2], int *fpr, omp_lock_t* pmux, int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, m, l;
	int la, maxla, argmaxla, cost, path_cost;
	int cost_tab[nodes+1];

	int index = -1;
	int lockResult = 0;
	List list = createList();
	
	for(i = 0; i <= nodes; i++) {
		P[i] = INF;
		pr[i] = 0;
		cost_tab[i] = 0;
	}
	
	if(check_s_t(s, t, P, nodes) != 0) {
		return 1;
	}

	/* dodanie head do listy */
	list.add(&list, t, path_cost);

	while(P[s] == INF) {
		/* sprawdzanie wynikow z innych watkow */
		if(list.isEmpty(&list) == 0)
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

		/* wyszukanie krawedzi wychodzacych z j w tabeli a */
		for(i = 0; i < nodes; i++) {
			if(ai[i][0] == j) {
				k = a[i][1];		//k - indeks startowy krawedzi wychodzacych z j
				//printf("i = %d ", i);
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

		if(k == 1 || pr[j] > maxla || maxla == -INF) {
			pr[j] = maxla;
			if(j != t) {
				P[j] = INF;
				length = length - 1;
				path_cost = path_cost - cost_tab[length];
				cost_tab[length] = 0;	
				/* usuwanie wezla i zdejmowanie lock */
				list.remove_(&list, j);
				//omp_unset_lock(&(pmux[j]));
				k = j;
				for(i = 0; i < nodes; i++) {
					if(P[i] == length - 1) {
						j = i;
						break;
					}
				}
			}
		}
		else {
			P[argmaxla] = length;
			j = argmaxla;
			path_cost = path_cost + cost;
			cost_tab[length] = cost;
			length = length + 1;	
			/* dodawanie wezla do list i ustawianie lock, 0 gdy juz ktos blokuje */
			//lockResult = omp_test_lock(&(pmux[j]));
			if (lockResult == 0)
			{	
				//odlokowanie blokowanych wezlow	
				if (list.isEmpty(&list) == 0)
				{
					list.setCurrToHead(&list);
					do
					{
						index = list.getCurr(&list);
						//omp_unset_lock(&(pmux[index]));
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

		if(argmaxla == s)
		{
			/* odlobkowanie wszystkich blokad */
			if (list.isEmpty(&list) == 0)
			{
				list.setCurrToHead(&list);
				do
				{
					index = list.getCurr(&list);
					//omp_unset_lock(&(pmux[index]));
				} while (list.getNext(&list) != -1);
			}

			/* wyczyszczenie listy */
			list.clear(&list);

			return path_cost;
		}
	}
	return 0;

}

int auction_search(int *pr, int *P, int (*a)[2], int (*ai)[2], int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, m, l;
	int la, maxla, argmaxla, cost, path_cost;
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
		printf("j = %d\n", j);

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

		//printf("K,M: %d %d\n", k, m);

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
		//printf("COST: %d, PATH_COST: %d\n", cost, path_cost);
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
				printf("dlugosc sciezki: %d\n", path_cost);
				return 0;
			}
		}
	}
	return 0;
}


int auction_omp_search(int *pr, int *P, int (*a)[2], int(*ai)[2], int nodes, int arcs, int s, int t)
{
	int result;
	int item, i, endloop = -1;
	int fpr[nodes];
	//omp_lock_t pmux[nodes];
	//omp_lock_t qmux;
	Queue queue = createQueue();
	int finalResult = -1;

	//omp_init_lock(&qmux);

	printf("P1\n");

	//#pragma omp parallel for
	for(i = 0; i < nodes; ++i)
	{
		fpr[i] = INF;
	}

	//inicjalizacja kolejki
	for(i = 0; i < nodes; ++i)
	{
		//omp_init_lock(&pmux[i]);
		queue.push(&queue, i);
	}

	printf("P2 %d\n", nodes);

	//#pragma omp parallel firstprivate(result, item, pr, P, a, nodes, arcs, s) shared(pmux, fpr, qmux)
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
			//omp_set_lock(&qmux);
			item = queue.pop(&queue);
			//omp_unset_lock(&qmux);

			printf("Auction_search\n");
			//result = omp_auction_search(pr, P, a, ai, fpr, pmux, nodes, arcs, s, item);
			printf("Result: %d\n", result);			

			if (result == -2)
			{
				//wstaw na koniec kolejki gdy jest blokada
				//omp_set_lock(&qmux);
				queue.push(&queue, item);
				//omp_unset_lock(&qmux);
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
					//omp_set_lock(&qmux);
					queue.size = 0;
					//omp_unset_lock(&qmux);
					finalResult = result;
					break;
				}
			}
		}
		while(queue.size != 0);
 	}

	for(i = 0; i < nodes; ++i)
	{
		//omp_destroy_lock(&pmux[i]);
	}

	//omp_destroy_lock(&qmux);
	queue.queue_clear(&queue);

	return finalResult;
}


int main(int argc, char* argv[])
{
	double time;
	int *prices, *P;
	int i, task, source, tail, nodes, arcs;
	int (*network)[2], (*network_i)[2];
	int *a0, *a1, *ai0, *ai1, *Psse, *prsse;
	clock_t start, end;
	char *filename;

	printf("Hello\n");

	task = atoi(argv[1]);
	arcs = atoi(argv[2]);
	nodes = atoi(argv[3]);
	filename = argv[4];

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
		//omp_set_num_threads(1);
	//auction_search(prices, P, network, nodes, arcs, source, tail);
		auction_omp_search(prices, P, network, network_i, nodes, arcs, source, tail);
	}
	else if(task == SEQ) {
		auction_search(prices, P, network, network_i, nodes, arcs, source, tail);
	}
	else if(task == SSE) {
		a0 = _mm_malloc(arcs*sizeof(int), 16);
		a1 = _mm_malloc(arcs*sizeof(int), 16);
		ai0 = _mm_malloc(nodes*sizeof(int), 16);
		ai1 = _mm_malloc(nodes*sizeof(int), 16);
		Psse = _mm_malloc((nodes+1)*sizeof(int), 16);
		prsse = _mm_malloc((nodes+1)*sizeof(int), 16);
		for(i = 0; i < arcs; i++) {
			a0[i] = (int) network[i][0];
			a1[i] = (int) network[i][1];
		}
		for(i = 0; i < nodes; i++) {
			ai0[i] = (int) network_i[i][0];
			ai1[i] = (int) network_i[i][1];
		}
		for(i = 0; i <= nodes; i++) {
			Psse[i] = (int) INF;
			prsse[i] = 0;
		}
		sse_auction_search(prsse, Psse, ai0, ai1, a0, a1, nodes, arcs, source, tail);
		_mm_free(a0);
		_mm_free(a1);
		_mm_free(ai0);
		_mm_free(ai1);
		_mm_free(Psse);
		_mm_free(prsse);
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

