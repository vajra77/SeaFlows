package handlers

import (
	"net/http"
	"seaflows/internal/services"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
)

type FlowHandler struct {
	service services.RRDService
}

func NewFlowHandler(service services.RRDService) *FlowHandler {
	return &FlowHandler{service: service}
}

func (h *FlowHandler) Get(ctx *gin.Context) {

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

	data, err := h.service.GetSingleFlow(srcMac, dstMac, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}
