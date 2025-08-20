#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define MAX_ROUTES 5
#define MAX_CMD 20
#define INF 2147483647
#define IN_QUEUE 'Q'
#define VISITED 'V'
#define NOT_IN_QUEUE 'N'
#define MAX_CACHE 200

int adjacents[6][2] = {{0, 1}, {1, 1}, {1, 0}, {0, -1}, {-1, -1}, {-1, 0}};	//formato {o, d}
int init_r, init_c, cache_size;

typedef struct Tile Tile;		//forward declaration per evitare errori

typedef struct AirRoute{
	Tile *dest;
	int cost;
} AirRoute;

typedef struct Tile{
	int travel_cost;
	int num_air_routes;
	char queue_state;
	//int x, y; 
	int o, d; 		//comprende il sistema di coordinate cartesiane e diagonali
	//AirRoute routes[MAX_ROUTES];
	AirRoute *routes;
	int dijkstra_cost;		//costo temporaneo per algoritmo di Dijkstra
	char dijkstra_visited;		//flag per controllare se una cella e' gia' stata visitata con Dijkstra
} Tile;

typedef struct Queue{
	Tile **tiles;
	int *distances;
	int head, tail, size;
} Queue;

/*
typedef struct HeapItem{
	int cost;
	Tile *dest;
} HeapItem;
*/

typedef struct MinHeap{
	//HeapItem *items;
	int *cost;
	Tile **dest;
	int size, length;
} MinHeap;

typedef struct DistCache{
	int start_idx;
	int dest_idx;
	int cost;
} DistCache;
DistCache cache[MAX_CACHE];

//-------------------------LOGICA DELLA QUEUE------------------------------

Queue *create_queue(int queue_size){
	Queue *queue = malloc(sizeof(Queue));
	queue->tiles = malloc(queue_size * sizeof(Tile*));		//creo un array di Tile*
	queue->distances = malloc(queue_size * sizeof(int));		//array di distanze dal centro
	queue->head = 0;				//indice di estrazione degli elementi dalla coda (testa)
	queue->tail = 0;				//indice di inserimento dei nuovi elementi della coda (coda)
	queue->size = queue_size;
	return queue;
}


void enqueue(Queue *queue, Tile *tile, int distance){
	//se la coda e' gia' piena esci (non dovrebbe succedere mai, ma male non fa)
	if(queue->tail == queue->size){
		return;
	}
	//metto in coda la cella considerata, metto in coda anche la sua distanza ed aumento il contatore della coda
	queue->tiles[queue->tail] = tile;
	queue->distances[queue->tail] = distance;
	queue->tail += 1;
}

Tile *dequeue(Queue *queue, int *distance){
	//se la coda e' vuota esci
	if(queue->head == queue->tail){
		return NULL;
	}
	//estraggo la cella in testa dall'array di celle, sposto il contatore della testa della coda
	Tile *tile = queue->tiles[queue->head];
	*distance = queue->distances[queue->head];//modifico la distanza passata per indirizzo cosi' da averla sempre pronta
	queue->head += 1;
	return tile;
}

//----------------------LOGICA DELL'HEAP----------------------------

MinHeap *create_minheap(int heap_size){
	MinHeap *heap = malloc(sizeof(MinHeap));
	//heap->items = malloc(heap_size * sizeof(HeapItem));
	heap->dest = malloc(heap_size * sizeof(Tile*));
	heap->cost = malloc(heap_size * sizeof(int));
	heap->length = 0;
	heap->size = heap_size;
	return heap;
}

void heap_swap(MinHeap *heap, int a, int b){
	/*
	HeapItem tmpitem = heap->items[a];
	heap->items[a] = heap->items[b];
	heap->items[b] = tmpitem;
	*/
	int tmp_cost = heap->cost[a];
	heap->cost[a] = heap->cost[b];
	heap->cost[b] = tmp_cost;
	Tile* tmp_dest = heap->dest[a];
	heap->dest[a] = heap->dest[b];
	heap->dest[b] = tmp_dest;
}

