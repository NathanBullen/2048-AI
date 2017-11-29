#include <time.h>
#include <stdlib.h>
#include "ai.h"
#include "utils.h"
#include "priority_queue.h"

#define NUM_MOVES 3
struct heap h;

/* -Angus
* Angus
* Don't dog us
* Give us a good score
* Don't be a bore
* Be a honey
* Angus.money
* ~Fin
*/
void initialize_ai(){
	heap_init(&h);
}
/**
* Initialise a Node
*/
void initialise_node(node_t* node){
	node->priority = 0;
	node->depth = 0;
	node->num_childs = 0;
	node->parent = NULL;
}
/**
* Copies the board from one node to another
*/
void copyboard(uint8_t copyfrom[SIZE][SIZE], uint8_t copyto[SIZE][SIZE]){
	int x,y;
	for (x=0; x < SIZE; x++){
		for (y=0; y < SIZE; y++){
			copyto[x][y] = copyfrom[x][y];
		}
	}	
}
//Storing parent node and highest score
typedef struct {
	node_t *parentnode;
	double score;
} decision_t;

//Storing array for the moves and decision_t
typedef struct{
	decision_t move[NUM_MOVES+1];
} bestscore_t;

//Dynamic Array
typedef struct {
  node_t *array;
  size_t used;
  size_t size;
} Array;

static int num_generatednodes = 0;
static int num_explorednodes = 0;
static int largestboardvalue = 0;

/**
* Initialise the Array
*/
void initArray(Array *a, size_t initialSize) {
  a->array = (node_t *)malloc(initialSize * sizeof(node_t));
  a->used = 0;
  a->size = initialSize;
}

/**
* Inserting a node into the array, changes the size as well.
*/
void insertArray(Array *a, node_t element) {
  if (a->used == a->size) {
    a->size += 1;
    a->array = (node_t *)realloc(a->array, a->size * sizeof(node_t));
  }
  a->array[a->used++] = element;
}

/**
* Frees the specified Array
*/
void freeArray(Array *explored) {
	int i = 0;
	while (i<size){
		free(explored);
	}
	explored->array = NULL;
	explored->used = explored->size = 0;
}

/**
* Checking if the x/y positions is a corner of the board
*/
bool isCorner(int x, int y) {
	if (!x||!y){
		return false;
	}
	if (x == 0 || x == (SIZE -1 )){
		if (y == 0 || y == (SIZE - 1)){
			return true;
		}
	}
	return false;
}

/**
* Returns the largest value of the board specified
*/
int getLargestValue(uint8_t board[SIZE][SIZE]){
	int largestvalue = 0;
	int x;
	int y;
	for (x=0; x < SIZE; x++){
		for (y=0; y < SIZE; y++){
			if (getTile(board,x,y) > largestvalue){
				largestvalue = getTile(board,x,y);
			}
		}
	}
	return largestvalue;
}

/**
* Gets the first X position searched of the board specified
*/
int getXScoreTile(uint8_t board[SIZE][SIZE], int score){
	int x;
	int y;
	for (x=0; x < SIZE; x++){
		for (y=0; y < SIZE; y++){
			if (getTile(board,x,y) == score){
				return x;
			}
		}
	}
	return -1;
}
/**
* Gets the first Y position searched of the board specified
*/
int getYScoreTile(uint8_t board[SIZE][SIZE], int score){
	int x;
	int y;
	for (x=0; x < SIZE; x++){
		for (y=0; y < SIZE; y++){
			if (getTile(board,x,y) == score){
				return y;
			}
		}
	}
	return -1;
}
/**
* Returns true if a tile next to one specified is one 'level' below
*/
bool getOrderedTile(uint8_t board[SIZE][SIZE], int tile_size, int tile_x, 
	int tile_y){
	int targettile;
	if (tile_size == 2){
		targettile = 2;
	}
	else{
		targettile = tile_size/2;
	}
	if ((tile_x + 1) < SIZE){
		if (board[tile_x+1][tile_y] == targettile){
			return true;
		}
	}
	if ((tile_x - 1) >= 0){
		if (board[tile_x-1][tile_y] == targettile){
			return true;
		}
	}
	if ((tile_y + 1) < SIZE){
		if (board[tile_x][tile_y+1] == targettile){
			return true;
		}
	}
	if ((tile_y - 1) >= 0){
		if (board[tile_x][tile_y-1] == targettile){
			return true;
		}
	}
	return false;
}

