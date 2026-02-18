package rrd

import (
	"errors"
	"time"

	RRDTool "github.com/ziutek/rrd"
)

type ResultData struct {
	Schedule   string      `json:"schedule"`
	Proto      int         `json:"proto"`
	Gamma      float64     `json:"gamma"`
	Values     []float64   `json:"values"`
	Timestamps []time.Time `json:"timestamps"`
}

func NewResultData(schedule string, proto int, gamma float64) *ResultData {
	return &ResultData{
		Schedule:   schedule,
		Proto:      proto,
		Gamma:      gamma,
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

	var start time.Time
	var stepDuration time.Duration

	const D = 300
	const W = 1800
	const M = 7200
	const Y = 86400

	now := time.Now()

	switch r.Schedule {
	case "weekly", "week", "w":
		start = now.AddDate(0, 0, -7) // last week
		stepDuration = W * time.Second
	case "monthly", "month", "m":
		start = now.AddDate(0, -1, 0) // last month
		stepDuration = M * time.Second
	case "yearly", "year", "y":
		start = now.AddDate(-1, 0, 0) // last year
		stepDuration = Y * time.Second
	case "daily", "day", "d":
		fallthrough
	default:
		start = now.AddDate(0, 0, -1) // last 24 hours
		stepDuration = D * time.Second
	}

	rrdData, err := RRDTool.Fetch(path, "AVG", start, now, stepDuration)
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
		r.Timestamps[i] = start.Add(time.Duration(i) * stepDuration)
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
