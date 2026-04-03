package models

// MonitorRecord contiene le informazioni essenziali per il debug in tempo reale
type MonitorRecord struct {
	Timestamp int64  `json:"ts"`
	SrcMAC    string `json:"src_mac"`
	DstMAC    string `json:"dst_mac"`
	Bytes4    uint64 `json:"b4"`
	Bytes6    uint64 `json:"b6"`
}
