package services

import "seaflows/internal/models"

type FlowProcessorService interface {
	Process(data *models.SflowData)
	Start()
	Stop()
}

type StorageService interface {
	UpdateFlows(flows map[string]*models.AggregatedFlow) error
	GetFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error)
	GetFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error)
}

type AddressMapperService interface {
	GetMACsFromAS(asn string) []string
	GetMACsFromASSet(asList []string) []string
	GetASNs() []string
}
