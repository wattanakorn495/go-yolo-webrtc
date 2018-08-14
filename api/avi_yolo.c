#include "avi_yolo.h"
static network *net;
static char **names;
static image **alphabet;
static detection *dets;
static layer l;

static float thresh = 0.5;
static float hier = 0.5;
static float nms = 0.4;

static int nboxes = 0;

void init_yolo(char *data_cfg,char *cfg, char *weights)
{
	/* init yolo */
	list *options = read_data_cfg(data_cfg);
	char *name_list = option_find_str(options, "names", "data/names.list");
	names = get_labels(name_list);
	alphabet = load_alphabet();

	net = load_network(cfg,weights,0);
	layer l = net->layers[net->n-1];
  	printf("classes: %d\n", l.classes );
}

void predict_image(image img)
{
	network_predict_image(net, img);
	dets = get_network_boxes(net, 1280, 720, thresh, hier, 0, 1, &nboxes);

    l = net->layers[net->n-1];
    if(nms > 0)
    { 
      do_nms_obj(dets, nboxes, l.classes, nms);
    }
    //printf("nboxes: %d, classes: %d\n", nboxes, l.classes);
}

void draw(image img)
{
	draw_detections(img, dets, nboxes, thresh, names, alphabet, l.classes);
}
