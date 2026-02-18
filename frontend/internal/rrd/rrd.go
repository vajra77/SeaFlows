package rrd

import (
	"errors"
	"log"
)

type Manager struct {
	root  string
	gamma float64
}

func NewManager(root string, gamma float64) *Manager {
	return &Manager{
		root:  root,
		gamma: gamma,
	}
}

func (m *Manager) GetSingleFlow(srcMAC string, dstMAC string, proto int, schedule string) (*Data, error) {

	path := m.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".rrd"

	data := NewData(m.gamma, proto, schedule, path)
	if data == nil {
		return nil, errors.New("unable to create new data from file")
	}

	return data, nil
}

func (m *Manager) GetMultipleFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*Data, error) {

	result := NewData(m.gamma, proto, schedule, "")

	for _, srcMAC := range srcMACs {
		for _, dstMAC := range dstMACs {
			path := m.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".rrd"
			err := result.AddFromFile(path)
			if err != nil {
				log.Printf("[W] unable to add new data from file: %s", path)
				continue
			}
		}
	}
	return result, nil
}
