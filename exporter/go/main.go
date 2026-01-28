package main

import (
	"encoding/json"
	"net/http"
	"strconv"
)

func main() {
	exporter := NewRRDExporter("/srv/rrd", 1.0)

	// APIv2: GET /api/v2/flow
	http.HandleFunc("/api/v2/flow", func(w http.ResponseWriter, r *http.Request) {
		src := r.URL.Query().Get("src")
		dst := r.URL.Query().Get("dst")
		protoStr := r.URL.Query().Get("proto")
		schedule := r.URL.Query().Get("schedule")

		proto, _ := strconv.Atoi(protoStr)

		values, err := exporter.GetFlowData(src, dst, proto, schedule)
		if err != nil {
			w.WriteHeader(http.StatusNotFound)
			json.NewEncoder(w).Encode(map[string]string{"error": "Unable to find RRD file"})
			return
		}

		response := FlowResponse{
			Source:      src,
			Destination: dst,
			Proto:       protoStr,
			Schedule:    schedule,
			Values:      values,
		}

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(response)
	})

	// APIv2: GET /api/v2/peer
	http.HandleFunc("/api/v2/peer", func(w http.ResponseWriter, r *http.Request) {
		src := r.URL.Query().Get("src")
		protoStr := r.URL.Query().Get("proto")
		schedule := r.URL.Query().Get("schedule")

		proto, _ := strconv.Atoi(protoStr)

		values, err := exporter.GetPeerData(src, proto, schedule)
		if err != nil {
			w.WriteHeader(http.StatusNotFound)
			json.NewEncoder(w).Encode(map[string]string{"error": "Unable to find RRD file"})
			return
		}

		response := FlowResponse{
			Source:      src,
			Destination: "all",
			Proto:       protoStr,
			Schedule:    schedule,
			Values:      values,
		}

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(response)
	})

	http.ListenAndServe(":8080", nil)
}
