package rrd

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"sync"
)

const (
	basePath = "/srv/rrd/flows"
	rrdStep  = "300" // 5 minuti
)

var (
	// Mutex per evitare race condition sulla creazione fisica delle cartelle/file
	fileMu sync.Mutex
)

// Update riceve i MAC puliti e i byte aggregati dal Broker
func Update(srcMac, dstMac string, bytes uint64) error {
	dir := filepath.Join(basePath, srcMac)
	fileName := fmt.Sprintf("flow_%s_to_%s.rrd", srcMac, dstMac)
	fullPath := filepath.Join(dir, fileName)

	// 1. Verifica esistenza e creazione (Lazy Initialization)
	if _, err := os.Stat(fullPath); os.IsNotExist(err) {
		fileMu.Lock()
		// Double check dopo il lock
		if _, err := os.Stat(fullPath); os.IsNotExist(err) {
			if err := createRRD(dir, fullPath); err != nil {
				fileMu.Unlock()
				return err
			}
		}
		fileMu.Unlock()
	}

	// 2. Esecuzione Update
	// Usiamo il comando "N" per dire a RRDTool di usare il timestamp attuale ("Now")
	// bytes4 e bytes6 sono i DS che avevi nel codice C
	// Qui assumiamo un'unica DS per semplicità o possiamo differenziare
	updateArgs := fmt.Sprintf("N:%d", bytes)

	// Esempio con chiamata diretta (ottimizzabile con rrdcached)
	cmd := exec.Command("rrdtool", "update", fullPath, updateArgs)
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("rrd update failed: %v", err)
	}

	return nil
}

func createRRD(dir, path string) error {
	// Crea la cartella del MAC sorgente se non esiste
	if err := os.MkdirAll(dir, 0755); err != nil {
		return err
	}

	// Definizione del file RRD (simile a rrdtool.c)
	// DS:bytes:GAUGE:600:0:U -> Heartbeat 600s, valore minimo 0, max sconosciuto
	// RRA:AVERAGE:0.5:1:288  -> 1 giorno di dati a risoluzione 5 min
	args := []string{
		"create", path,
		"--step", rrdStep,
		"DS:bytes:GAUGE:600:0:U",
		"RRA:AVERAGE:0.5:1:288",  // 5 min per 24 ore
		"RRA:AVERAGE:0.5:6:336",  // 30 min per 1 settimana
		"RRA:AVERAGE:0.5:24:720", // 2 ore per 2 mesi
	}

	cmd := exec.Command("rrdtool", args...)
	return cmd.Run()
}
