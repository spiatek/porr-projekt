__kernel void taskParallelAdd(__global int* FPR) 
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
		//	printf("FMUX: watek %d czeka na zwolnienie blokady\n", omp_get_thread_num());
			omp_set_lock(fmux);
		//	printf("FMUX: watek %d w sekcji krytycznej\n", omp_get_thread_num());
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
		//			printf("QMUX: watek %d czeka na zwolnienie blokady\n", omp_get_thread_num());
					omp_set_lock(qmux);
		//			printf("QMUX: watek %d w sekcji krytycznej\n", omp_get_thread_num());
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
		//			printf("QMUX: watek %d wychodzi z sekcji krytycznej\n", omp_get_thread_num());
					omp_unset_lock(qmux);
		//			printf("QMUX: watek %d wyszedl z sekcji krytycznej\n", omp_get_thread_num());
		//			printf("FMUX: watek %d wychodzi z sekcji krytycznej\n", omp_get_thread_num());
					omp_unset_lock(fmux);
		//			printf("FMUX: watek %d wyszedl z sekcji krytycznej\n", omp_get_thread_num());
							
					return fpr[finalIndex] + finalValue;
				}
			} while (list.getNext(&list) != -1);
		//	printf("FMUX: watek %d wychodzi z sekcji krytycznej\n", omp_get_thread_num());
			omp_unset_lock(fmux);
		//	printf("FMUX: watek %d wyszedl z sekcji krytycznej\n", omp_get_thread_num());
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
		//		printf("FMUX: watek %d czeka na zwolnienie blokady\n", omp_get_thread_num());
				omp_set_lock(fmux);
		//		printf("FMUX: watek %d w sekcji krytycznej\n", omp_get_thread_num());
		//		printf("QMUX: watek %d czeka na zwolnienie blokady\n", omp_get_thread_num());
				omp_set_lock(qmux);
		//		printf("QMUX: watek %d w sekcji krytycznej\n", omp_get_thread_num());
				
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
				
		//		printf("QMUX: watek %d wychodzi z sekcji krytycznej\n", omp_get_thread_num());
				omp_unset_lock(qmux);
		//		printf("QMUX: watek %d wyszedl z sekcji krytycznej\n", omp_get_thread_num());
		//		printf("FMUX: watek %d wychodzi z sekcji krytycznej\n", omp_get_thread_num());
				omp_unset_lock(fmux);
		//		printf("FMUX: watek %d wyszedl z sekcji krytycznej\n", omp_get_thread_num());
								
				//printf("before RETURN path_cost=%d\n", path_cost - 1);
				return path_cost;
			}
		}	
	}
	return 0;

}