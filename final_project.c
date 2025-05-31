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
	for(int i = 0; i < nr; i++){
		for(int j = 0; j < nc; j++){
			printf("%d ", map[i+j*nr].travel_cost);
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



	//map = init(10, 20, map);
	//change_cost(map, 1, 2, 3, 4);

	

	free(map);
	return 0;
}
