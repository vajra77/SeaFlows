package handlers

import (
	"net/http"
	"seaflows/internal/rrd"
	"strconv"

	"github.com/gin-gonic/gin"
)

type FlowHandler struct {
	manager *rrd.Manager
}

func NewFlowHandler(manager *rrd.Manager) *FlowHandler {
	return &FlowHandler{manager: manager}
}

func (h *FlowHandler) Get(ctx *gin.Context) {
	// parse ID parameter
	src := ctx.Param("id")
	dst := ctx.Param("dst")
	sched := ctx.Param("schedule")
	proto, _ := strconv.Atoi(ctx.Param("proto"))

	data, err := h.manager.GetSingleFlow(src, dst, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}
