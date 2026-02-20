package services

import (
	"errors"
	"fmt"
	"log"
	"seaflows/internal/models"
	"sync"
)

type RRDService interface {
	GetSingleFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error)
	GetMultipleFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error)
}

type rrdService struct {
	root  string
	gamma float64
}

func NewRRDService(root string, gamma float64) RRDService {
	return &rrdService{
		root:  root,
		gamma: gamma,
	}
}

func (s *rrdService) GetSingleFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error) {

	path := s.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".rrd"

	data := models.NewRRDData(s.gamma, proto, schedule, path)
	if data == nil {
		return nil, errors.New("unable to create new data from file")
	}

	return data, nil
}

func (s *rrdService) GetMultipleFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error) {

	dests := make([]*models.RRDData, len(srcMACs))

	wg := new(sync.WaitGroup)
	errChan := make(chan error, len(srcMACs))

	for i, srcMAC := range srcMACs {
		wg.Add(1)
		dests[i] = models.NewRRDData(s.gamma, proto, schedule, "")
		go func(idx int, mac string) {
			defer wg.Done()
			if err := s.addDestinations(dests[idx], mac, dstMACs); err != nil {
				errChan <- fmt.Errorf("error on MAC %s: %w", mac, err)
			}
		}(i, srcMAC)
	}

	wg.Wait()
	close(errChan)

	for err := range errChan {
		if err != nil {
			return nil, err
		}
	}

	result := models.NewRRDData(s.gamma, proto, schedule, "")
	for _, d := range dests {
		if d == nil {
			continue
		}
		if err := result.Add(d); err != nil {
			log.Printf("[WARN] Failed to merge data: %v", err)
		}
	}
	return result, nil
}

func (s *rrdService) addDestinations(result *models.RRDData, srcMAC string, dstMACs []string) error {

	for _, dstMAC := range dstMACs {
		path := s.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".rrd"
		err := result.AddFromFile(path)
		if err != nil {
			return err
		}
	}
	return nil
}
