package services

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"seaflows/internal/models"
	"strings"
	"sync"
	"time"
)

type RRDService interface {
	GetSingleFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error)
	GetMultipleFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error)
}

type rrdService struct {
	basePath   string
	socketPath string
	step       int
	gamma      float64
	mu         sync.Mutex
}

func NewRRDService(bPath, sPath string, step int, gamma float64) StorageService {
	return &rrdService{
		basePath:   bPath,
		socketPath: sPath,
		step:       step,
		gamma:      gamma,
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

func (s *rrdService) UpdateFlow(srcMac, dstMac string, ipv int, bytes uint32) error {

	dir := filepath.Join(s.basePath, "flows", srcMac)
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

	return s.sendToDaemon(fullPath, ipv, bytes)
}

func (s *rrdService) sendToDaemon(fullPath string, ipv int, bytes uint32) error {

	conn, err := net.Dial("unix", s.socketPath)
	if err != nil {
		return fmt.Errorf("unable to connect to rrdcached: %w", err)
	}
	defer func(conn net.Conn) {
		err := conn.Close()
		if err != nil {
			log.Printf("[WARN] Failed to close connection to rrdcached: %v", err)
		}
	}(conn)

	now := time.Now().Unix()

	var command string
	if ipv == 4 {
		command = fmt.Sprintf("UPDATE %s %d:%d:0\n", fullPath, now, bytes)
	} else {
		command = fmt.Sprintf("UPDATE %s %d:0:%d\n", fullPath, now, bytes)
	}

	if _, err = conn.Write([]byte(command)); err != nil {
		return fmt.Errorf("error while writing to socket: %w", err)
	}

	reader := bufio.NewReader(conn)
	response, err := reader.ReadString('\n')
	if err != nil {
		return fmt.Errorf("error while reading response from rrdcached: %w", err)
	}

	if strings.HasPrefix(response, "-1") {
		return fmt.Errorf("rrdcached returned an error: %s", strings.TrimSpace(response))
	}

	return nil
}

func (s *rrdService) createRRDFile(dir, path string) error {

	err := os.MkdirAll(dir, 0755)
	if err != nil {
		return err
	}

	args := []string{
		"create", path,
		"--step", fmt.Sprintf("%d", s.step),
		"DS:bytes4:ABSOLUTE:600:0:U",
		"DS:bytes6:ABSOLUTE:600:0:U",
		"RRA:AVERAGE:0.5:1:600",
		"RRA:AVERAGE:0.5:6:700",
		"RRA:AVERAGE:0.5:24:775",
		"RRA:AVERAGE:0.5:288:797",
		"RRA:MAX:0.5:1:600",
		"RRA:MAX:0.5:6:700",
		"RRA:MAX:0.5:24:775",
		"RRA:MAX:0.5:288:797",
	}
	cmd := exec.Command("rrdtool", args...)

	if output, err := cmd.CombinedOutput(); err != nil {
		return fmt.Errorf("error in rrdtool create: %s (details: %s)", err, string(output))
	}

	return nil
}

func (s *rrdService) addRRDFiles(result *models.RRDData, srcMAC string, dstMACs []string) error {

	for _, dstMAC := range dstMACs {
		path := filepath.Join(s.basePath, "flows", srcMAC, fmt.Sprintf("flow_%s_to_%s.rrd", srcMAC, dstMAC))
		err := result.AddFromFile(path)
		if err != nil {
			log.Printf("[WARN] Failed to add RRD file: %v", err)
			continue
		}
	}
	return nil
}
