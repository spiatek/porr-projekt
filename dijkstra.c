#include <stdio.h>

#define MAX 7
#define INF 999

int auction_search(int *pr, int *P, int a[][MAX], int s, int t)
{
	int i, length = 1, j = t;
	int la, maxla, argmaxla;
	if(s == t) {
		printf("s = t, brak sciezki\n");
		return 0;
	}
	for(i = 0; i < MAX; i++) {
		P[i] = INF;
		pr[i] = 0;
	}
	if(t >= 0 && t < MAX) {
		P[t] = 0;
	}
	else {
		printf("Blad, t musi byc z zakresu 0..MAX-1\n");
		return 0;
	}
	if(s < 0 || s >= MAX) {
		printf("Blad, s musi byc z zakresu 0..MAX-1\n");
		return 0;
	}
	while(P[s] == INF) {
		printf("Sciezka: ");
		for(i = 0; i < MAX; i++) {
			printf("%d ", P[i]);
		}
		printf("\n");
		maxla = 0 - INF;
		argmaxla = -1;
		printf("j = %d\n", j);
		for(i = 0; i < MAX; i++) {
			la = pr[i] - a[i][j];
			if(la > maxla && P[i] == INF) {	//pomijane sa te, ktore juz sa w P
				maxla = la;
				argmaxla = i;
			}
		}
		if(pr[j] > maxla) {
			printf("Contracting path\n");
			pr[j] = maxla;
			if(j != t) {
				P[j] = INF;
				length = length - 1;
				for(i = 0; i < MAX; i++) {		//wraca do poprzedniego j
					if(P[i] == length - 1) {
						j = i;
						break;
					}
				}
			}
		}
		else {
			printf("Extending path\n");
			P[argmaxla] = length;
			j = argmaxla;
			length = length + 1;
			if(argmaxla == s)
				return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	int a[MAX][MAX]={{INF,2,4,7,INF,5,INF},{2,INF,INF,6,3,INF,8},{4,INF,INF,INF,INF,6,INF},{7,6,INF,INF,INF,1,6},{INF,3,INF,INF,INF,INF,7},{5,INF,6,1,INF,INF,6},{INF,8,INF,6,7,6,INF}};
	int prices[MAX];
	int P[MAX];
	int i, source, tail;
	source = atoi(argv[1]);
	tail = atoi(argv[2]);
	auction_search(prices, P, a, source, tail);
	for(i = 0; i < MAX; i++) {
		printf("%d ", P[i]);
	}
	printf("\n");
	return 0;
}
