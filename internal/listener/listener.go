package listener

import (
	"fmt"
	"net"
	"seaflows/sflow"
	"sync"
	"time"
)

func Run(wg *sync.WaitGroup, id int, port int, address string, msgQ chan sflow.StorableFlow) {

	var buf [16384]byte

	addrString := fmt.Sprintf("%s:%d", address, port)
	udpAddr, err := net.ResolveUDPAddr("udp", addrString)

	if err != nil {
		fmt.Println(err)
		return
	}

	conn, err := net.ListenUDP("udp", udpAddr)

	if err != nil {
		fmt.Println(err)
		return
	}

	for {
		nBytes, _, err := conn.ReadFromUDP(buf[0:])
		if err != nil {
			fmt.Println(err)
		} else {
			var dgram *sflow.Datagram
			dgram, err = sflow.Decode(buf[0:nBytes])
			if err != nil {
				for _, sample := range dgram.Samples {
					for _, record := range sample.Records {
						if record.Packet.Protocol == 0x0800 {
							sf := sflow.StorableFlow{
								Timestamp:     uint32(time.Now().Unix()),
								SrcMacAddress: record.Packet.DatalinkHeader.EthernetHeader.SrcMacAddress.String(),
								DstMacAddress: record.Packet.DatalinkHeader.EthernetHeader.DstMacAddress.String(),
								Proto:         4,
								SrcIPAddress:  record.Packet.Ipv4Header.SrcIPAddress.String(),
								DstIPAddress:  record.Packet.Ipv4Header.DstIPAddress.String(),
								SamplingRate:  sample.SamplingRate,
								ComputedSize:  record.Packet.Size,
								Size:          sample.SamplingRate * record.Packet.Size,
							}
							msgQ <- sf
						} else {
							sf := sflow.StorableFlow{
								Timestamp:     uint32(time.Now().Unix()),
								SrcMacAddress: record.Packet.DatalinkHeader.EthernetHeader.SrcMacAddress.String(),
								DstMacAddress: record.Packet.DatalinkHeader.EthernetHeader.DstMacAddress.String(),
								Proto:         4,
								SrcIPAddress:  record.Packet.Ipv4Header.SrcIPAddress.String(),
								DstIPAddress:  record.Packet.Ipv4Header.DstIPAddress.String(),
								SamplingRate:  sample.SamplingRate,
								ComputedSize:  record.Packet.Size,
								Size:          sample.SamplingRate * record.Packet.Size,
							}
							msgQ <- sf
						}
					}
				}
			}
		}
	}
	wg.Done()
}
