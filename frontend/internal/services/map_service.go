package services

import (
	"log"
	"seaflows/internal/helpers"
	"seaflows/internal/models"
	"sync"
	"time"
)

type MapService interface {
	GetMACs(asn string) []string
}

type mapService struct {
	url  string
	data *models.MapData
	mu   sync.RWMutex
}

func NewMapService(url string) (MapService, error) {

	initialData, err := helpers.PopulateFromIXF(url)
	if err != nil {
		return nil, err
	}

	srv := &mapService{
		url:  url,
		data: initialData,
	}

	go srv.startPeriodicRefresh(3 * time.Hour)

	return srv, nil
}

func (s *mapService) GetMACs(asn string) []string {

	s.mu.RLock()
	defer s.mu.RUnlock()

	return s.data.GetAllMACs(asn)
}

func (s *mapService) GetASNs() []string {

	s.mu.RLock()
	defer s.mu.RUnlock()

	return s.data.GetAllASNs()
}

func (s *mapService) startPeriodicRefresh(interval time.Duration) {

	ticker := time.NewTicker(interval)
	defer ticker.Stop()

	for range ticker.C {
		newData, err := helpers.PopulateFromIXF(s.url)
		if err != nil {
			log.Printf("[CRIT] MapService: failed to refresh IXF data: %v", err)
			continue
		}

		s.mu.Lock()
		s.data = newData
		s.mu.Unlock()

		log.Printf("[INFO] MapService: IXF data successfully refreshed")
	}
}
