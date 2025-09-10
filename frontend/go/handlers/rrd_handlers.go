package handlers

import (
	"encoding/json"
	"frontend/go/rrd"
	"net/http"
)

func HandleGetFlows(w http.ResponseWriter, r *http.Request) {

	var src string = r.URL.Query().Get("src")
	var dst string = r.URL.Query().Get("dst")
	var schedule string = r.URL.Query().Get("period")

	data, err := rrd.GetFlow(schedule, src, dst)

	if err != nil {
		http.Error(w, "Errore nella lettura dei dati RRD", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(data)
}
