package handlers

import (
	"context"
	"fmt"
	"log"
	"net"
	"runtime"
	"seaflows/internal/services"
	"time"

	"seaflows/internal/models"
)

type sFlowHandler struct {
	listenAddr string
	processor  services.FlowProcessorService
	workerPool int
}

func NewSFlowHandler(addr string, processor services.FlowProcessorService) NetHandler {
	return &sFlowHandler{
		listenAddr: addr,
		processor:  processor,
		workerPool: runtime.NumCPU(),
	}
}

func (h *sFlowHandler) Listen(ctx context.Context) error {
	addr, err := net.ResolveUDPAddr("udp", h.listenAddr)
	if err != nil {
		return fmt.Errorf("error while resolving address: %w", err)
	}

	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		return fmt.Errorf("error while opening socket: %w", err)
	}
	defer func(conn *net.UDPConn) {
		err := conn.Close()
		if err != nil {
			log.Printf("error closing socket connection: %v", err)
		}
	}(conn)

	_ = conn.SetReadBuffer(16 * 1024 * 1024)

	packetChan := make(chan []byte, 10000)

	for i := 0; i < h.workerPool; i++ {
		go h.worker(packetChan)
	}

	log.Printf("[INFO] Listening on %s with %d workers", h.listenAddr, h.workerPool)

	go func() {
		for {
			select {
			case <-ctx.Done():
				return
			default:
				buf := make([]byte, 65535)
				n, _, err := conn.ReadFromUDP(buf)
				if err != nil {
					continue
				}
				packetChan <- buf[:n]
			}
		}
	}()

	<-ctx.Done()
	return nil
}

func (h *sFlowHandler) worker(packetChan <-chan []byte) {
	for data := range packetChan {
		var dgram models.Datagram

		if err := dgram.UnmarshalBinary(data); err != nil {
			continue
		}

		now := time.Now().Unix()

		for _, sample := range dgram.Samples {
			for _, record := range sample.Records {
				if record.DataFormat == 1 { // Raw Packet Header
					var flow *models.SflowData
					if record.Packet.DatalinkHeader.EthernetHeader.EthType == 0x0800 { // IPv4
						flow = &models.SflowData{
							Timestamp:    now,
							SrcMAC:       record.Packet.DatalinkHeader.EthernetHeader.SrcMACAddress,
							DstMAC:       record.Packet.DatalinkHeader.EthernetHeader.DstMACAddress,
							IPv:          4,
							SrcIP:        record.Packet.Ipv4Header.SrcIPAddress.String(),
							DstIP:        record.Packet.Ipv4Header.DstIPAddress.String(),
							SamplingRate: sample.SamplingRate,
							Size:         record.Packet.Size,
						}
					} else if record.Packet.DatalinkHeader.EthernetHeader.EthType == 0x86DD { // IPv6
						flow = &models.SflowData{
							Timestamp:    now,
							SrcMAC:       record.Packet.DatalinkHeader.EthernetHeader.SrcMACAddress,
							DstMAC:       record.Packet.DatalinkHeader.EthernetHeader.DstMACAddress,
							IPv:          6,
							SrcIP:        record.Packet.Ipv6Header.SrcIPAddress.String(),
							DstIP:        record.Packet.Ipv6Header.DstIPAddress.String(),
							SamplingRate: sample.SamplingRate,
							Size:         record.Packet.Size,
						}
					} else { // ignore non-IP packets
						continue
					}
					h.processor.Process(flow)
				}
			}
		}
	}
}
