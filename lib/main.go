package main

// #include "types.h"
import "C"
import "fmt"

func main() {

}

//export sum
func sum(args C.struct_go_args) C.struct_go_args {
	fmt.Printf("here is the object: num: %v str: %+v\n", args.num, C.GoString(args.str))

	var result C.struct_go_args
	result.num = 3
	result.str = C.CString(string("hello world"))
	return result
}
