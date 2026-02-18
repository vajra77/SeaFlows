package exporter

import (
	"fmt"
	"math"
	"os"
	"strings"
	"time"

	RRDTool "github.com/ziutek/rrd"
)

type RRD struct {
	rootDir string
	gamma   float64
}

type FlowResponse struct {
	Source      string    `json:"src"`
	Destination string    `json:"dst"`
	Proto       string    `json:"proto"`
	Schedule    string    `json:"schedule"`
	Values      []float64 `json:"values"`
}

func NewRRDExporter(rootDir string, gamma float64) *RRDExporter {
	return &RRDExporter{
		rootDir: rootDir,
		gamma:   gamma,
	}
}

func (e *RRDExporter) GetFlowDataByMAC(srcMac string, dstMac string, proto int, schedule string) ([]float64, error) {
	rrdFile := e.rootDir + "/flows/" + srcMac + "/" + "flow_" + srcMac + "_to_" + dstMac + ".rrd"

	var start time.Time
	var stepDuration time.Duration
	var resultValues []float64

	now := time.Now()

	switch schedule {
	case "weekly", "week", "w":
		start = now.AddDate(0, 0, -7) // last week
		stepDuration = 1800 * time.Second
	case "monthly", "month", "m":
		start = now.AddDate(0, -1, 0) // last month
		stepDuration = 7200 * time.Second
	case "yearly", "year", "y":
		start = now.AddDate(-1, 0, 0) // last year
		stepDuration = 86400 * time.Second
	case "daily", "day", "d":
		fallthrough
	default:
		start = now.AddDate(0, 0, -1) // last 24 hours
		stepDuration = 300 * time.Second
	}

	rrdData, err := RRDTool.Fetch(rrdFile, "AVG", start, now, stepDuration)
	if err != nil {
		return nil, err
	}

	allValues := rrdData.Values()

	dsIndex := 0
	if proto == 6 {
		dsIndex = 1
	}

	numIntervals := len(allValues) / 2
	resultValues = make([]float64, numIntervals)

	for i := 0; i < numIntervals; i++ {
		// Calcola l'indice corretto nell'array piatto
		val := allValues[i*2+dsIndex]
		resultValues[i] = e.gamma * val * 8
	}

	return resultValues, nil
}

func (e *RRDExporter) GetPeerDataByMAC(peerMac string, proto int, schedule string) ([]float64, error) {
	peerDir := e.rootDir + "/flows/" + peerMac

	// read all files in directory
	files, err := os.ReadDir(peerDir)
	if err != nil {
		return nil, fmt.Errorf("unable to read peer directory: %v", err)
	}

	var resultValues []float64
	var start time.Time
	var stepDuration time.Duration

	now := time.Now()

	switch schedule {
	case "weekly", "week", "w":
		start = now.AddDate(0, 0, -7)
		stepDuration = 1800 * time.Second
	case "monthly", "month", "m":
		start = now.AddDate(0, -1, 0)
		stepDuration = 7200 * time.Second
	case "yearly", "year", "y":
		start = now.AddDate(-1, 0, 0)
		stepDuration = 86400 * time.Second
	default:
		start = now.AddDate(0, 0, -1)
		stepDuration = 300 * time.Second
	}

	for _, file := range files {
		if !file.IsDir() && strings.HasSuffix(file.Name(), ".rrd") {
			rrdPath := peerDir + "/" + file.Name()

			rrdData, err := RRDTool.Fetch(rrdPath, "AVG", start, now, stepDuration)
			if err != nil {
				continue // skip corrupted or errored files
			}

			allValues := rrdData.Values()

			dsIndex := 0
			if proto == 6 {
				dsIndex = 1
			}

			numIntervals := len(allValues) / 2
			if resultValues == nil {
				resultValues = make([]float64, numIntervals)
			}

			for i := 0; i < numIntervals; i++ {
				// Calcola l'indice corretto nell'array piatto
				val := allValues[i*2+dsIndex]

				if !math.IsNaN(val) {
					resultValues[i] += e.gamma * val * 8
				}
			}
		}
	}

	if resultValues == nil {
		return nil, fmt.Errorf("no data for peer %s", peerMac)
	}

	return resultValues, nil
}
