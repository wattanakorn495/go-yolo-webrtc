#include "loadnet.h"

int main(int argc, char const *argv[])
{
	char *cfg = "cfg/yolov3-tiny.cfg";
  char *weights = "yolov3-tiny.weights";
  loadnet(cfg,weights);
  //network *net;
  //net = getnet();
  layer l = net->layers[net->n-1];
  printf("classes: %d\n", l.classes );
  return 0;
}