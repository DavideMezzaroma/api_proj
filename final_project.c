#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROUTES 5
#define MAX_CMD 20
#define IN_QUEUE 'Q'
#define VISITED 'V'
#define NOT_IN_QUEUE 'N'

typedef struct Tile Tile;		//forward declaration per evitare errori

typedef struct AirRoute{
	Tile *dest;
	int cost;
} AirRoute;

typedef struct Tile{
	int travel_cost;
	int num_air_routes;
	char queue_state;
	int x, y, o, d; 		//comprende il sistema di coordinate cartesiane e diagonali
	AirRoute routes[MAX_ROUTES];
} Tile;

typedef struct Queue{
	Tile **tiles;
	int *distances;
	int head, end, size;
} Queue;

const int adjacents[6][2] = {{0, 1}, {1, 1}, {1, 0}, {0, -1}, {-1, -1}, {-1, 0}};	//formato {o, d}
int init_r, init_c;

//-------------------------LOGICA DELLA QUEUE------------------------------

Queue *create_queue(int queue_size){
	Queue *queue = malloc(sizeof(Queue));
	queue->tiles = malloc(queue_size * sizeof(Tile*));		//creo un array di Tile*
	queue->distances = malloc(queue_size * sizeof(int));
	queue->head = 0;				//indice di estrazione degli elementi dalla coda (testa)
	queue->end = 0;				//indice di inserimento dei nuovi elementi della coda (coda)
	queue->size = queue_size;
	return queue;
}

int queue_is_empty(Queue *queue){
	return (queue->head == queue->end);	//se inizio e fine coda coincidono allora la coda e' vuota
}

int queue_is_full(Queue *queue){
	return (queue->end == queue->size);	//se fine coda e dimensione massima coincidono allora la coda e' piena
}

void enqueue(Queue *queue, Tile *tile, int distance){
	//se la coda e' gia' piena esci (non dovrebbe succedere mai, ma male non fa)
	if(queue_is_full(queue)){
		return;
	}
	//metto in coda la cella considerata, metto in coda anche la sua distanza ed aumento il contatore della coda
	queue->tiles[queue->end] = tile;
	queue->distances[queue->end] = distance;
	queue->end += 1;
}

Tile *dequeue(Queue *queue, int *distance){
	//se la coda e' vuota esci
	if(queue_is_empty(queue)){
		return NULL;
	}
	//estraggo la cella in testa dall'array di celle, sposto il contatore della testa della coda
	Tile *tile = queue->tiles[queue->head];
	//tile->queue_state = VISITED;
	*distance = queue->distances[queue->head];//modifico la distanza passata per indirizzo cosi' da averla sempre pronta
	queue->head += 1;
	return tile;
}

//---------------------FUNZIONI DI AUSILIO-------------------------

void visualize(Tile *map){
	printf("\n\n");
	for(int y = 0; y < init_r; y++){
		if(y % 2 == 1){
			printf(" ");
		}
		for(int x = 0; x < init_c; x++){
			//printf("%d,%d ", map[y+x*init_r].o, map[y+x*nr].d);
			printf("%d ", map[y+x*init_r].travel_cost);
			//printf("%c ", map[y+x*init_r].queue_state);
			//printf("%d ", map[y+x*init_r].num_air_routes);
			//printf("%d ", map[y+x*init_r].routes[0].cost);
		}
		printf("\n");
	}
}

//resituisce l'indice della tile corretta all'interno della mappa (da' le coordinate xy, date le od)
int tile_od(int o, int d, Tile *map){
	int x = o - ((d+1)>>1);
	//printf("%d,%d ", x, d);
	return x*init_r+d;
}

//resituisce la coordinata o, date xy. y e' uguale a d, quindi non serve ricalcolarla
int tile_xy(int x, int y, Tile *map){
	int o = x + ((y+1)>>1);
	return o;
}

//TODO: da rifare tutta questa funzione in qualche altro modo perche' e' oscena
int hex_distance(Tile *map, int start_x, int start_y, int dest_x, int dest_y){

	//start_d e dest_d si possono sostituire con y per risparmiare un po' di memoria i guess
	int start_d = start_y;
	int start_o = tile_xy(start_x, start_y, map);
	int start_q = start_d - start_o;

	int dest_d = dest_y;
	int dest_o = tile_xy(dest_x, dest_y, map);
	int dest_q = dest_d - dest_o;

	if(dest_o-start_o > dest_d-start_d){
		return abs(dest_o-start_o);
	}
	if(dest_q-start_q > dest_d-start_d){
		return abs(dest_q-start_q);
	}
	return abs(dest_d-start_d);
}


