#include "darknet.h"

void init_yolo(char *data_cfg,char *cfg, char *weights);
void predict_image(image img);
void draw(image img);