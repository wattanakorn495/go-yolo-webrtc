GST_FLAG= `pkg-config --cflags --libs gstreamer-1.0`
INCLUDE_FLAG= -Wall -Iinclude/
CUDA_FLAG= -DGPU -I/usr/local/cuda/include/
DARKNET_FLAG=-ldarknet
THREAD=-pthread

all : gst_probe_yolo gst_yolo_thread loadnet api api_test gst_yolo_api #loadloadnet loadnet_test

gst_probe_yolo : gst_probe_yolo.c
	gcc $(INCLUDE_FLAG) -o gst_probe_yolo gst_probe_yolo.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)

gst_yolo_thread : gst_yolo_thread.c
	gcc $(INCLUDE_FLAG) $(THREAD) -o gst_yolo_thread gst_yolo_thread.c $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)

loadnet : test/loadnet.c
	gcc $(INCLUDE_FLAG) -c test/loadnet.c -o test/loadnet.o $(DARKNET_FLAG) $(CUDA_FLAG) $(GST_FLAG)
	ar -rc test/libloadnet.a test/loadnet.o
	#gcc  test/loadnet_test.c test/loadnet.a -ldarknet -o test/loadnet_test
	gcc -o test/loadnet_test test/loadnet_test.c test/libloadnet.a -ldarknet

api : api/avi_yolo.c
	gcc $(INCLUDE_FLAG) -c api/avi_yolo.c -o api/avi_yolo.o $(DARKNET_FLAG) $(CUDA_FLAG)
	ar -rc api/libavi_yolo.a api/avi_yolo.o 

api_test : api/avi_yolo_test.c
	gcc -o  api/test api/avi_yolo_test.c api/libavi_yolo.a -ldarknet

gst_yolo_api : gst_yolo_api.c
	gcc -o gst_yolo_api gst_yolo_api.c api/libavi_yolo.a -ldarknet $(GST_FLAG)

#loadloadnet : loadloadnet.c
#	gcc -c loadloadnet.c 
#	ar -rc loadloadnet.a loadloadnet.o loadnet.o 
#	#ar -rc loadloadnet.a loadloadnet.o

#loadnet_test : loadnet_test.c
#	gcc -o loadnet_test loadnet_test.c loadloadnet.a $(DARKNET_FLAG)