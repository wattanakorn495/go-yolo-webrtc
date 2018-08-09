GST_FLAG= `pkg-config --cflags --libs gstreamer-1.0`
INCLUDE_FLAG= -Wall -Iinclude/
CUDA_FLAG= -DGPU -I/usr/local/cuda/include/
DARKNET_FLAG=-ldarknet
THREAD=-pthread

all : gst_probe_yolo gst_yolo_thread #loadnet loadloadnet loadnet_test

gst_probe_yolo : gst_probe_yolo.c
	gcc $(INCLUDE_FLAG) -o gst_probe_yolo gst_probe_yolo.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)

gst_yolo_thread : gst_yolo_thread.c
	gcc $(INCLUDE_FLAG) $(THREAD) -o gst_yolo_thread gst_yolo_thread.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)

#loadnet : loadnet.c
#	gcc $(INCLUDE_FLAG) -c loadnet.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)
#	ar -rc loadnet.a loadnet.o

#loadloadnet : loadloadnet.c
#	gcc -c loadloadnet.c 
#	ar -rc loadloadnet.a loadloadnet.o loadnet.o 
#	#ar -rc loadloadnet.a loadloadnet.o

#loadnet_test : loadnet_test.c
#	gcc -o loadnet_test loadnet_test.c loadloadnet.a $(DARKNET_FLAG)