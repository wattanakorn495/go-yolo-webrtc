package main

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: libavi_yolo.a -ldarknet
#include "avi_yolo.h"
*/
import "C"
import (
	"fmt"
)

func main() {
	fmt.Println("hello")
	data_cfg := "cfg/coco.data"
	cfg:="cfg/yolov3-tiny.cfg"
	weights:="yolov3-tiny.weights"

	cdata_cfg := C.CString(data_cfg)
	ccfg := C.CString(cfg)
	cweights := C.CString(weights)
	//C.loadnet(ccfg,cweights)
	C.init_yolo(cdata_cfg, ccfg, cweights);
}