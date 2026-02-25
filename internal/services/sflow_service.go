package services

import (
	"log"
	"seaflows/internal/models"
	"sync"
	"time"
)

type sflowService struct {
	mu            sync.Mutex
	ticker        *time.Ticker
	items         map[string]*models.SflowData
	flushInterval time.Duration
	storage       StorageService
	done          chan struct{}
}

func NewSflowService(interval time.Duration, storage StorageService) FlowProcessorService {
	return &sflowService{
		items:         make(map[string]*models.SflowData),
		flushInterval: interval,
		ticker:        time.NewTicker(interval),
		storage:       storage,
		done:          make(chan struct{}),
	}
}

// Process processes an sflow data container and stores values
func (s *sflowService) Process(data *models.SflowData) {

	// build a hash key for aggregation of multiple flows
	key := data.SrcMAC + data.DstMAC

	s.mu.Lock()
	defer s.mu.Unlock()

	// if flow exists, add data
	if existing, ok := s.items[key]; ok {
		existing.Size += data.SamplingRate * data.Size
		existing.Timestamp = data.Timestamp
	} else {
		// create new flow with base data
		data.Size = data.SamplingRate * data.Size
		s.items[key] = data
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
	// locks and snap a shot of accumulated data
	s.mu.Lock()
	snapshot := s.items
	// re-initialize data store and unlock
	s.items = make(map[string]*models.SflowData)
	s.mu.Unlock()

	if len(snapshot) == 0 {
		return
	}

	// works on snapshot by furtherly grouping flows to aggregate v4/v6 data
	grouped := make(map[string]*models.AggregatedFlow)

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

	// batch store data
	err := s.storage.UpdateFlows(grouped)
	if err != nil {
		log.Println("[ERR] Error while flushing batch:", err)
	}
}
