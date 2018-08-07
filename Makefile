GST_FLAG= `pkg-config --cflags --libs gstreamer-1.0`
INCLUDE_FLAG= -Wall -Iinclude/
CUDA_FLAG= -DGPU -I/usr/local/cuda/include/
DARKNET_FLAG=-ldarknet

gst_probe_yolo : gst_probe_yolo.c
	gcc $(INCLUDE_FLAG) -o gst_probe_yolo gst_probe_yolo.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)