void push(MinHeap *heap, Tile *tile, int dijkstra_cost){
	//se l'heap e' gia' pieno return
	if(heap->length == heap->size){
		return;
	}

	//altrimenti aggiungi in fondo all'array e poi riordinalo come heap
	int curr_idx = heap->length;
	//heap->items[curr_idx].dest = tile;
	heap->dest[curr_idx] = tile;
	//heap->items[curr_idx].cost = dijkstra_cost;
	heap->cost[curr_idx] = dijkstra_cost;

	int is_heap = 0;		//flag per determinare quando smettere di riordinare
	while(!is_heap){
		int parent_idx = (curr_idx - 1) / 2;

		//if(heap->items[curr_idx].cost < heap->items[parent_idx].cost){
		if(heap->cost[curr_idx] < heap->cost[parent_idx]){
			heap_swap(heap, curr_idx, parent_idx);
			curr_idx = parent_idx;
		}
		else{
			is_heap = 1;
		}
	}

	heap->length += 1;
}

Tile* pop(MinHeap *heap, int* dijkstra_cost){

	/*
	HeapItem first = heap->items[0];
	heap->items[0] = heap->items[heap->length-1];
	*/
	Tile* first_dest = heap->dest[0];
	int first_cost = heap->cost[0];
	heap->dest[0] = heap->dest[heap->length-1];
	heap->cost[0] = heap->cost[heap->length-1];

	heap->length -= 1;

	int is_heap = 0;		//flag per determinare quando smettere di riordinare
	int curr_idx = 0;
	while(!is_heap){
		int lchild_idx = (curr_idx * 2) + 1;
		int rchild_idx = (curr_idx * 2) + 2;
		int new_first_idx = curr_idx;

		//se l'array e' sufficientemente lungo && il figlio sinistro ha costo minore del nuovo primo li scambio
		//if(lchild_idx < heap->length && heap->items[lchild_idx].cost < new_first_cost){
		if(lchild_idx < heap->length && heap->cost[lchild_idx] < heap->cost[new_first_idx]){
			new_first_idx = lchild_idx;
		}
		//analogamente per il figlio destro
		//if(rchild_idx < heap->length && heap->items[rchild_idx].cost < new_first_cost){
		if(rchild_idx < heap->length && heap->cost[rchild_idx] < heap->cost[new_first_idx]){
			new_first_idx = rchild_idx;
		}

		//se il nuovo primo e' uguale all'indice corrente allora esco altrimenti continuo
		if(new_first_idx == curr_idx){
			is_heap = 1;
		}
		else{
			heap_swap(heap, new_first_idx, curr_idx);
			curr_idx = new_first_idx;
		}
	}
	*dijkstra_cost = first_cost;
	return first_dest;
}

void reset_cache(){
	for(int i = 0; i < cache_size; ++i){
		cache[i].start_idx = -1;
		cache[i].dest_idx = -1;
		cache[i].cost = -1;
	}
	cache_size = 0;
}

//---------------------FUNZIONI DI AUSILIO-------------------------

void visualize(Tile *map, int opt){
	printf("\n\n");
	for(int y = 0; y < init_r; y++){
		if(y % 2 == 1){
			printf(" ");
		}
		for(int x = 0; x < init_c; x++){
			//printf("%d,%d ", map[y+x*init_r].o, map[y+x*nr].d);
			if(opt == 0){
				printf("%d ", map[y+x*init_r].travel_cost);
			}
			if(opt == 1){
				if(map[y+x*init_r].dijkstra_cost == INF){
					printf("0 ");
				}
				else{
					printf("%d ", map[y+x*init_r].dijkstra_cost);
				}
			}
		}
		printf("\n");
	}
}


//-----------------------------FUNZIONI RICHIESTE DAL PROGETTO----------------------------

