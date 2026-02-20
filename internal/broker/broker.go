package broker

import (
	"seaflows/internal/rrd" // Gestirà la scrittura fisica
	"seaflows/internal/sflow"
	"sync"
	"time"
)

type FlowKey struct {
	SrcMac string
	DstMac string
}

type AggregatedData struct {
	Bytes    uint64
	LastSeen time.Time
}

type Broker struct {
	mu       sync.Mutex
	bucket   map[FlowKey]*AggregatedData
	interval time.Duration
}

func New(flushInterval time.Duration) *Broker {
	return &Broker{
		bucket:   make(map[FlowKey]*AggregatedData),
		interval: flushInterval,
	}
}

// Add aggiunge un sample decodificato al bucket in memoria
func (b *Broker) Add(sample *sflow.StorableFlow) {
	key := FlowKey{
		SrcMac: sample.SrcMacAddress,
		DstMac: sample.DstMacAddress,
	}

	b.mu.Lock()
	defer b.mu.Unlock()

	if _, exists := b.bucket[key]; !exists {
		b.bucket[key] = &AggregatedData{}
	}

	// Calcolo della dimensione stimata (SamplingRate * PacketSize)
	// Questo corregge il valore sottostimato che causava grafici vuoti
	b.bucket[key].Bytes += uint64(sample.SamplingRate) * uint64(sample.Size)
	b.bucket[key].LastSeen = time.Now()
}

// Run avvia il ciclo di svuotamento (Flush) periodico
func (b *Broker) Run() {
	ticker := time.NewTicker(b.interval)
	for range ticker.C {
		b.flush()
	}
}

func (b *Broker) flush() {
	// Fase di "Swap": prendiamo i dati e resettiamo il bucket sotto lock breve
	b.mu.Lock()
	snaphot := b.bucket
	b.bucket = make(map[FlowKey]*AggregatedData)
	b.mu.Unlock()

	// Ora processiamo lo snapshot fuori dal lock principale.
	// Questo permette al Collector di continuare a ricevere pacchetti
	// mentre noi scriviamo su RRD (operazione lenta).
	for key, data := range snaphot {
		// Passiamo i dati al modulo RRD che comporrà il path
		// /srv/rrd/flows/<macsrc>/flow_<macsrc>_to_<macdst>.rrd
		rrd.Update(key.SrcMac, key.DstMac, data.Bytes)
	}
}
