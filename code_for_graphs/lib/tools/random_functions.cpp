/******************************************************************************
 * random_functions.cpp 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#include "random_functions.h"

MersenneTwister random_functions::m_mt;
int random_functions::m_seed = 0;
uint32_t random_functions::rand_counter = 0;

random_functions::random_functions()  {
	rand_counter = 0;
}

random_functions::~random_functions() {
}
