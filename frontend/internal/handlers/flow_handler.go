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

func NewFlowHandler(rrd services.RRDService) *FlowHandler {
	return &FlowHandler{service: rrd}
}

func (h *FlowHandler) Get(ctx *gin.Context) {

	srcMac := strings.ReplaceAll(ctx.Param("src_mac"), ":", "")
	dstMac := strings.ReplaceAll(ctx.Param("dst_mac"), ":", "")
	sched := ctx.Param("schedule")
	proto, _ := strconv.Atoi(ctx.Param("proto"))

	data, err := h.service.GetSingleFlow(srcMac, dstMac, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}
