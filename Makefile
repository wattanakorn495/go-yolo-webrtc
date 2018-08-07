GST_FLAG= `pkg-config --cflags --libs gstreamer-1.0`
INCLUDE_FLAG= -Wall -Iinclude/
CUDA_FLAG= -DGPU -I/usr/local/cuda/include/
DARKNET_FLAG=-ldarknet
THREAD=-pthread

gall : gst_probe_yolo gst_yolo_thread

gst_probe_yolo : gst_probe_yolo.c
	gcc $(INCLUDE_FLAG) -o gst_probe_yolo gst_probe_yolo.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)

gst_yolo_thread : gst_yolo_thread.c
	gcc $(INCLUDE_FLAG) $(THREAD) -o gst_yolo_thread gst_yolo_thread.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)