Tile* init(int num_c, int num_r, Tile *map){

	if(map != NULL){
		free(map);
	}
	int map_dimension = num_c * num_r;
	map = malloc(map_dimension * sizeof(Tile));
	if(map != NULL){
		for(int i = 0; i < map_dimension; i++){
			map[i].travel_cost = 1;
			map[i].num_air_routes = 0;
			map[i].routes = NULL;
			map[i].queue_state = NOT_IN_QUEUE;
			//map[i].y = i % num_c;
			//map[i].x = i / num_r;
			map[i].d = i % num_r;
			map[i].o = (i / num_r) + ((map[i].d+1)>>1);
			map[i].dijkstra_cost = INF;
			map[i].dijkstra_visited = NOT_IN_QUEUE;		//riutilizzo le flag definite per la coda
		}
		for(int i = 0; i < MAX_CACHE; ++i){
			cache[i].start_idx = -1;
			cache[i].dest_idx = -1;
			cache[i].cost = -1;
		}
		cache_size = 0;
		//reset_cache(MAX_CACHE);
	}
	else{
		printf("Errore durante l'allocazione dinamica\n");
		return NULL;
	}

	printf("OK\n");
	return map;
}


void change_cost(Tile *map, int x, int y, int v, int r){

	if(map == NULL){
		printf("KO\n");			//mappa non inizializzata
		return;
	}
	if(v < -10 || v > 10){
		printf("KO\n");			//valore di v non valido
		return;
	}

	reset_cache(cache_size);
	
	//in base al raggio calcolo quante celle devo mettere nella coda e alloco una coda della dimensione giusta
	int max_queue_size = 1 + 3 * r * (r - 1);
	Tile *center = &map[x * init_r + y];
	Queue *queue = create_queue(max_queue_size);
	enqueue(queue, center, 0);		//metto in coda il centro, con distanza 0
	center->queue_state = IN_QUEUE;

	while(queue->head != queue->tail){
		int curr_dist;
		Tile *curr_tile = dequeue(queue, &curr_dist);	//passando la distanza posso modificarla nella funzione
		//questa e' inizializzata a 0 per la chiamata enqueue sul centro, quindi da ora la incremento di 1 ogni
		//volta che passo ad un anello piu' largo

		int increment;
		increment = (v * (r - curr_dist)) / r;
		if(v < 0){
			increment = floor(increment - 1);
		}

		curr_tile->travel_cost += increment;		//incremento sia il costo della cella che delle sue rotte aeree
		for(int i = 0; i < curr_tile->num_air_routes; ++i){
			curr_tile->routes[i].cost += increment;
		}

		if(curr_dist < r-1){
			for(int i = 0; i < 6; ++i){
				int adj_o = curr_tile->o+adjacents[i][0];
				int adj_d = curr_tile->d+adjacents[i][1];
				int adj_x = adj_o - ((adj_d+1)>>1);

				//controllo che la cella non sia fuori dalla mappa (casi con centro vicino al bordo)
				if(adj_d >= 0 && adj_d < init_r && adj_x >= 0 && adj_x < init_c){
					//se non e' in coda, lo aggiungo (non deve essere ne' in coda ne' gia' visitato)
					Tile *neighbor = &map[adj_x * init_r + adj_d];
					if(neighbor->queue_state == NOT_IN_QUEUE){
						neighbor->queue_state = IN_QUEUE;
						enqueue(queue, neighbor, curr_dist+1);
					}
				}
			}
		}
	}
	//una volta finita tutta quanta la coda, riuso lo stesso array per pulire i queue_state delle celle interessate
	for(int i = 0; i < queue->tail; ++i){
		queue->tiles[i]->queue_state = NOT_IN_QUEUE;
	}
	free(queue->tiles);
	free(queue->distances);
	free(queue);
	printf("OK\n");
}


