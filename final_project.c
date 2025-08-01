#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROUTES 5
#define MAX_CMD 20
#define IN_QUEUE 'Q'
#define VISITED 'V'
#define NOT_IN_QUEUE 'N'

typedef struct Tile Tile;		//forward declaration per evitare errori del cazzo

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

const int adjacents[6][2] = {{0, 1}, {1, 1}, {1, 0}, {0, -1}, {-1, -1}, {-1, 0}};	//formato {o, d}
int init_r, init_c;

void visualize(Tile *map, int nc, int nr){
	printf("\n\n");
	for(int y = 0; y < nr; y++){
		if(y % 2 == 1){
			printf(" ");
		}
		for(int x = 0; x < nc; x++){
			//printf("%d,%d ", map[y+x*nr].o, map[y+x*nr].d);
			//printf("%d ", map[y+x*nr].travel_cost);
			//printf("%c ", map[y+x*nr].queue_state);
			printf("%d ", map[y+x*nr].num_air_routes);
			//printf("%d ", map[y+x*nr].routes[0].cost);
		}
		printf("\n");
	}
}


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
			map[i].d = i % num_c;			//anche qui, d ed y sono la stessa cosa, potrei toglierne uno
			map[i].o = i / num_r +((map[i].d+1)>>1);
		}
		printf("OK\n");
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
		printf("KO\n");
		return;
	}
	map[x+y].travel_cost += v;
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
	if(dest->travel_cost < 0){
		printf("KO\n");
		return;
	}
	capire se e' realmente da mettere questo controllo
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

	//start_d e dest_d si puossono sostituire con y per risparmiare un po' di memoria i guess
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

			if(par_a >= 0 && par_b >= 0 && par_a < init_c && par_b < init_r){
				change_cost(map, par_a*init_r, par_b, par_c, par_d);
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
			visualize(map, init_c, init_r);
		}

	}

	/*
	visit_adj(map[3*init_r+3], map);
	visit_adj(map[4*init_r+6], map);
	visualize(map, init_c, init_r);
	*/

	free(map);
	return 0;
}
