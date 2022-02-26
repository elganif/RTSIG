#ifndef GLOBAL_VARIABLES
#define GLOBAL_VARIABLES

static const int arenaSize = 100;
enum Team {
    WEST = 1,
    NORTH = 2,
    EAST = 3,
    SOUTH = 4,
    
    };
    float westDistVec[arenaSize][arenaSize];
    float northDistVec[arenaSize][arenaSize];
    float eastDistVec[arenaSize][arenaSize];
    float southDistVec[arenaSize][arenaSize];
#endif
