package main

import (
	"context"
	"fmt"
	"net"
	"os"
	"os/signal"
	"runtime"
	"syscall"
	"time"

	"seaflows/internal/broker"
	"seaflows/internal/sflow"
)

const (
	listenAddress = "0.0.0.0:6343"
	maxUDPSize    = 65535
	flushInterval = 60 * time.Second // Aggreghiamo dati per 1 minuto prima di scrivere su RRD
)

func main() {
	// 1. Inizializziamo il Broker
	// Questo gestisce la memoria e il timer per lo svuotamento verso RRD
	flowBroker := broker.New(flushInterval)
	go flowBroker.Run()

	// 2. Configurazione Socket UDP
	addr, err := net.ResolveUDPAddr("udp", listenAddress)
	if err != nil {
		fmt.Printf("Errore risoluzione: %v\n", err)
		os.Exit(1)
	}

	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		fmt.Printf("Errore apertura socket: %v\n", err)
		os.Exit(1)
	}
	defer conn.Close()

	// Aumentiamo il buffer OS per gestire i picchi (importante contro i NaN)
	conn.SetReadBuffer(16 * 1024 * 1024)

	fmt.Printf("SeaFlows Collector v2 avviato su %s\n", listenAddress)
	fmt.Printf("Utilizzo di %d worker per la decodifica\n", runtime.NumCPU())

	// 3. Pool di Worker
	packetChan := make(chan []byte, 10000)
	for i := 0; i < runtime.NumCPU(); i++ {
		go worker(packetChan, flowBroker)
	}

	// 4. Ciclo di lettura
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	go func() {
		for {
			buf := make([]byte, maxUDPSize)
			n, _, err := conn.ReadFromUDP(buf)
			if err != nil {
				return
			}
			packetChan <- buf[:n]
		}
	}()

	<-ctx.Done()
	fmt.Println("\nSpegnimento... salvataggio ultimi dati in corso.")
	// Qui si potrebbe aggiungere un flowBroker.Flush() finale per non perdere nulla
}

func worker(packetChan <-chan []byte, b *broker.Broker) {
	for buf := range packetChan {
		// A. Decodifica sFlow (con BigEndian e MAC puliti)
		dgram, err := sflow.Decode(buf)
		if err != nil {
			continue
		}

		// B. Estrazione dei flussi dai campioni
		for _, sample := range dgram.Samples {
			for _, record := range sample.Records {
				if record.DataFormat == 1 { // Raw Packet Header

					// Creiamo l'oggetto da passare al broker
					flow := &sflow.StorableFlow{
						SrcMacAddress: record.Packet.DatalinkHeader.EthernetHeader.SrcMacAddress,
						DstMacAddress: record.Packet.DatalinkHeader.EthernetHeader.DstMacAddress,
						SamplingRate:  sample.SamplingRate,
						Size:          record.Packet.Size,
					}

					// C. Invio al broker per l'aggregazione
					b.Add(flow)
				}
			}
		}
	}
}
