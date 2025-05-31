#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROUTES 5
#define MAX_CMD 16


typedef struct Tile{
	int travel_cost;
	int num_air_routes;
	struct Tile* air_routes[MAX_ROUTES];
} Tile;


void visualize(Tile *map, int nc, int nr){
	for(int y = 0; y < nr; y++){
		if(y % 2 == 1){
			printf(" ");
		}
		for(int x = 0; x < nc; x++){
			printf("%d ", map[y+x*nr].travel_cost);
			/*
			int d = y;
			int o = x+((y+1)>>1);
			printf("%d,%d  ",o, d);
			*/
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
		printf("mappa non inizializzata\n");
		return;
	}
	map[x+y].travel_cost = v;
}


int hex_distance(Tile *map, int start_x, int start_y, int dest_x, int dest_y){

	//start_d e dest_d si puossono evitare per risparmiare un po' di memoria i guess
	int start_d = start_y;
	int start_o = start_x + ((start_y+1)>>1);
	int start_q = start_d - start_o;

	int dest_d = dest_y;
	int dest_o = dest_x + ((dest_y+1)>>1);
	int dest_q = dest_d - dest_o;

	if(dest_o-start_o > dest_d-start_d){
		return abs(dest_o-start_o);
	}
	if(dest_q-start_q > dest_d-start_d){
		return abs(dest_q-start_q);
	}
	return abs(dest_d-start_d);
}

int main(){

	Tile *map = NULL;
	char cmd[MAX_CMD];
	int init_r, init_c;
	int ch_c, ch_r, v, r;

	while(scanf(" %s", cmd) != EOF){
		//	init()
		if(!strcmp(cmd, "init")){
			scanf(" %d %d", &init_c, &init_r);
			map = init(init_c, init_r, map);
		}
		//	change_cost()
		else if(!strcmp(cmd, "change_cost")){
			scanf(" %d %d %d %d", &ch_c, &ch_r, &v, &r);
			if(ch_c >= 0 && ch_r >= 0 && ch_c < init_c && ch_r < init_r){
				change_cost(map, ch_c*init_r, ch_r, v, r);
			}
			else{
				printf("Out of bounds\n");
			}
		}
		visualize(map, init_c, init_r);

	}

	free(map);
	return 0;
}
