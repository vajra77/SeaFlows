package services

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"seaflows/internal/models"
	"sync"
)

type RRDService interface {
	GetSingleFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error)
	GetMultipleFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error)
}

type rrdService struct {
	basePath string
	step     string
	gamma    float64
	mu       sync.Mutex
}

func NewRRDService(path, step string, gamma float64) StorageService {
	return &rrdService{
		basePath: path,
		step:     step,
		gamma:    gamma,
	}
}

func (s *rrdService) GetFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error) {

	path := filepath.Join(s.basePath, "flows", srcMAC, fmt.Sprintf("flow_%s_to_%s.rrd", srcMAC, dstMAC))

	data, err := models.NewRRDData(s.gamma, proto, schedule, path)
	if err != nil {
		return nil, err
	}

	return data, nil
}

func (s *rrdService) GetFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error) {

	dests := make([]*models.RRDData, len(srcMACs))

	wg := new(sync.WaitGroup)
	errChan := make(chan error, len(srcMACs))

	for i, srcMAC := range srcMACs {
		data, err := models.NewRRDData(s.gamma, proto, schedule, "")
		if err != nil {
			return nil, err
		}
		dests[i] = data
		wg.Add(1)
		go func(idx int, mac string) {
			defer wg.Done()
			if err := s.addRRDFiles(dests[idx], mac, dstMACs); err != nil {
				errChan <- fmt.Errorf("error on MAC %s: %w", mac, err)
			}
		}(i, srcMAC)
	}

	wg.Wait()
	close(errChan)

	for err := range errChan {
		if err != nil {
			return nil, err
		}
	}

	result, err := models.NewRRDData(s.gamma, proto, schedule, "")
	if err != nil {
		return nil, err
	}

	for _, d := range dests {
		if d == nil {
			continue
		}
		if err := result.Add(d); err != nil {
			log.Printf("[WARN] Failed to merge data: %v", err)
		}
	}
	return result, nil
}

func (s *rrdService) UpdateFlow(srcMac, dstMac string, proto int, bytes uint32) error {

	dir := filepath.Join(s.basePath, srcMac)
	fileName := fmt.Sprintf("flow_%s_to_%s.rrd", srcMac, dstMac)
	fullPath := filepath.Join(dir, fileName)

	if _, err := os.Stat(fullPath); os.IsNotExist(err) {
		s.mu.Lock()
		if _, err := os.Stat(fullPath); os.IsNotExist(err) {
			err := s.createRRDFile(dir, fullPath)
			if err != nil {
				log.Printf("[WARN] Failed to create RRD file: %v", err)
			}
		}
		s.mu.Unlock()
	}

	var updateArgs string

	if proto == 4 {
		updateArgs = fmt.Sprintf("N:%d:0", bytes)
	} else {
		updateArgs = fmt.Sprintf("N:0:%d", bytes)
	}

	cmd := exec.Command("rrdtool", "update", fullPath, updateArgs)
	return cmd.Run()
}

func (s *rrdService) createRRDFile(dir, path string) error {

	err := os.MkdirAll(dir, 0755)
	if err != nil {
		return err
	}

	args := []string{
		"create", path,
		"--step", s.step,
		"DS:bytes4:GAUGE:600:U:U",
		"DS:bytes6:GAUGE:600:U:U",
		"RRA:AVERAGE:0.5:1:600",
		"RRA:AVERAGE:0.5:6:700",
		"RRA:AVERAGE:0.5:24:775",
		"RRA:AVERAGE:0.5:288:797",
		"RRA:MAX:0.5:1:600",
		"RRA:MAX:0.5:6:700",
		"RRA:MAX:0.5:24:775",
		"RRA:MAX:0.5:444:797",
	}
	return exec.Command("rrdtool", args...).Run()
}

func (s *rrdService) addRRDFiles(result *models.RRDData, srcMAC string, dstMACs []string) error {

	for _, dstMAC := range dstMACs {
		path := filepath.Join(s.basePath, "flows", srcMAC, fmt.Sprintf("flow_%s_to_%s.rrd", srcMAC, dstMAC))
		err := result.AddFromFile(path)
		if err != nil {
			return err
		}
	}
	return nil
}
