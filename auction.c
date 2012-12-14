#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "read_network.c"

#define INF		9999999

int auction_search(int *pr, int *P, int (*a)[2], int nodes, int arcs, int s, int t)
{
	int i, length = 1, j = t, k, l;
	int la, maxla, argmaxla, cost, path_cost;
	if(s == t) {
		printf("s = t, brak sciezki\n");
		return 0;
	}
	for(i = 0; i < nodes; i++) {
		P[i] = INF;		//sciezka: dla poszczegolnych wezlow wartosc to pozycja w sciezce
		pr[i] = 0;		//ceny
	}
	if(t >= 0 && t <= nodes) {
		P[t] = 0;
	}
	else {
		printf("Blad, t musi byc z zakresu 0..MAX-1\n");
		printf("%d\n", t);
		return 0;
	}
	if(s < 0 || s > nodes) {
		printf("Blad, s musi byc z zakresu 0..MAX-1\n");
		printf("%d\n", s);
		return 0;
	}

	while(P[s] == INF) {
		//printf("Sciezka: ");
		//for(i = 0; i < nodes; i++) {
		//	printf("%d ", P[i]);
		//}
		//printf("\n");
		maxla = 0 - INF;
		argmaxla = -1;
		//printf("j = %d\n", j);
		k = -1;		//indeks poprzedzajacy luki wychodzace z j w tabeli nettab
		for(i = 0; i < nodes+arcs; i++) {
			if(a[i][0] == 0 && a[i][1] == j) {
				k = i;
				//printf("i = %d ", i);
				break;
			}
		}
		if(k != -1) {
			for(i = k + 1; i < nodes+arcs; i++) {
				if(a[i][0] == 0) {			//przejrzano juz wszystkie luki
					break;
				}
				l = a[i][0];
				la = pr[l] - a[i][1];		//cena - koszt dotarcia do wezla
				//printf("la: %d %d %d\n", i, nettab[i][1], la);
				//l = nettab[i][0];
				if(la > maxla && P[l] == INF) {		//pomijane sa te, ktore juz sa w P
					//printf("nowy max: %d %d\n", l, a[i][1]);
					maxla = la;
					argmaxla = l;			//numer wezla
					cost = a[i][1];
				}
			}
		}
		//printf("pr[j] = %d, maxla = %d, argmaxla = %d\n", pr[j], maxla, argmaxla);
		if(pr[j] > maxla || maxla == -INF) {
			//printf("Contracting path\n");
			pr[j] = maxla;
			if(j != t) {
				P[j] = INF;
				length = length - 1;
				k = j;					//zapamietuje j jako k
				for(i = 0; i < nodes; i++) {		//wraca do poprzedniego j
					if(P[i] == length - 1) {
						j = i;
						break;
					}
				}
				for(i = 0; i < nodes+arcs; i++) {
					if(a[i][0] == 0 && a[i][1] == j) {						
						break;
					}
				}
				while(1) {
					i++;
					if(a[i][0] == k) {
						path_cost = path_cost - a[i][1];
						break;
					}
				}
			}
		}
		else {
			//printf("Extending path\n");
			P[argmaxla] = length;
			j = argmaxla;
			path_cost = path_cost + cost;
			length = length + 1;
			if(argmaxla == s)
				return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	double time;
	int *prices, *P;
	int source, tail, nodes, arcs;
	int (*network)[2];
	arcs = atoi(argv[1]);
	nodes = atoi(argv[2]);
	network = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	read_network("outp", &source, &tail, &nodes, &arcs, network);
	prices = (int*)malloc(nodes*sizeof(int));
	P = (int*)malloc(nodes*sizeof(int));
	auction_search(prices, P, network, nodes, arcs, source, tail);
	time = clock()/CLOCKS_PER_SEC;
	printf("Czas wykonania programu: %f\n", time);
	//for(i = 0; i < nodes; i++) {
	//	printf("%d ", P[i]);
	//}
	//printf("\n");
	return 0;
}
