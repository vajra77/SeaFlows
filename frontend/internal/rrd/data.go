package rrd

import (
	"errors"
	"time"

	RRDTool "github.com/ziutek/rrd"
)

type ResultData struct {
	Gamma      float64       `json:"gamma"`
	Proto      int           `json:"proto"`
	Schedule   string        `json:"schedule"`
	Start      time.Time     `json:"start"`
	End        time.Time     `json:"end"`
	Step       time.Duration `json:"step"`
	Values     []float64     `json:"values"`
	Timestamps []time.Time   `json:"timestamps"`
}

func NewResultData(gamma float64, proto int, schedule string) *ResultData {

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
	return &ResultData{
		Gamma:      gamma,
		Proto:      proto,
		Start:      start,
		End:        end,
		Step:       stepDuration,
		Schedule:   schedule,
		Values:     make([]float64, 0),
		Timestamps: make([]time.Time, 0),
	}
}

func (r *ResultData) CanBeAddedTo(or *ResultData) bool {
	result := r.Schedule == or.Schedule && r.Proto == or.Proto && r.Gamma == or.Gamma
	result = result && len(r.Values) == len(or.Values) && len(r.Timestamps) == len(or.Timestamps)
	return result
}

func (r *ResultData) Fetch(path string) error {

	rrdData, err := RRDTool.Fetch(path, "AVG", r.Start, r.End, r.Step)
	if err != nil {
		return nil
	}

	allValues := rrdData.Values()

	dsIndex := 0
	if r.Proto == 6 {
		dsIndex = 1
	}

	numIntervals := len(allValues) / 2

	for i := 0; i < numIntervals; i++ {
		val := allValues[i*2+dsIndex]
		r.Values[i] = val * 8 * r.Gamma
		r.Timestamps[i] = r.Start.Add(time.Duration(i) * r.Step)
	}

	return nil
}

func (r *ResultData) Add(or *ResultData) error {

	if !r.CanBeAddedTo(or) {
		return errors.New("data sets cannot be added")
	}

	for i, _ := range r.Values {
		// THIS MIGHT BE TOO RESTRICTIVE!
		//if r.Timestamps[i] != or.Timestamps[i] {
		//	return errors.New("data sets cannot be added due to uncompatible timestamps")
		//}
		r.Values[i] += or.Values[i]
	}

	return nil
}
