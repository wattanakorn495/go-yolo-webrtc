package main

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: loadnet.a -ldarknet
#include "loadnet.h"
*/
import "C"
import (
	"fmt"
)

func main() {
	fmt.Println("hello")
	cfg:="cfg/yolov3-tiny.cfg"
	weights:="yolov3-tiny.weights"

	ccfg := C.CString(cfg)
	cweights := C.CString(weights)
	C.loadnet(ccfg,cweights)
}