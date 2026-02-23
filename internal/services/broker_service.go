package services

import (
	"log"
	"seaflows/internal/models"
	"sync"
	"time"
)

type brokerService struct {
	mu            sync.Mutex
	items         map[string]*models.SflowData
	flushInterval time.Duration
	storage       StorageService
}

func NewBrokerService(interval time.Duration, storage StorageService) FlowProcessorService {
	return &brokerService{
		items:         make(map[string]*models.SflowData),
		flushInterval: interval,
		storage:       storage,
	}
}

func (s *brokerService) Process(data *models.SflowData) {

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

func (s *brokerService) Start() {
	ticker := time.NewTicker(s.flushInterval)
	for range ticker.C {
		s.flush()
	}
}

func (s *brokerService) flush() {
	s.mu.Lock()

	snapshot := s.items
	s.items = make(map[string]*models.SflowData)
	s.mu.Unlock()

	if len(snapshot) == 0 {
		return
	}

	for _, data := range snapshot {
		err := s.storage.UpdateFlow(data.SrcMAC, data.DstMAC, data.IPv, data.Size)
		if err != nil {
			log.Println("[ERR] Error while updating flow:", err)
			continue
		}
	}
}
