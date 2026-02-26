package services

import (
	"bufio"
	"bytes"
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

// GetFlow returns aggregate data for a traffic flow between two given hosts, identified by
// source and destination MAC addresses. Data is collected for a scheduled period of time and
// is bound to protocol (IPv4/IPv6)
//
// Returns a new models.RRDData container struct and error
func (s *rrdService) GetFlow(srcMAC string, dstMAC string, proto int, schedule string) (*models.RRDData, error) {

	// build forward and reverse path (src->dst/src<-dst)
	pathOut := filepath.Join(s.basePath, "flows", srcMAC, fmt.Sprintf("flow_%s_to_%s.rrd", srcMAC, dstMAC))
	pathIn := filepath.Join(s.basePath, "flows", dstMAC, fmt.Sprintf("flow_%s_to_%s.rrd", dstMAC, srcMAC))

	// create a new RRDData container with data sourced from pathOut,pathIn RRD files
	data, err := models.NewRRDData(s.gamma, proto, schedule, pathOut, pathIn)
	if err != nil {
		return nil, err
	}

	return data, nil
}

// GetFlows returns aggregate data for an aggregate flow between two sets of hosts, identified by
// their MAC addresses. Schedule and proto as in GetFlow
//
// Returns a new models.RRDData container struct and error
func (s *rrdService) GetFlows(srcMACs []string, dstMACs []string, proto int, schedule string) (*models.RRDData, error) {

	// prepare a slice as a buffer of destination
	dests := make([]*models.RRDData, len(srcMACs))

	// init sync mechanisms
	wg := new(sync.WaitGroup)
	errChan := make(chan error, len(srcMACs))

	// loop through src MACs
	for i, srcMAC := range srcMACs {
		// create an empty RRDData container
		data, err := models.NewRRDData(s.gamma, proto, schedule, "", "")
		if err != nil {
			return nil, err
		}

		// put container in buffer
		dests[i] = data
		wg.Add(1)

		// concurrent loop through destinations
		go func(idx int, mac string) {
			defer wg.Done()
			if err := s.addRRDFiles(dests[idx], mac, dstMACs); err != nil {
				errChan <- fmt.Errorf("error on MAC %s: %w", mac, err)
			}
		}(i, srcMAC)
	}

	// wait for all goroutines to end
	wg.Wait()
	close(errChan)

	// check for any given error in the errChan
	for err := range errChan {
		if err != nil {
			return nil, err
		}
	}

	// create a new empty RRDData result container
	result, err := models.NewRRDData(s.gamma, proto, schedule, "", "")
	if err != nil {
		return nil, err
	}

	// loop through pre-prepared buffers
	for _, d := range dests {
		if d == nil {
			continue
		}
		// merge data from given buffer
		if err := result.Add(d); err != nil {
			log.Printf("[WARN] Failed to merge data: %v", err)
		}
	}
	return result, nil
}

// UpdateFlows updates a batch of RRD files with data from aggregated flow
// Returns error
func (s *rrdService) UpdateFlows(flows map[string]*models.AggregatedFlow) error {
	if len(flows) == 0 {
		return nil
	}

	now := time.Now().Unix()
	var cmdBuffer bytes.Buffer

	for _, flow := range flows {
		dir := filepath.Join(s.basePath, "flows", flow.SrcMAC)
		fileName := fmt.Sprintf("flow_%s_to_%s.rrd", flow.SrcMAC, flow.DstMAC)
		fullPath := filepath.Join(dir, fileName)

		if _, err := os.Stat(fullPath); os.IsNotExist(err) {
			s.mu.Lock()
			if _, err := os.Stat(fullPath); os.IsNotExist(err) {
				if err := s.createRRDFile(dir, fullPath); err != nil {
					log.Printf("[WARN] Failed to create RRD file %s: %v", fullPath, err)
				}
			}
			s.mu.Unlock()
		}

		updateLine := fmt.Sprintf("UPDATE %s %d:%d:%d\n", fullPath, now, flow.Bytes4, flow.Bytes6)
		cmdBuffer.WriteString(updateLine)
	}

	cmdBuffer.WriteString(".\n")

	conn, err := net.Dial("unix", s.socketPath)
	if err != nil {
		return fmt.Errorf("unable to connect to rrdcached: %w", err)
	}
	defer func(conn net.Conn) {
		err := conn.Close()
		if err != nil {
			log.Printf("[ERR] error while closing UDP socket")
		}
	}(conn)

	reader := bufio.NewReader(conn)

	if _, err = conn.Write([]byte("BATCH\n")); err != nil {
		return fmt.Errorf("error writing BATCH: %w", err)
	}

	resp1, err := reader.ReadString('\n')
	if err != nil {
		return fmt.Errorf("error reading BATCH response: %w", err)
	}
	if !strings.HasPrefix(resp1, "0 Go ahead") {
		return fmt.Errorf("rrdcached rejected BATCH: %s", strings.TrimSpace(resp1))
	}

	if _, err = conn.Write(cmdBuffer.Bytes()); err != nil {
		return fmt.Errorf("error writing updates: %w", err)
	}

	resp2, err := reader.ReadString('\n')
	if err != nil {
		return fmt.Errorf("error reading final response: %w", err)
	}

	if !strings.HasPrefix(resp2, "0 errors") {
		log.Printf("[WARN] BATCH result: %s", strings.TrimSpace(resp2))

		var numErrors int
		_, _ = fmt.Sscanf(resp2, "%d errors", &numErrors)

		for i := 0; i < numErrors; i++ {
			errMsg, _ := reader.ReadString('\n')
			log.Printf("[ERR] failed UPDATE: %s", strings.TrimSpace(errMsg))
		}
	}

	return nil
}

// createRRDFile creates a new RRD file with proper setup
// Returns error
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

// addRRDFiles adds to existing RRDData result from multiple src->{dst[0], dst[n]} RRD files
// Returns error
func (s *rrdService) addRRDFiles(result *models.RRDData, srcMAC string, dstMACs []string) error {

	for _, dstMAC := range dstMACs {
		pathOut := filepath.Join(s.basePath, "flows", srcMAC, fmt.Sprintf("flow_%s_to_%s.rrd", srcMAC, dstMAC))
		pathIn := filepath.Join(s.basePath, "flows", dstMAC, fmt.Sprintf("flow_%s_to_%s.rrd", dstMAC, srcMAC))
		err := result.AddBiDirFiles(pathOut, pathIn)
		if err != nil {
			// skip error and continue for valuable data
			log.Printf("[WARN] Failed to add RRD files: %v", err)
			continue
		}
	}
	return nil
}