void visit_adj(Tile start, Tile *map){
	map[start.x*init_r+start.y].queue_state = IN_QUEUE;
	for(int i = 0; i < 6; ++i){
		int curr_tile = tile_od(start.o+adjacents[i][0], start.d+adjacents[i][1], map);
		if(map[curr_tile].queue_state == NOT_IN_QUEUE){
			map[curr_tile].queue_state = IN_QUEUE;
		}
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
			map[i].queue_state = NOT_IN_QUEUE;
			//map[i].y = i % num_c;
			//map[i].x = i / num_r;
			map[i].d = i % num_r;
			map[i].o = (i / num_r) + ((map[i].d+1)>>1);
		}
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
	
	//in base al raggio calcolo quante celle devo mettere nella coda e alloco una coda della dimensione giusta
	int max_queue_size = 1 + 3 * r * (r - 1);
	Tile *center = &map[x * init_r + y];
	Queue *queue = create_queue(max_queue_size);
	enqueue(queue, center, 0);		//metto in coda il centro, con distanza 0
	center->queue_state = IN_QUEUE;

	while(!queue_is_empty(queue)){
		int curr_dist;
		Tile *curr_tile = dequeue(queue, &curr_dist);	//passando la distanza posso modificarla nella funzione
		//questa e' inizializzata a 0 per la chiamata enqueue sul centro, quindi da ora la incremento di 1 ogni
		//volta che passo ad un anello piu' largo, senza chiamare hex_distance()
		int increment = v * (r - curr_dist) / r;	//calcolo l'incremento da fare
		curr_tile->travel_cost += increment;		//incremento sia il costo della cella che delle sue rotte aeree
		for(int i = 0; i < curr_tile->num_air_routes; ++i){
			curr_tile->routes[i].cost += increment;
		}

		if(curr_dist < r-1){
			for(int i = 0; i < 6; ++i){
				int adj_o = curr_tile->o+adjacents[i][0];
				int adj_d = curr_tile->d+adjacents[i][1];
				int adj_x = adj_o - ((adj_d+1)>>1);

				Tile *neighbor = &map[adj_x * init_r + adj_d];
				//controllo che la cella non sia fuori dalla mappa (casi con centro vicino al bordo)
				if(adj_d >= 0 && adj_d < init_r && adj_x >= 0 && adj_x < init_c){
					//se non e' in coda, lo aggiungo (non deve essere ne' in coda ne' gia' visitato)
					if(neighbor->queue_state == NOT_IN_QUEUE){
						neighbor->queue_state = IN_QUEUE;
						enqueue(queue, neighbor, curr_dist+1);
					}
				}
			}
		}
	}
	//una volta finita tutta quanta la coda, riuso lo stesso array per pulire i queue_state delle celle interessate
	for(int i = 0; i < queue->end; ++i){
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

	//se la rotta aerea gia' e' presente, rimuoverla
	for(int i = 0; i < start->num_air_routes; ++i){
		//printf("%p, %p\n", (void *)dest, (void *)start->routes[i].dest);
		if(start->routes[i].dest == dest){
			for(int j = i; j < start->num_air_routes-1; ++j){
				start->routes[j] = start->routes[j+1];		//sposto indietro tutte le rimanenti rotte
			}
				start->num_air_routes -= 1;			//decremento il numero di rotte della cella
			printf("OK\n");
			return;
		}
	}
	//se non c'e' spazio per altre rotte aeree, print KO
	if(start->num_air_routes >= MAX_ROUTES){
		printf("KO\n");
		return;
	}
	/*
	 * capire se questo controllo serve davvero
	if(dest->travel_cost < 0){
		printf("KO\n");
		return;
	}
	*/

	//altrimenti 
	int total_cost = start->travel_cost;		//inizializzo il costo a quello della cella di uscita
	for(int i = 0; i < start->num_air_routes; ++i){
		total_cost += start->routes[i].cost;	//aggiungo il costo di ogni rotta aerea presente
	}
	int average = total_cost / (start->num_air_routes + 1);

	start->routes[start->num_air_routes].dest = &map[dest_x * init_r + dest_y];
	start->routes[start->num_air_routes].cost = average;
	start->num_air_routes += 1;
	printf("OK\n");
}



int main(){

	Tile *map = NULL;
	char cmd[MAX_CMD];
	int par_a, par_b, par_c, par_d;

	while(scanf(" %s", cmd) != EOF){
		//	init()
		if(!strcmp(cmd, "init")){
			scanf(" %d %d", &init_c, &init_r);
			map = init(init_c, init_r, map);
		}
		//	change_cost()
		else if(!strcmp(cmd, "change_cost")){
			scanf(" %d %d %d %d", &par_a, &par_b, &par_c, &par_d);

			if(par_a >= 0 && par_b >= 0 && par_a < init_c && par_b < init_r && par_d > 0){
				change_cost(map, par_a, par_b, par_c, par_d);
			}
			else{
				printf("KO\n");
			}
		}
		//	toggle_air_route()
		else if(!strcmp(cmd, "toggle_air_route")){
			scanf(" %d %d %d %d", &par_a, &par_b, &par_c, &par_d);
			if(par_a < 0 || par_a >= init_c || par_b < 0 || par_b >= init_r ||
			   par_c < 0 || par_c >= init_c || par_d < 0 || par_d >= init_r){
				printf("KO\n");
			}
			else{
				toggle_air_route(map, par_a, par_b, par_c, par_d);
			}
		}
		else if(!strcmp(cmd, "travel_cost")){
			scanf(" %d %d %d %d", &par_a, &par_b, &par_c, &par_d);
			//printf("travel_cost(%d, %d, %d, %d)\n", par_a, par_b, par_c, par_d);
		}
		else if(!strcmp(cmd, "v")){
			visualize(map);
		}

	}

	/*
	visit_adj(map[3*init_r+3], map);
	visit_adj(map[4*init_r+6], map);
	visualize(map);
	*/

	free(map);
	return 0;
}
