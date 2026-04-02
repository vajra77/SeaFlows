package models

import (
	"sort"
	"strings"
)

type AddressMap struct {
	IPv4Address string `json:"ipv4_address"`
	IPv6Address string `json:"ipv6_address"`
	MACAddress  string `json:"mac_address"`
}

type ASNData struct {
	ASN  string `json:"asn"`
	Name string `json:"name"`
}

type MapData struct {
	Maps  map[string][]*AddressMap `json:"maps"`
	Names map[string]string        `json:"names"`
}

func NewMapData() *MapData {
	return &MapData{
		Maps:  make(map[string][]*AddressMap),
		Names: make(map[string]string),
	}
}

func (m *MapData) AddName(asn, name string) {
	m.Names[asn] = name
}

func (m *MapData) AddAddressMap(asn, ipv4, ipv6, mac string) {

	if mac == "" {
		return // Ignoriamo entry senza MAC
	}

	cleanMAC := strings.ToLower(strings.ReplaceAll(mac, ":", ""))

	for _, existing := range m.Maps[asn] {
		if existing.MACAddress == cleanMAC {
			return
		}
	}

	m.Maps[asn] = append(m.Maps[asn],
		&AddressMap{
			IPv4Address: ipv4,
			IPv6Address: ipv6,
			MACAddress:  cleanMAC,
		})
}

func (m *MapData) GetAllMACs(asn string) []string {

	data := make([]string, len(m.Maps[asn]))

	_, found := m.Maps[asn]
	if found {
		for i := range m.Maps[asn] {
			data[i] = m.Maps[asn][i].MACAddress
		}
	}
	return data
}

func (m *MapData) GetAllASNs() []ASNData {

	data := make([]ASNData, 0)

	for k := range m.Maps {
		data = append(data, ASNData{ASN: k, Name: m.Names[k]})
	}
	sort.Slice(data, func(i, j int) bool {
		return data[i].Name <= data[j].Name
	})
	return data
}
