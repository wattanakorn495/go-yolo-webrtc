#include "loadnet.h"

network *net = NULL;

void loadnet(char *cfg, char *weights)
{
	net = load_network(cfg,weights,0);
	layer l = net->layers[net->n-1];
  	printf("classes: %d\n", l.classes );
}

network *getnet()
{
	return net;
}