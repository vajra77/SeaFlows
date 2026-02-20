package models

import "strings"

type AddressMap struct {
	IPv4Address string `json:"ipv4_address"`
	IPv6Address string `json:"ipv6_address"`
	MACAddress  string `json:"mac_address"`
}

type MapData struct {
	Maps map[string][]*AddressMap `json:"maps"`
}

func NewMapData() *MapData {
	return &MapData{
		Maps: make(map[string][]*AddressMap),
	}
}

func (m *MapData) AddAddressMap(asn, ipv4, ipv6, mac string) {

	_, found := m.Maps[asn]

	if !found {
		m.Maps[asn] = append(m.Maps[asn],
			&AddressMap{
				IPv4Address: ipv4,
				IPv6Address: ipv6,
				MACAddress:  strings.ToLower(strings.ReplaceAll(mac, ":", "")),
			})
	}
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

func (m *MapData) GetAllASNs() []string {

	data := make([]string, len(m.Maps))

	for k := range m.Maps {
		data = append(data, k)
	}

	return data
}
