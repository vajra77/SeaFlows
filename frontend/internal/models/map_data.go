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
	m.Maps[asn] = append(m.Maps[asn],
		&AddressMap{
			ipv4,
			ipv6,
			strings.ToLower(strings.ReplaceAll(mac, ":", "")),
		})
}

func (m *MapData) GetAddressMaps(asn string) []*AddressMap {

	_, found := m.Maps[asn]
	if found {
		return m.Maps[asn]
	} else {
		return nil
	}
}

func (m *MapData) GetAllMACs(asn string) []string {

	_, found := m.Maps[asn]
	if found {
		result := make([]string, len(m.Maps[asn]))
		for i := range m.Maps[asn] {
			result[i] = m.Maps[asn][i].MACAddress
		}
		return result
	}
	return nil
}
