package main

import (
	"seaflows/broker"
	"seaflows/listener"
	"seaflows/sflow"
	"sync"
)

var wg sync.WaitGroup

func main() {

	for i := 0; i < 8; i++ {
		msgQ := make(chan sflow.StorableFlow, 32)
		wg.Go(func() { listener.Run(&wg, i, 6343+i, "127.0.0.1", msgQ) })
		wg.Go(func() { broker.Run(&wg, msgQ) })
	}
	wg.Wait()
}
