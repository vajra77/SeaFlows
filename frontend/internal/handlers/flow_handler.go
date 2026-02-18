package handlers

import (
	"net/http"
	"seaflows/internal/services"
	"strconv"

	"github.com/gin-gonic/gin"
)

type FlowHandler struct {
	service services.RRDService
}

func NewFlowHandler(rrd services.RRDService) *FlowHandler {
	return &FlowHandler{service: rrd}
}

func (h *FlowHandler) Get(ctx *gin.Context) {
	// parse ID parameter
	src := ctx.Param("id")
	dst := ctx.Param("dst")
	sched := ctx.Param("schedule")
	proto, _ := strconv.Atoi(ctx.Param("proto"))

	data, err := h.service.GetSingleFlow(src, dst, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}
