package listener

import (
	"fmt"
	"net"
	"seaflows/sflow"
)

func Run(id int, port int, address string) {

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
			sflow.Decode(buf[0:nBytes])
		}
	}
}