void toggle_air_route(Tile *map, int start_x, int start_y, int dest_x, int dest_y){

	Tile *start = &map[start_x * init_r + start_y];
	Tile *dest = &map[dest_x * init_r + dest_y];

	if(start->routes == NULL){
		start->routes = malloc(MAX_ROUTES * sizeof(AirRoute));
	}

	//se la rotta aerea gia' e' presente, rimuoverla
	for(int i = 0; i < start->num_air_routes; ++i){

		if(start->routes[i].dest == dest){
			for(int j = i; j < start->num_air_routes-1; ++j){
				start->routes[j] = start->routes[j+1];		//sposto indietro tutte le rimanenti rotte
			}
				start->num_air_routes -= 1;			//decremento il numero di rotte della cella
				
			reset_cache(cache_size);
			printf("OK\n");
			return;
		}
	}
	//se non c'e' spazio per altre rotte aeree, print KO
	if(start->num_air_routes >= MAX_ROUTES){
		printf("KO\n");
		return;
	}

	//altrimenti 
	int total_cost = start->travel_cost;		//inizializzo il costo a quello della cella di uscita
	for(int i = 0; i < start->num_air_routes; ++i){
		total_cost += start->routes[i].cost;	//aggiungo il costo di ogni rotta aerea presente
	}
	int average = total_cost / (start->num_air_routes + 1);
	//imposto il costo della rotta aerea appena inserita al valore calcolato
	start->routes[start->num_air_routes].dest = &map[dest_x * init_r + dest_y];
	start->routes[start->num_air_routes].cost = average;
	start->num_air_routes += 1;
	reset_cache(cache_size);
	printf("OK\n");
}


void travel_cost(Tile* map, int start_x, int start_y, int dest_x, int dest_y){

	int start_idx = start_x * init_r + start_y;
	int dest_idx = dest_x * init_r + dest_y;
	int need_caching = 0;
	
	for(int i = 0; i < init_r * init_c; ++i){
		map[i].dijkstra_cost = INF;
		map[i].dijkstra_visited = NOT_IN_QUEUE;
	}

	for(int i = 0; i < cache_size; ++i){
		if(start_idx == cache[i].start_idx && dest_idx == cache[i].dest_idx){
			printf("%d\n", cache[i].cost);
			return;
		}
	}

	if(cache_size < MAX_CACHE){
		cache[cache_size].start_idx = start_idx;
		cache[cache_size].dest_idx = dest_idx;
		need_caching = 1;
	}


	int max_heap_size = init_r * init_c;
	MinHeap *heap = create_minheap(max_heap_size);
	map[start_idx].dijkstra_cost = 0;
	push(heap, &map[start_idx], 0);


	int found = 0;
	//finche' c'e' qualcosa "in coda" e non e' gia' stato trovato un percorso esegui il loop
	while(heap->length > 0 && !found){
		int curr_cost = 0;
		Tile* curr_tile = pop(heap, &curr_cost);
		//Tile *curr_tile = curr_item.dest;

		//se la cella attuale ha costo in heap diverso da quello di dijkstra significa che e' stata modificata dopo essere stata inserita nell'heap e va scartata
		//if(curr_item.cost != curr_tile->dijkstra_cost){
		if(curr_cost != curr_tile->dijkstra_cost){
			continue;
		}
		//se ho trovato la destinazione esco subito
		if(curr_tile == &map[dest_idx]){
			found = 1;
			break;
		}
		//se la cella attuale ha costo negativo oppure e' gia' stata visitata saltala
		if(curr_tile->travel_cost <= 0){
			continue;
		}
		if(curr_tile->dijkstra_visited == VISITED){
			continue;
		}
		curr_tile->dijkstra_visited = VISITED;

		for(int i = 0; i < 6; ++i){
			int adj_o = curr_tile->o+adjacents[i][0];
			int adj_d = curr_tile->d+adjacents[i][1];
			if(adj_d < 0 || adj_d >= init_r){
				continue;
			}
			int adj_x = adj_o - ((adj_d+1)>>1);
			if(adj_x < 0 || adj_x >= init_c){
				continue;
			}
			//controllo che la cella non sia fuori dalla mappa

			int adj_idx = adj_x * init_r + adj_d;
			Tile *adjacent = &map[adj_idx];
			if(adjacent->dijkstra_visited == VISITED){
				continue;
			}
			int total_cost = curr_tile->dijkstra_cost + curr_tile->travel_cost;
			if(total_cost < adjacent->dijkstra_cost){
				adjacent->dijkstra_cost = total_cost;
				push(heap, adjacent, adjacent->dijkstra_cost);
			}
		}

		for(int i = 0; i < curr_tile->num_air_routes; ++i){
			Tile *air_dest = curr_tile->routes[i].dest;
			if(curr_tile->routes[i].cost <= 0 || air_dest->dijkstra_visited == VISITED){
				continue;
			}
			int total_cost = curr_tile->dijkstra_cost + curr_tile->routes[i].cost;
			if(total_cost < air_dest->dijkstra_cost){
				air_dest->dijkstra_cost = total_cost;
				push(heap, air_dest, air_dest->dijkstra_cost);
			}
		}
	}
	if(found){
		if(need_caching){
			cache[cache_size].cost = map[dest_idx].dijkstra_cost;
			cache_size += 1;
		}
		printf("%d\n", map[dest_idx].dijkstra_cost);
	}
	else{
		if(need_caching){
			cache[cache_size].cost = -1;
			cache_size += 1;
		}
		printf("-1\n");
	}
	//free(heap->items);
	free(heap->cost);
	free(heap->dest);
	free(heap);
}


