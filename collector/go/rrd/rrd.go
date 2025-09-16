package rrd

import "C"
import (
	"unsafe"
)

// #cgo CFLAGS: -I../../cc/include/rrdtool.h
// #cgo LDFLAGS: -L../../cc/lib -lrrdtool -Wl,-rpath=../../cc/lib
// #include "rrdtool.h"
import "C"

func Prepare(src string, dst string) (int, error) {
	csrc := C.CString(src)
	cdst := C.CString(dst)
	defer C.free(unsafe.Pointer(csrc))
	defer C.free(unsafe.Pointer(cdst))
	return int(C.rrd_prepare(csrc, cdst)), nil
}

func Store(src string, dst string, bytes4 uint32, bytes6 uint32) (int, error) {
	csrc := C.CString(src)
	cdst := C.CString(dst)
	defer C.free(unsafe.Pointer(csrc))
	defer C.free(unsafe.Pointer(cdst))
	cbytes4 := C.uint32_t(bytes4)
	cbytes6 := C.uint32_t(bytes6)
	return int(C.rrd_store(csrc, cdst, cbytes4, cbytes6)), nil
}
