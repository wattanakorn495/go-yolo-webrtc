#include "avi_yolo.h"
image image_buffer;

int main(int argc, char const *argv[])
{
	image_buffer = make_image(1280, 720, 3);
	char *data_cfg = "cfg/coco.data";
	char *cfg = "cfg/yolov3-tiny.cfg";
	char *weights = "yolov3-tiny.weights";
	init_yolo(data_cfg, cfg, weights);
	return 0;
}