int main(){

	Tile *map = NULL;
	char cmd[MAX_CMD];
	int par_a, par_b, par_c, par_d;
	int useless;

	while(scanf(" %s", cmd) != EOF){
		//	init()
		if(!strcmp(cmd, "init")){
			useless = scanf(" %d %d", &init_c, &init_r);
			map = init(init_c, init_r, map);
		}
		//	change_cost()
		else if(!strcmp(cmd, "change_cost")){
			useless = scanf(" %d %d %d %d", &par_a, &par_b, &par_c, &par_d);

			if(par_a >= 0 && par_b >= 0 && par_a < init_c && par_b < init_r && par_d > 0){
				change_cost(map, par_a, par_b, par_c, par_d);
			}
			else{
				printf("KO\n");
			}
		}
		//	toggle_air_route()
		else if(!strcmp(cmd, "toggle_air_route")){
			useless = scanf(" %d %d %d %d", &par_a, &par_b, &par_c, &par_d);
			if(par_a < 0 || par_a >= init_c || par_b < 0 || par_b >= init_r ||
			   par_c < 0 || par_c >= init_c || par_d < 0 || par_d >= init_r){
				printf("KO\n");
			}
			else{
				toggle_air_route(map, par_a, par_b, par_c, par_d);
			}
		}
		else if(!strcmp(cmd, "travel_cost")){
			useless = scanf(" %d %d %d %d", &par_a, &par_b, &par_c, &par_d);
			if(map == NULL){
				printf("-1\n");
			}
			else if(par_a == par_c && par_b == par_d){
				printf("0\n");			//se inizio e destinazione sono uguali stampa 0
			}
			else if(par_a < 0 || par_a >= init_c || par_b < 0 || par_b >= init_r ||
			   	par_c < 0 || par_c >= init_c || par_d < 0 || par_d >= init_r){
				printf("-1\n");			//se inizio o destinazione sono fuori dalla mappa stampa -1
			}
			else{
				travel_cost(map, par_a, par_b, par_c, par_d);
			}
		}
		else if(!strcmp(cmd, "v")){
			int opt;
			useless = scanf(" %d", &opt);
			visualize(map, opt);
		}
		(void)useless;

	}

	free(map);
	return 0;
}
