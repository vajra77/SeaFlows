package services

import (
	"seaflows/internal/helpers"
	"seaflows/internal/models"
)

type MapService interface {
	GetMACs(asn string) []string
}

type mapService struct {
	url  string
	data *models.MapData
}

func NewMapService(url string) (MapService, error) {

	data, err := helpers.PopulateFromIXF(url)
	if err != nil {
		return nil, err
	}

	return &mapService{
		url,
		data,
	}, nil
}

func (s *mapService) GetMACs(asn string) []string {

	return s.data.GetAllMACs(asn)
}
