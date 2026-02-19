package helpers

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"seaflows/internal/models"
	"time"
)

type IXFExport struct {
	MemberList []IXFMember `json:"member_list"`
}

type IXFMember struct {
	Asn        int       `json:"asnumber"`
	Connection []IXFConn `json:"connection_list"`
}

type IXFConn struct {
	VlanList []IXFVlan `json:"vlan_list"`
}

type IXFVlan struct {
	IPv4 *IXFAddr `json:"ipv4"`
	IPv6 *IXFAddr `json:"ipv6"`
}

type IXFAddr struct {
	Address string `json:"address"`
	MAC     string `json:"mac_address"`
}

func PopulateFromIXF(url string) (*models.MapData, error) {
	client := &http.Client{Timeout: 10 * time.Second}

	resp, err := client.Get(url)
	if err != nil {
		return nil, err
	}
	defer func(Body io.ReadCloser) {
		err := Body.Close()
		if err != nil {

		}
	}(resp.Body)

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("errore API: %s", resp.Status)
	}

	var export IXFExport
	if err := json.NewDecoder(resp.Body).Decode(&export); err != nil {
		return nil, err
	}

	mapData := models.NewMapData()

	for _, member := range export.MemberList {
		asnStr := fmt.Sprintf("%d", member.Asn)

		for _, conn := range member.Connection {
			for _, vlan := range conn.VlanList {
				ipv4 := ""
				ipv6 := ""
				mac := ""

				if vlan.IPv4 != nil {
					ipv4 = vlan.IPv4.Address
					mac = vlan.IPv4.MAC
				}
				if vlan.IPv6 != nil {
					ipv6 = vlan.IPv6.Address
					if mac == "" {
						mac = vlan.IPv6.MAC
					}
				}

				if ipv4 != "" || ipv6 != "" {
					mapData.AddAddressMap(asnStr, ipv4, ipv6, mac)
				}
			}
		}
	}

	return mapData, nil
}
