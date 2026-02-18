package services

import (
	"errors"
	"log"
	"seaflows/internal/models"
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

	path := s.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".services"

	data := models.NewData(s.gamma, proto, schedule, path)
	if data == nil {
		return nil, errors.New("unable to create new data from file")
	}

	return data, nil
}

func (s *rrdService) GetMultipleFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error) {

	result := models.NewData(s.gamma, proto, schedule, "")

	for _, srcMAC := range srcMACs {
		for _, dstMAC := range dstMACs {
			path := s.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".services"
			err := result.AddFromFile(path)
			if err != nil {
				log.Printf("[W] unable to add new data from file: %s", path)
				continue
			}
		}
	}
	return result, nil
}
