package services

import "seaflows/internal/models"

type FlowProcessorService interface {
	Process(data *models.SflowData)
	Start()
}

type StorageService interface {
	UpdateFlow(srcMac, dstMac string, bytes4 uint32, bytes6 uint32) error
	UpdateFlowsBatch(flows map[string]*models.AggregatedFlowData) error
	GetFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error)
	GetFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error)
}

type AddressMapperService interface {
	GetMACsFromAS(asn string) []string
	GetMACsFromASSet(asList []string) []string
	GetASNs() []string
}
