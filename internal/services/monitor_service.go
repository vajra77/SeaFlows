package services

import (
	"encoding/json"
	"os"
	"seaflows/internal/models"
	"syscall"
)

type MonitorService struct {
	pipePath string
	dataChan chan models.MonitorRecord
}

func NewMonitorService(path string) (*MonitorService, error) {
	// Crea la FIFO se non esiste
	if _, err := os.Stat(path); os.IsNotExist(err) {
		err = syscall.Mkfifo(path, 0666)
		if err != nil {
			return nil, err
		}
	}

	ms := &MonitorService{
		pipePath: path,
		// Buffer capiente per non bloccare mai il chiamante
		dataChan: make(chan models.MonitorRecord, 1000),
	}

	go ms.start()
	return ms, nil
}

func (ms *MonitorService) Send(record models.MonitorRecord) {
	select {
	case ms.dataChan <- record:
	default:
		// if channel is full, drop the record
	}
}

func (ms *MonitorService) start() {
	for record := range ms.dataChan {
		f, err := os.OpenFile(ms.pipePath, os.O_WRONLY|os.O_APPEND, os.ModeNamedPipe)
		if err != nil {
			continue
		}

		jsonData, _ := json.Marshal(record)
		_, _ = f.Write(append(jsonData, '\n'))

		err = f.Close()
		if err != nil {
			return
		}
	}
}
