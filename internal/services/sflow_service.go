package services

import (
	"log"
	"seaflows/internal/models"
	"sync"
	"time"
)

type sflowService struct {
	mu            sync.Mutex
	items         map[string]*models.SflowData
	flushInterval time.Duration
	storage       StorageService
}

func NewSflowService(interval time.Duration, storage StorageService) FlowProcessorService {
	return &sflowService{
		items:         make(map[string]*models.SflowData),
		flushInterval: interval,
		storage:       storage,
	}
}

func (s *sflowService) Process(data *models.SflowData) {

	key := data.SrcMAC + data.DstMAC

	s.mu.Lock()
	defer s.mu.Unlock()

	if existing, ok := s.items[key]; ok {
		existing.Size += data.SamplingRate * data.Size
		existing.Timestamp = data.Timestamp
	} else {
		data.Size = data.SamplingRate * data.Size
		s.items[key] = data
	}
}

func (s *sflowService) Start() {
	ticker := time.NewTicker(s.flushInterval)
	for range ticker.C {
		s.flush()
	}
}

func (s *sflowService) flush() {
	s.mu.Lock()
	snapshot := s.items
	s.items = make(map[string]*models.SflowData)
	s.mu.Unlock()

	if len(snapshot) == 0 {
		return
	}

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
			grouped[key].Bytes4 += uint32(data.Size)
		} else if data.IPv == 6 {
			grouped[key].Bytes6 += uint32(data.Size)
		}
	}

	err := s.storage.UpdateRRDFiles(grouped)
	if err != nil {
		log.Println("[ERR] Error while flushing batch:", err)
	}
}
