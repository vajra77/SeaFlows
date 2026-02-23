package handlers

import (
	"fmt"
	"net/http"
	"seaflows/internal/services"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
)

type flowHandler struct {
	storage services.StorageService
	mapper  services.AddressMapperService
}

func NewFlowHandler(rrdS services.StorageService, mapS services.AddressMapperService) FlowHandler {
	return &flowHandler{
		storage: rrdS,
		mapper:  mapS,
	}
}

func (h *flowHandler) GetSingleFlow(ctx *gin.Context) {

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

func (h *flowHandler) GetP2PFlow(ctx *gin.Context) {

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

	srcMACs := h.mapper.GetMACs(srcAsn)
	dstMACs := h.mapper.GetMACs(dstAsn)

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
