package rrd

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

func (m *Manager) GetSingleFlow(srcMAC string, dstMAC string, schedule string, proto int) (*ResultData, error) {

	result := NewResultData(schedule, proto, m.gamma)

	path := m.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".rrd"

	err := result.Fetch(path)
	if err != nil {
		return nil, err
	}

	return result, nil
}

func (m *Manager) GetMultipleFlows(srcMACs []string, dstMACs []string, schedule string, proto int) (*ResultData, error) {

	result := NewResultData(schedule, proto, m.gamma)

	for _, srcMAC := range srcMACs {
		for _, dstMAC := range dstMACs {
			tempRes := NewResultData(schedule, proto, m.gamma)
			path := m.root + "/flows/" + srcMAC + "/" + "flow_" + srcMAC + "_to_" + dstMAC + ".rrd"
			err := tempRes.Fetch(path)
			if err != nil {
				continue
			}
			err = result.Add(tempRes)
			if err != nil {
				continue
			}
		}
	}
	return result, nil
}
