package main

// #include "types.h"
import "C"
import "fmt"

func main() {

}

//export sum
func sum(args C.struct_go_args) C.struct_go_args {
	fmt.Println("here is the object", args)

	var result C.struct_go_args
	result.num = 3
	return result
}
