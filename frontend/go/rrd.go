package seaflows_exporter

import (
	"time"

	rrdtool "github.com/ziutek/rrd"
)

type RRDExporter struct {
	rootDir string
	gamma   float64
}

func NewRRDExporter(rootDir string, gamma float64) *RRDExporter {
	return &RRDExporter{
		rootDir: rootDir,
		gamma:   gamma,
	}
}

func (e *RRDExporter) GetRootDir() string {
	return e.rootDir
}

func (e *RRDExporter) GetGamma() float64 {
	return e.gamma
}

func (e *RRDExporter) GetFlow(schedule string, src string, dst string) []float64 {

	rrdFile := e.rootDir + "/" + src + "/" + "flow_" + src + "_to_" + dst + ".rrd"
	result, err := rrdtool.Fetch(rrdFile, "AVG", time.Now(), time.Now(), 300)
	if err != nil {
		return result.Values()
	}
	return []float64{}
}
