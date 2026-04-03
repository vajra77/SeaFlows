package services

import (
	"fmt"
	"log"
	"seaflows/internal/models"
	"sync"
	"time"
)

const shardCount = 32
const offset32 = 2166136261
const prime32 = 16777619

type shard struct {
	mu    sync.Mutex
	items map[string]*models.SflowData
}

type sflowService struct {
	shards        [shardCount]*shard
	ticker        *time.Ticker
	flushInterval time.Duration
	storage       StorageService
	done          chan struct{}
}

func NewSflowService(interval time.Duration, storage StorageService) FlowProcessorService {
	s := &sflowService{
		flushInterval: interval,
		ticker:        time.NewTicker(interval),
		storage:       storage,
		done:          make(chan struct{}),
	}

	for i := 0; i < shardCount; i++ {
		s.shards[i] = &shard{
			items: make(map[string]*models.SflowData),
		}
	}

	return s
}

func (s *sflowService) getShard(src, dst string) *shard {
	var hash uint32 = offset32

	// Hash della sorgente
	for i := 0; i < len(src); i++ {
		hash ^= uint32(src[i])
		hash *= prime32
	}
	// Hash della destinazione
	for i := 0; i < len(dst); i++ {
		hash ^= uint32(dst[i])
		hash *= prime32
	}

	return s.shards[hash%shardCount]
}

// Process processes an sflow data container and stores values
func (s *sflowService) Process(data *models.SflowData) {

	key := fmt.Sprintf("%s-%s-%d", data.SrcMAC, data.DstMAC, data.IPv)
	sh := s.getShard(data.SrcMAC, data.DstMAC)

	sh.mu.Lock()
	defer sh.mu.Unlock()

	// if flow exists, add data
	if existing, ok := sh.items[key]; ok {
		existing.Size += data.SamplingRate * data.Size
		existing.Timestamp = data.Timestamp
	} else {
		newData := *data
		newData.Size = data.SamplingRate * data.Size
		sh.items[key] = &newData
	}
}

// Start starts a ticker process that trigger data flushing every
// flushInterval seconds (60)
func (s *sflowService) Start() {
	for {
		select {
		case <-s.ticker.C:
			s.flush()
		case <-s.done:
			return
		}
	}
}

// Stop stops the processor service
func (s *sflowService) Stop() {
	s.ticker.Stop()

	close(s.done)

	log.Println("[INFO] flushing last data")
	s.flush()
}

// flush dumps data to storage
func (s *sflowService) flush() {
	// Raccogliamo i dati da tutti gli shard
	allData := make([]map[string]*models.SflowData, shardCount)
	for i := 0; i < shardCount; i++ {
		s.shards[i].mu.Lock()
		allData[i] = s.shards[i].items
		s.shards[i].items = make(map[string]*models.SflowData)
		s.shards[i].mu.Unlock()
	}

	// Gruppiamo i flussi per aggregare v4/v6 data
	grouped := make(map[string]*models.AggregatedFlow)

	for _, snapshot := range allData {
		for _, data := range snapshot {
			key := data.SrcMAC + "-" + data.DstMAC
			if _, exists := grouped[key]; !exists {
				grouped[key] = &models.AggregatedFlow{
					SrcMAC: data.SrcMAC,
					DstMAC: data.DstMAC,
				}
			}
			if data.IPv == 4 {
				grouped[key].Bytes4 += data.Size
			} else if data.IPv == 6 {
				grouped[key].Bytes6 += data.Size
			}
		}
	}

	if len(grouped) == 0 {
		return
	}

	// batch store data
	err := s.storage.UpdateFlows(grouped)
	if err != nil {
		log.Println("[ERR] Error while flushing batch:", err)
	}
}