/*
* Obtains data from the 2048 file so it can be printed,
* this includes the time the algorithm takes, the score
* and the max depth.
*/
void setData(double timediff, int score, int max_depth){
	FILE *fp;
	fp = fopen("output.txt", "w");
	fprintf(fp, "Max Depth - %i \n", max_depth);
	fprintf(fp, "Generated - %i \n", num_generatednodes);
	fprintf(fp, "Expanded - %i \n", num_explorednodes);
	fprintf(fp, "Time - %.3f seconds\n", timediff);
	fprintf(fp, "Expanded/Second - %.0f \n", ((num_explorednodes)/timediff));
	fprintf(fp, "Max Tile - %i \n", largestboardvalue);
	fprintf(fp, "Score - %i \n", score);
	fclose(fp);
}
/**
 * Find best action by building all possible paths up to depth max_depth
 * and back propagate using either max or avg
 */
move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, 
	propagation_t propagation ){
	if (max_depth == 0){
		largestboardvalue = getLargestValue(board);
		move_t best_action = rand() % (NUM_MOVES + 1);
		return best_action;
	}
	bestscore_t *calcmove = (bestscore_t *)malloc(sizeof(bestscore_t));
	node_t *current = (node_t *)malloc(sizeof(node_t));
	Array explored;
	initialise_node(current);
	initArray(&explored, 2);
	copyboard(board, current->board);
	heap_push(&h, current);
	num_generatednodes += 1;
	num_explorednodes += 1;
	while (h.count > 0){
		node_t *pop = heap_delete(&h);
		insertArray(&explored, *pop);
		if (pop->depth < max_depth){
			int i;
			for(i = 0; i < (NUM_MOVES + 1); i++){
				num_generatednodes += 1;
				node_t *temp = (node_t *)malloc(sizeof(node_t));
				initialise_node(temp);
				temp->depth = pop->depth + 1;
				temp->parent = pop;
				copyboard(pop->board, temp->board);
				if (pop->depth == 0){
					calcmove->move[i].parentnode = temp;
					calcmove->move[i].score = temp->priority;
				}
				if (execute_move_t(temp->board, &temp->priority, i)){
					temp->parent->num_childs += 1;
					//Heuristics - adding random tile
					if (countEmpty(temp->board) < 5){
						addRandom(temp->board);
					}
					heap_push(&h, temp);
					num_explorednodes += 1;
					double totalscore = 0;
					double boardscore = 0;
					double emptyscore = 0;
					double cornerscore = 0;
					while(temp->depth > 1){
						int boardlargest = getLargestValue(temp->board);
						int x = getXScoreTile(temp->board, boardlargest);
						int y = getYScoreTile(temp->board, boardlargest);
						//Corner Heuristic
						if (isCorner(x,y)){
							cornerscore = (boardlargest);
						}
						//Order Heuristic
						if (getOrderedTile(board, x, y, boardlargest)){
							cornerscore += ((boardlargest/2)/(temp->depth));
						}
						//Empty tiles
						emptyscore = (countEmpty(temp->board)*5);
						//calculating weight
						boardscore = ((temp->priority + emptyscore)/
							(temp->depth));
						totalscore += boardscore;
						temp = temp->parent;
					}
					totalscore += temp->priority;
					int k;
					// Propagating moves
					for(k = 0; k < (NUM_MOVES + 1); k++){
						if (temp == calcmove->move[k].parentnode){
							if (propagation == 0){
								if (totalscore > calcmove->move[k].score){
									calcmove->move[k].score = totalscore;
								}
							}
							else if(propagation == 1){
								calcmove->move[k].score += totalscore;
							}
						}
					}
				}
				else{
					//If the move isn't possible
					if (pop->depth == 0){
						//This move shouldn't be even though of in
						//the best score algorithm
						calcmove->move[i].score = -1;
					}
					free(temp);
				}
			temp = NULL;
			}
		pop = NULL;
		}
	}
	move_t best_action = 0;
	double bestscore = 0;
	bool breakeven = false;
	for (int i = 0; i < (NUM_MOVES + 1); i++){
		if((calcmove->move[i].score) > bestscore){
			bestscore = calcmove->move[i].score;
			best_action = i;
			breakeven = false;
		}
		if(calcmove->move[i].score == bestscore){
			breakeven = true;
		}

	}
	//Checking for breaks
	if (breakeven){
		int breakevenscore = -1;
		for (int i = 0; i < (NUM_MOVES + 1); i++){
			if((calcmove->move[i].score < bestscore)){
				calcmove->move[i].score = -1;
			}
		}
		int chosen_one;
		while(breakevenscore == -1){
			chosen_one = rand() % (NUM_MOVES + 1);
			breakevenscore = calcmove->move[chosen_one].score;
		}
		best_action = chosen_one;
	}
	largestboardvalue = getLargestValue(board);
	//Freeing
	emptyPQ(&h);
	freeArray(&explored);
	free(calcmove);
	return best_action;
}
