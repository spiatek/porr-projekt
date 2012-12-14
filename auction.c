#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define NODES		400
#define ARCS		1500
#define MAX_LINE_LENGTH 100
#define MIN		0
#define ASN		1
#define INF		9999999

int (*nettab)[2];
int source, tail;		//wezel poczatkowy i koncowy

int get_matrix()
{
	int nodes, arcs, type;		//liczba wezlow, krawedzi, typ sieci
	char *res, str[100];
	char c;
	int i, j, tmp, tmp1, tmp2, tmp4;
	int tmp3;
	FILE* f;

	if((f = fopen("outp", "r")) == NULL) {				//outp - plik wyjsciowy
		printf("Nie mozna otworzyc pliku.\n");
		exit(1);
	}
	while(1) {
		if((fscanf(f, "%c", &c)) == 0) {
			printf("Nie mozna odczytac znaku.\n");
			exit(1);
		}
		if(c != 'c') {
			break;
		}
		if((res = fgets(str, MAX_LINE_LENGTH, f)) == NULL) {
			printf("Nie mozna odczytac linii.\n");
			exit(1);
		}
	}

	if(c != 'p') {
		printf("Nie rozpoznano znaku poczatku linii.\n");
		exit(1);
	}
	if((fscanf(f, "%s %d %d\n", str, &nodes, &arcs)) == 0) {
		printf("Nie mozna odczytac parametrow sieci\n");
		exit(1);
	}
	if(strcmp(str, "min") == 0) {
		type = MIN;
	}
	else if(strcmp(str, "asn") == 0) {
		type = ASN;
	}
	else {							//zalozenie: siec musi byc typu assignment lub min cost flow
		printf("Niewlasciwy typ sieci.\n");
		exit(1);
	}

	nettab = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	
	//ta petla jest do poprawienia, zeby obslugiwala wiecej source i tail
	for(i = 0; ; i++) {
		if((fscanf(f, "%c %d", &c, &tmp)) == 0) {
			printf("Nie mozna odczytac znaku 2.\n");
			exit(1);
		}
		if(c != 'n') {
			break;
		}
		if(i == 0) {
			source = tmp;
		}
		else if(i == 1) {
		//	tail = tmp;
			tail = 399;
		}
		if((res = fgets(str, MAX_LINE_LENGTH, f)) == NULL) {
			printf("Nie mozna odczytac linii 2.\n");
			exit(1);
		}
	}

	nettab[0][0] = 0;
	nettab[0][1] = tmp;
	
	if(type == MIN) {
		if((fscanf(f, " %d %d %d %d", &tmp, &tmp1, &tmp2, &tmp3)) == 0) {
			printf("Nie mozna odczytac znaku 3.\n");
			exit(1);
		}
	}
	else if(type == ASN) {
		if((fscanf(f, " %d %d", &tmp, &tmp1)) == 0) {
			printf("Nie mozna odczytac znaku 3.\n");
			exit(1);
		}
	}
	
	nettab[1][0] = tmp;
	nettab[1][1] = tmp1;

	for(i = 1, j = 1; i + j < arcs + nodes; i++) {
		if((res = fgets(str, MAX_LINE_LENGTH, f)) == NULL) {
			printf("Nie mozna odczytac linii 3.\n");
			exit(1);
		}
		if((fscanf(f, "%c ", &c)) == 0) {
			printf("Nie mozna odczytac znaku 4.\n");
			exit(1);
		}
		else if(c != 'a') {
			break;
		}
		//printf("%c ", c);
		if(type == MIN) {
			if((fscanf(f, "%d %d %d %d %d", &tmp, &tmp1, &tmp2, &tmp3, &tmp4)) == 0) {
				printf("Nie mozna odczytac znaku 5.\n");
				exit(1);
			}
			//printf("%d %d %d %d %d\n", tmp, tmp1, tmp2, tmp3, tmp4);
		}
		else if(type == ASN) {
			if((fscanf(f, "%d %d %d", &tmp, &tmp1, &tmp4)) == 0) {
				printf("Nie mozna odczytac znaku 5.\n");
				exit(1);
			}
		}
		if(tmp != j) {
			nettab[i+j][0] = 0;
			nettab[i+j][1] = tmp;
			j++;
		}
		nettab[i+j][0] = tmp1;
		nettab[i+j][1] = tmp4;
	}

	fclose(f);

	tmp = i + j;
	for(i = 0; i < tmp; i++) {
		//printf("%d %d\n", nettab[i][0], nettab[i][1]);
	}

	return 0;
}

int auction_search(int *pr, int *P)
{
	int s = source, t = tail;
	int i, length = 1, j = t, k, l;
	int la, maxla, argmaxla;
	if(s == t) {
		printf("s = t, brak sciezki\n");
		return 0;
	}
	for(i = 0; i < NODES; i++) {
		P[i] = INF;		//sciezka: dla poszczegolnych wezlow wartosc to pozycja w sciezce
		pr[i] = 0;		//ceny
	}
	if(t >= 0 && t <= NODES) {
		P[t] = 0;
	}
	else {
		printf("Blad, t musi byc z zakresu 0..MAX-1\n");
		printf("%d\n", t);
		return 0;
	}
	if(s < 0 || s > NODES) {
		printf("Blad, s musi byc z zakresu 0..MAX-1\n");
		printf("%d\n", s);
		return 0;
	}
	while(P[s] == INF) {
		//printf("Sciezka: ");
		//for(i = 0; i < NODES; i++) {
		//	printf("%d ", P[i]);
		//}
		//printf("\n");
		maxla = 0 - INF;
		argmaxla = -1;
		//printf("j = %d\n", j);
		k = -1;		//indeks poprzedzajacy luki wychodzace z j w tabeli nettab
		for(i = 0; i < NODES+ARCS; i++) {
			if(nettab[i][0] == 0 && nettab[i][1] == j) {
				k = i;
				//printf("i = %d ", i);
				break;
			}
		}
		if(k != -1) {
			for(i = k + 1; i < NODES+ARCS; i++) {
				if(nettab[i][0] == 0) {			//przejrzano juz wszystkie luki
					break;
				}
				l = nettab[i][0];
				la = pr[l] - nettab[i][1];		//cena - koszt dotarcia do wezla
				//printf("la: %d %d %d\n", i, nettab[i][1], la);
				//l = nettab[i][0];
				if(la > maxla && P[l] == INF) {		//pomijane sa te, ktore juz sa w P
					//printf("nowy max: %d %d\n", l, nettab[i][1]);
					maxla = la;
					argmaxla = l;			//numer wezla
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
				for(i = 0; i < NODES; i++) {		//wraca do poprzedniego j
					if(P[i] == length - 1) {
						j = i;
						break;
					}
				}
			}
		}
		else {
			//printf("Extending path\n");
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
	get_matrix();	
	int prices[NODES];
	int P[NODES];
	int i, t;
	double time;
	//source = atoi(argv[1]);
	//tail = atoi(argv[2]);
	auction_search(prices, P);
	time = clock()/CLOCKS_PER_SEC;
	t = clock();
	printf("Czas wykonania programu: %d\n", t);
	//for(i = 0; i < NODES; i++) {
	//	printf("%d ", P[i]);
	//}
	//printf("\n");
	return 0;
}
