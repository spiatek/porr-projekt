#include <stdio.h>
#include <time.h>

#include "read_network.c"

#define INFINITE 9999999

int allselected(int *selected, int n)
{
	int i;
	for(i = 1; i < n + 1; i++) {
		if(selected[i] == 0) {
			return 0;
		}
	}
	return 1;
}

void shortpath(int (*cost)[2], int *preced, int *distance, int nodes, int arcs, int s, int t)
{
	int current = t, i, j, k, dc, smalldist, newdist;
	int selected[nodes + 1];
	
	for(i = 0; i <= nodes; i++) {
		distance[i] = INFINITE;
		selected[i] = 0;
	}
	distance[current] = 0;
	
	
	while(!allselected(selected, nodes))
	{
        	smalldist = INFINITE;
		dc = distance[current];
		k = 0;

		//printf("%d ", current);
		/* wyszukanie nieodwiedzonego wierzcholka (k) z najmniejszym distance (smalldist) */
		for(i = 1; i <= nodes; i++) {
			if(selected[i] == 0 && distance[i] < smalldist) {
				//printf("nowe distance[j] < smalldist i rowne\n", distance[i]);
				smalldist = distance[i];
				k = i;
				//printf("nowym wezlem bedzie %d\n", k);
			}
		}
		
		if(k != 0) {
			current = k;
			//printf("nowa iteracja z %d\n", current);
			selected[current] = 1;

			/* dotarcie do sasiadow current */
			for(i = 0; i < nodes + arcs; i++) {
				if(cost[i][0] == 0 && cost[i][1] == current) {
					break;
				}
			}
	
			while(i < nodes + arcs) {
				i++;
				j = cost[i][0];
				if(j == 0) {
					break;
				}
				//printf("rozp. wezel: %d\n", j);
				if(selected[j] == 0) {
					//printf("wezel %d jeszcze nie byl wybierany\n", j);
					//printf("jego koszt: %d\n", cost[i][1]);
					newdist = dc + cost[i][1];
					//printf("newdist: %d\n", newdist);
					if(newdist < distance[j]) {
						//printf("newdist < distance[j]\n");
						distance[j] = newdist;
						preced[j] = current;
					}
				}
			}
		}
	}
	//for(i = 0; i <= nodes; i++) {
       	//	printf("%d ", preced[i]);
	//}
	//printf("\n");
}

int main(int argc, char* argv[])
{
	double time;
	int *preced, *distance;
	int source, tail, nodes, arcs, i;
	int (*network)[2];
	clock_t start, end;
	char *filename;

	arcs = atoi(argv[1]);
	nodes = atoi(argv[2]);
	filename = argv[3];

	network = (int (*)[2])malloc((arcs+nodes)*2*sizeof(int));		//przydzial pamieci dla tablicy z grafem
	
	read_network(filename, &source, &tail, &nodes, &arcs, network);
	
	preced = (int*)malloc((nodes+1)*sizeof(int));
	for(i = 0; i < nodes; i++) {
		preced[i] = 0;
	}
	distance = (int*)malloc((nodes+1)*sizeof(int));
	
	start = clock();
	shortpath(network, preced, distance, nodes, arcs, source, tail);	
	end = clock();
	
	time = ((double) (end - start)) / CLOCKS_PER_SEC;

	printf("Czas wykonania programu: %5.1f [ms]\n", time*1000);
	//for(i = 0; i < nodes; i++) {
	//	printf("%d ", P[i]);
	//}
	//printf("\n");
	//for(i = 0; i <= nodes; i++) {
       	//	printf("%d\n", distance[i]);
	//}

	free(network);
	free(preced);
	free(distance);

	return 0;
}

