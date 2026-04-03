package services

import (
	"encoding/json"
	"log"
	"os"
	"seaflows/internal/models"
	"syscall"
)

type MonitorService struct {
	pipePath string
	dataChan chan models.MonitorRecord
}

func NewMonitorService(path string) *MonitorService {

	if path == "" {
		return nil
	}

	_ = os.Remove(path)
	err := syscall.Mkfifo(path, 0666)
	if err != nil {
		log.Printf("[WARN] Failed to create monitor pipe: %v", err)
		return nil
	}
	_ = os.Chmod(path, 0666)

	ms := &MonitorService{
		pipePath: path,
		dataChan: make(chan models.MonitorRecord, 1000),
	}

	go ms.start()
	log.Printf("[INFO] Monitor service started on pipe: %s", path)
	return ms
}

func (ms *MonitorService) Send(record models.MonitorRecord) {
	if ms == nil {
		return
	}
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
