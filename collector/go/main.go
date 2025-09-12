package main

import (
	"seaflows/listener"
	"sync"
)

var wg sync.WaitGroup

func main() {

	for i := 0; i < 8; i++ {
		wg.Go(func() { listener.Run(i, 6343+i, "127.0.0.1") })
	}
	wg.Wait()
}
