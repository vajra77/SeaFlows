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
	Avg      [][]interface{} `json:"avg"`
	Max      [][]interface{} `json:"max"`
}

func NewRRDData(gamma float64, proto int, schedule string, path string) (*RRDData, error) {

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

	if path != "" {
		err := data.AddFromFile(path)
		if err != nil {
			return nil, err
		}
	}

	return &data, nil
}

func (d *RRDData) Add(other *RRDData) error {

	if d.Length == 0 {
		d.Avg = make([][]interface{}, other.Length)
		d.Max = make([][]interface{}, other.Length)
		for i := 0; i < other.Length; i++ {
			d.Avg[i] = []interface{}{other.Avg[i][0], other.Avg[i][1]}
			d.Max[i] = []interface{}{other.Max[i][0], other.Max[i][1]}
		}
		d.Length = other.Length
		return nil
	}

	if d.Length != other.Length {
		return errors.New("RRDData.Add: length does not match other.Length")
	}

	for i := 0; i < d.Length; i++ {

		if max2, ok2 := other.Max[i][1].(float64); ok2 {
			if max1, ok1 := d.Max[i][1].(float64); ok1 {
				d.Max[i][1] = max1 + max2
			} else {
				d.Max[i][1] = max2
			}
		}

		if avg2, ok2 := other.Avg[i][1].(float64); ok2 {
			if avg1, ok1 := d.Avg[i][1].(float64); ok1 {
				d.Avg[i][1] = avg1 + avg2
			} else {
				d.Avg[i][1] = avg2
			}
		}
	}
	return nil
}

func (d *RRDData) AddFromFile(path string) error {

	if path == "" {
		return errors.New("RRDData.AddFromFile: empty path")
	}

	avgData, err := RRDTool.Fetch(path, "AVERAGE", d.Start, d.End, d.Step)
	if err != nil {
		return errors.New("RRDData.AddFromFile: " + err.Error())
	}

	maxData, err := RRDTool.Fetch(path, "MAX", d.Start, d.End, d.Step)
	if err != nil {
		return errors.New("RRDData.AddFromFile: " + err.Error())
	}

	avgValues := avgData.Values()
	maxValues := maxData.Values()

	dsIndex := 0
	if d.Proto == 6 {
		dsIndex = 1
	}

	numIntervals := len(avgValues) / 2

	if d.Length == 0 {
		d.Avg = make([][]interface{}, numIntervals)
		d.Max = make([][]interface{}, numIntervals)
		d.Length = numIntervals
	} else if numIntervals != d.Length {
		return errors.New("RRDData.AddFromFile: data length does not match")
	}

	for i := 0; i < d.Length; i++ {
		tsJS := d.Start.Add(time.Duration(i)*d.Step).Unix() * 1000

		avg2 := avgValues[i*2+dsIndex]
		if math.IsNaN(avg2) {
			avg2 = 0.0
		}

		valToAddAvg := avg2 * 8 * d.Gamma

		var avg1 float64
		var okAvg bool
		if d.Avg[i] != nil && len(d.Avg[i]) == 2 {
			avg1, okAvg = d.Avg[i][1].(float64)
		}

		avgPoint := make([]interface{}, 2)
		avgPoint[0] = tsJS
		if okAvg {
			avgPoint[1] = avg1 + valToAddAvg
		} else {
			avgPoint[1] = valToAddAvg
		}
		d.Avg[i] = avgPoint

		max2 := maxValues[i*2+dsIndex]
		if math.IsNaN(max2) {
			max2 = 0.0
		}
		valToAddMax := max2 * 8 * d.Gamma

		var max1 float64
		var okMax bool
		if d.Max[i] != nil && len(d.Max[i]) == 2 {
			max1, okMax = d.Max[i][1].(float64)
		}

		maxPoint := make([]interface{}, 2)
		maxPoint[0] = tsJS
		if okMax {
			maxPoint[1] = max1 + valToAddMax
		} else {
			maxPoint[1] = valToAddMax
		}
		d.Max[i] = maxPoint
	}
	return nil
}
