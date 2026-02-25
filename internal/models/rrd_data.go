package models

import (
	"errors"
	"math"
	"time"

	RRDTool "github.com/ziutek/rrd"
)

const (
	RRDStep          = 300
	RRDFlushInterval = 60
)

type Result [][]float64

type RRDData struct {
	Gamma    float64         `json:"gamma"`
	Proto    int             `json:"proto"`
	Schedule string          `json:"schedule"`
	Start    time.Time       `json:"start"`
	End      time.Time       `json:"end"`
	Step     time.Duration   `json:"step"`
	Length   int             `json:"length"`
	AvgIn    [][]interface{} `json:"avg_in"`
	AvgOut   [][]interface{} `json:"avg_out"`
	MaxIn    [][]interface{} `json:"max_in"`
	MaxOut   [][]interface{} `json:"max_out"`
}

func NewRRDData(gamma float64, proto int, schedule string, pathOut, pathIn string) (*RRDData, error) {

	const D = 300
	const W = 1800
	const M = 7200
	const Y = 86400

	var start time.Time
	var stepDuration time.Duration

	end := time.Now()

	switch schedule {
	case "weekly", "week", "w":
		start = end.AddDate(0, 0, -7) // last week
		stepDuration = W * time.Second
	case "monthly", "month", "m":
		start = end.AddDate(0, -1, 0) // last month
		stepDuration = M * time.Second
	case "yearly", "year", "y":
		start = end.AddDate(-1, 0, 0) // last year
		stepDuration = Y * time.Second
	case "daily", "day", "d":
		fallthrough
	default:
		start = end.AddDate(0, 0, -1) // last 24 hours
		stepDuration = D * time.Second
	}

	data := RRDData{
		Gamma:    gamma,
		Proto:    proto,
		Schedule: schedule,
		Start:    start,
		End:      end,
		Step:     stepDuration,
		Length:   0,
	}

	if pathIn != "" && pathOut != "" {
		err := data.AddBiDirFiles(pathOut, pathIn)
		if err != nil {
			return nil, err
		}
	}
	return &data, nil
}

func (d *RRDData) Add(other *RRDData) error {
	if other == nil || other.Length == 0 {
		return nil
	}

	if d.Length == 0 {
		d.Length = other.Length
		d.AvgOut = cloneArray(other.AvgOut)
		d.MaxOut = cloneArray(other.MaxOut)
		d.AvgIn = cloneArray(other.AvgIn)
		d.MaxIn = cloneArray(other.MaxIn)
		return nil
	}

	if d.Length != other.Length {
		return errors.New("RRDData.Add: length mismatch")
	}

	for i := 0; i < d.Length; i++ {
		d.AvgOut[i][1] = d.AvgOut[i][1].(float64) + other.AvgOut[i][1].(float64)
		d.MaxOut[i][1] = d.MaxOut[i][1].(float64) + other.MaxOut[i][1].(float64)
		d.AvgIn[i][1] = d.AvgIn[i][1].(float64) + other.AvgIn[i][1].(float64)
		d.MaxIn[i][1] = d.MaxIn[i][1].(float64) + other.MaxIn[i][1].(float64)
	}
	return nil
}

func cloneArray(src [][]interface{}) [][]interface{} {
	dst := make([][]interface{}, len(src))
	for i := range src {
		dst[i] = []interface{}{src[i][0], src[i][1]}
	}
	return dst
}

func (d *RRDData) AddBiDirFiles(pathOut, pathIn string) error {

	var avgOut, maxOut, avgIn, maxIn RRDTool.FetchResult
	var hasOut, hasIn bool

	// check/fetch path out (src->dst)
	var errOut error
	avgOut, errOut = RRDTool.Fetch(pathOut, "AVERAGE", d.Start, d.End, d.Step)
	if errOut == nil {
		maxOut, errOut = RRDTool.Fetch(pathOut, "MAX", d.Start, d.End, d.Step)
		if errOut == nil {
			hasOut = true
		}
	}

	// check/fetch path in (src<-dst)
	var errIn error
	avgIn, errIn = RRDTool.Fetch(pathIn, "AVERAGE", d.Start, d.End, d.Step)
	if errIn == nil {
		maxIn, errIn = RRDTool.Fetch(pathIn, "MAX", d.Start, d.End, d.Step)
		if errIn == nil {
			hasIn = true
		}
	}

	// determine number of samples
	numIntervals := 0
	if hasOut {
		numIntervals = len(avgOut.Values()) / 2
	} else if hasIn {
		numIntervals = len(avgIn.Values()) / 2
	} else {
		// no file found
		return nil
	}

	// init data slices
	if d.Length == 0 {
		d.Length = numIntervals
		d.AvgOut = make([][]interface{}, numIntervals)
		d.MaxOut = make([][]interface{}, numIntervals)
		d.AvgIn = make([][]interface{}, numIntervals)
		d.MaxIn = make([][]interface{}, numIntervals)

		for i := 0; i < numIntervals; i++ {
			tsJS := d.Start.Add(time.Duration(i)*d.Step).Unix() * 1000
			d.AvgOut[i] = []interface{}{tsJS, 0.0}
			d.MaxOut[i] = []interface{}{tsJS, 0.0}
			d.AvgIn[i] = []interface{}{tsJS, 0.0}
			d.MaxIn[i] = []interface{}{tsJS, 0.0}
		}
	} else if numIntervals != d.Length {
		return errors.New("AddBiDirFiles: data length mismatch")
	}

	dsIndex := 0
	if d.Proto == 6 {
		dsIndex = 1
	}

	// fill-in data
	for i := 0; i < d.Length; i++ {
		if hasOut {
			v := avgOut.Values()[i*2+dsIndex]
			if !math.IsNaN(v) {
				d.AvgOut[i][1] = d.AvgOut[i][1].(float64) + (v * 8 * d.Gamma)
			}
			vMax := maxOut.Values()[i*2+dsIndex]
			if !math.IsNaN(vMax) {
				d.MaxOut[i][1] = d.MaxOut[i][1].(float64) + (vMax * 8 * d.Gamma)
			}
		}

		if hasIn {
			v := avgIn.Values()[i*2+dsIndex]
			if !math.IsNaN(v) {
				d.AvgIn[i][1] = d.AvgIn[i][1].(float64) + (v * 8 * d.Gamma)
			}
			vMax := maxIn.Values()[i*2+dsIndex]
			if !math.IsNaN(vMax) {
				d.MaxIn[i][1] = d.MaxIn[i][1].(float64) + (vMax * 8 * d.Gamma)
			}
		}
	}
	return nil
}
