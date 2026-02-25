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
		avgData, err := RRDTool.Fetch(path, "AVERAGE", data.Start, data.End, data.Step)
		if err != nil {
			return nil, err
		}

		maxData, err := RRDTool.Fetch(path, "MAX", data.Start, data.End, data.Step)
		if err != nil {
			return nil, err
		}

		avgValues := avgData.Values()
		maxValues := maxData.Values()

		dsIndex := 0
		if data.Proto == 6 {
			dsIndex = 1
		}

		numIntervals := len(avgValues) / 2

		for i := 0; i < numIntervals; i++ {
			avgV := avgValues[i*2+dsIndex]
			if math.IsNaN(avgV) {
				avgV = 0.0
			}

			maxV := maxValues[i*2+dsIndex]
			if math.IsNaN(maxV) {
				maxV = 0.0
			}

			avgPoint := make([]interface{}, 2)
			avgPoint[0] = data.Start.Add(time.Duration(i) * data.Step)
			avgPoint[1] = avgV
			data.Avg = append(data.Avg, avgPoint)

			maxPoint := make([]interface{}, 2)
			maxPoint[0] = avgPoint[0]
			maxPoint[1] = maxV
			data.Max = append(data.Max, maxPoint)

		}
		data.Length = numIntervals
	}

	return &data, nil
}

func (d *RRDData) Add(other *RRDData) error {

	if d.Length == 0 {
		// if d is just initialized, copy values
		d.Avg = make([][]interface{}, other.Length)
		d.Max = make([][]interface{}, other.Length)
		copy(d.Avg, other.Avg)
		copy(d.Max, other.Max)
		d.Length = other.Length
	} else {
		if d.Length != other.Length {
			return errors.New("RRDData.Add: length does not match other.Length")
		}

		for i := range d.Length {

			m1 := d.Max[i][1]
			m2 := other.Max[i][1]

			max1, ok1 := m1.(float64)
			max2, ok2 := m2.(float64)

			if ok1 && ok2 {
				d.Max[i][1] = max1 + max2
			} else if ok2 {
				d.Max[i][1] = max2
			} else if ok1 {
				// keep existing value
			} else {
				d.Max[i][1] = 0.0
			}

			a1 := d.Avg[i][1]
			a2 := other.Avg[i][1]

			avg1, ok1 := a1.(float64)
			avg2, ok2 := a2.(float64)

			if ok1 && ok2 {
				d.Avg[i][1] = avg1 + avg2
			} else if ok2 {
				d.Avg[i][1] = avg2
			} else if ok1 {
				// keep existing value
			} else {
				d.Avg[i][1] = 0.0
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

		a1 := d.Avg[i][1]
		avg1, ok1 := a1.(float64)
		avg2 := avgValues[i*2+dsIndex]
		if math.IsNaN(avg2) {
			avg2 = 0.0
		}

		d.Avg[i][0] = d.Start.Add(time.Duration(i) * d.Step)
		if ok1 {
			d.Avg[i][1] = avg1 + (avg2 * 8 * d.Gamma)
		} else {
			d.Avg[i][1] = avg2
		}

		m1 := d.Max[i][1]
		max1, ok1 := m1.(float64)
		max2 := maxValues[i*2+dsIndex]
		if math.IsNaN(max2) {
			max2 = 0.0
		}

		d.Max[i][0] = d.Start.Add(time.Duration(i) * d.Step)
		if ok1 {
			d.Max[i][1] = max1 + (max2 * 8 * d.Gamma)
		} else {
			d.Max[i][1] = max2
		}
	}
	return nil
}
