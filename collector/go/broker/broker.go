package broker

import (
	"seaflows/rrd"
	"seaflows/sflow"
	"sync"
	"time"
)

type bucketNode struct {
	srcMacAddress string
	dstMacAddress string
	bytes4        uint32
	bytes6        uint32
}

var bucket []bucketNode
var mutex sync.Mutex

func flushBucket() {

	for {
		time.Sleep(60 * time.Second)
		mutex.Lock()
		for _, node := range bucket {
			_, err := rrd.Store(node.srcMacAddress, node.dstMacAddress, node.bytes4, node.bytes6)
			if err != nil {
				// do smth
			}
		}
		mutex.Unlock()
	}

}

func addToBucket(srcMacAddress string, dstMacAddress string, proto uint32, numBytes uint32) {
	var found bool = false

	mutex.Lock()
	for _, bkt := range bucket {
		if bkt.srcMacAddress == srcMacAddress && bkt.dstMacAddress == dstMacAddress {
			found = true
			if proto == 4 {
				bkt.bytes4 += numBytes
			} else {
				bkt.bytes6 += numBytes
			}
		}
	}
	if !found {
		if proto == 4 {
			bucket = append(bucket, bucketNode{srcMacAddress, dstMacAddress, numBytes, 0})
		} else {
			bucket = append(bucket, bucketNode{srcMacAddress, dstMacAddress, 0, numBytes})
		}
	}
	mutex.Unlock()
}

func Run(wg *sync.WaitGroup, msgQ chan sflow.StorableFlow) {

	go flushBucket()

	for {
		sf := <-msgQ
		go addToBucket(sf.SrcMacAddress, sf.DstMacAddress, sf.Proto, sf.ComputedSize)
	}
	wg.Done()
}
