#include "darknet.h"
//network *load_network(char *cfg, char *weights, int clear);
extern network *net;
void loadnet(char *cfg, char *weights);
network *getnet();