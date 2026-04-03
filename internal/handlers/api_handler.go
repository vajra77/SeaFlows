package handlers

import (
	"fmt"
	"net/http"
	"seaflows/internal/services"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
)

type apiHandler struct {
	storage    services.StorageService
	mapper     services.AddressMapperService
	totalCache map[string]interface{}
	cacheMu    sync.Mutex
}

func NewAPIHandler(rrdS services.StorageService, mapS services.AddressMapperService) APIHandler {
	h := &apiHandler{
		storage:    rrdS,
		mapper:     mapS,
		totalCache: make(map[string]interface{}),
	}

	go h.startTotalFlowRefresh(10 * time.Minute)
	return h
}

func (h *apiHandler) startTotalFlowRefresh(interval time.Duration) {
	ticker := time.NewTicker(interval)
	defer ticker.Stop()

	// Eseguiamo il primo calcolo subito
	h.refreshCache()

	for range ticker.C {
		h.refreshCache()
	}
}

func (h *apiHandler) refreshCache() {
	// Implementazione del calcolo pesante
	asns := h.mapper.GetASNs()
	if len(asns) == 0 {
		return
	}

	var allMACs []string
	for _, asn := range asns {
		macs := h.mapper.GetMACsFromAS(asn.ASN)
		allMACs = append(allMACs, macs...)
	}

	// Nota: qui potresti voler iterare per i vari protocolli/schedules comuni
	// Per semplicità ora gestiamo una chiave combinata
	schedules := []string{"daily", "weekly", "monthly", "yearly"}
	protocols := []int{0, 4, 6}

	for _, s := range schedules {
		for _, p := range protocols {
			data, err := h.storage.GetFlows(allMACs, allMACs, p, s)
			if err == nil {
				key := fmt.Sprintf("%s-%d", s, p)
				h.cacheMu.Lock()
				h.totalCache[key] = data
				h.cacheMu.Unlock()
			}
		}
	}
}

func (h *apiHandler) GetTotalFlow(ctx *gin.Context) {
	sched := ctx.Query("schedule")
	if sched == "" {
		sched = "daily"
	}
	protoStr := ctx.Query("proto")
	proto, _ := strconv.Atoi(protoStr)

	key := fmt.Sprintf("%s-%d", sched, proto)

	h.cacheMu.Lock()
	data, exists := h.totalCache[key]
	h.cacheMu.Unlock()

	if !exists {
		ctx.JSON(http.StatusAccepted, gin.H{"message": "Data is being calculated, please try again later"})
		return
	}

	ctx.JSON(http.StatusOK, data)
}

func (h *apiHandler) GetSingleFlow(ctx *gin.Context) {

	srcMac := strings.ReplaceAll(ctx.Query("src_mac"), ":", "")
	dstMac := strings.ReplaceAll(ctx.Query("dst_mac"), ":", "")
	sched := ctx.Query("schedule")
	protoStr := ctx.Query("proto")

	if srcMac == "" || dstMac == "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "src_mac and dst_mac are required"})
		return
	}
	if sched == "" {
		sched = "daily"
	}

	proto, err := strconv.Atoi(protoStr)
	if err != nil && protoStr != "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "invalid proto value, must be integer"})
		return
	}

	data, err := h.storage.GetFlow(srcMac, dstMac, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}

func (h *apiHandler) GetP2PFlow(ctx *gin.Context) {

	srcStr := strings.ToUpper(ctx.Query("src_as"))
	srcAsn := strings.TrimPrefix(srcStr, "AS")

	dstStr := strings.ToUpper(ctx.Query("dst_as"))
	dstAsn := strings.TrimPrefix(dstStr, "AS")

	sched := ctx.Query("schedule")
	protoStr := ctx.Query("proto")

	if srcAsn == "" || dstAsn == "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "src_as and dst_as are required"})
		return
	}

	if sched == "" {
		sched = "daily"
	}

	proto, err := strconv.Atoi(protoStr)
	if err != nil && protoStr != "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "invalid proto value, must be integer"})
		return
	}

	srcMACs := h.mapper.GetMACsFromAS(srcAsn)
	dstMACs := h.mapper.GetMACsFromAS(dstAsn)

	if len(srcMACs) == 0 {
		errStr := fmt.Sprintf("no source MACs found for ASN %s ", srcAsn)
		ctx.JSON(http.StatusNotFound, gin.H{"error": errStr})
		return
	}

	if len(dstMACs) == 0 {
		errStr := fmt.Sprintf("no destination MACs found for ASN %s ", dstAsn)
		ctx.JSON(http.StatusNotFound, gin.H{"error": errStr})
		return
	}

	data, err := h.storage.GetFlows(srcMACs, dstMACs, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}

func (h *apiHandler) GetAggregateFlow(ctx *gin.Context) {
	srcStr := strings.ToUpper(ctx.Query("src_as_set"))
	srcSet := strings.ReplaceAll(srcStr, "AS", "")

	dstStr := strings.ToUpper(ctx.Query("dst_as_set"))
	dstSet := strings.ReplaceAll(dstStr, "AS", "")

	sched := ctx.Query("schedule")
	protoStr := ctx.Query("proto")

	if srcSet == "" || dstSet == "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "src_as and dst_as are required"})
		return
	}

	if sched == "" {
		sched = "daily"
	}

	proto, err := strconv.Atoi(protoStr)
	if err != nil && protoStr != "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "invalid proto value, must be integer"})
		return
	}

	srcASList := strings.Split(srcSet, ",")
	dstASList := strings.Split(dstSet, ",")

	srcMACs := h.mapper.GetMACsFromASSet(srcASList)
	dstMACs := h.mapper.GetMACsFromASSet(dstASList)

	data, err := h.storage.GetFlows(srcMACs, dstMACs, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}

func (h *apiHandler) GetMACs(ctx *gin.Context) {

	var data []string

	asnStr := strings.ToUpper(ctx.Query("as"))
	asn := strings.TrimPrefix(asnStr, "AS")
	if asn == "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "parameter `as' is required"})
		return
	}
	data = h.mapper.GetMACsFromAS(asn)

	ctx.JSON(http.StatusOK, data)
}

func (h *apiHandler) GetASNs(ctx *gin.Context) {

	data := h.mapper.GetASNs()

	ctx.JSON(http.StatusOK, data)
}
