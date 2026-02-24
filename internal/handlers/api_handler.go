package handlers

import (
	"fmt"
	"net/http"
	"seaflows/internal/services"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
)

type apiHandler struct {
	storage services.StorageService
	mapper  services.AddressMapperService
}

func NewAPIHandler(rrdS services.StorageService, mapS services.AddressMapperService) APIHandler {
	return &apiHandler{
		storage: rrdS,
		mapper:  mapS,
	}
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
