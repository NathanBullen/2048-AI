#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"


void initialize_ai();

move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation );
void setData(double time, int score, int max_depth);
#endif