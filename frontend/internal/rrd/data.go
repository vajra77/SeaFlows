package rrd

import (
	"errors"
	"time"

	RRDTool "github.com/ziutek/rrd"
)

type Data struct {
	Gamma      float64       `json:"gamma"`
	Proto      int           `json:"proto"`
	Schedule   string        `json:"schedule"`
	Start      time.Time     `json:"start"`
	End        time.Time     `json:"end"`
	Step       time.Duration `json:"step"`
	Length     int           `json:"length"`
	Values     []float64     `json:"values"`
	Timestamps []time.Time   `json:"timestamps"`
}

func NewData(gamma float64, proto int, schedule string, path string) *Data {

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

	data := Data{
		Gamma:      gamma,
		Proto:      proto,
		Schedule:   schedule,
		Start:      start,
		End:        end,
		Step:       stepDuration,
		Length:     0,
		Values:     make([]float64, 0),
		Timestamps: make([]time.Time, 0),
	}

	if path != "" {
		rrdData, err := RRDTool.Fetch(path, "AVG", data.Start, data.End, data.Step)
		if err != nil {
			return nil
		}

		allValues := rrdData.Values()

		dsIndex := 0
		if data.Proto == 6 {
			dsIndex = 1
		}

		numIntervals := len(allValues) / 2

		for i := 0; i < numIntervals; i++ {
			val := allValues[i*2+dsIndex]
			data.Values[i] = val * 8 * data.Gamma
			data.Timestamps[i] = data.Start.Add(time.Duration(i) * data.Step)
		}
		data.Length = numIntervals
	}

	return &data
}

func (d *Data) AddFromFile(path string) error {

	if path != "" {
		rrdData, err := RRDTool.Fetch(path, "AVG", d.Start, d.End, d.Step)
		if err != nil {
			return nil
		}

		allValues := rrdData.Values()

		dsIndex := 0
		if d.Proto == 6 {
			dsIndex = 1
		}

		numIntervals := len(allValues) / 2

		if numIntervals != d.Length {
			return errors.New("RRD length does not match")
		}

		for i := 0; i < numIntervals; i++ {
			val := allValues[i*2+dsIndex]
			d.Values[i] += val * 8 * d.Gamma
		}
	}
	return nil
}
