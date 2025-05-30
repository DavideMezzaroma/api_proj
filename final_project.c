#include <stdio.h>
#include <stdlib.h>


typedef struct{
	int travel_cost;
	int num_air_routes;
} Tile;


void init(int num_c, int num_r, Tile *map){

	int map_dimension = num_c * num_r;
	map = malloc(map_dimension * sizeof(Tile));
	for(int i = 0; i < map_dimension; i++){
		map[i].travel_cost = 1;
		map[i].num_air_routes = 0;
	}
	printf("OK\n");
}


int main(){

	Tile *map;
	init(10, 20, map);


	for(int i = 0; i < 10*20; i++){
		printf("%d, ", map[i].travel_cost);

	}


	return 0;
}
