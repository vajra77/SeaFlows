package handlers

import (
	"fmt"
	"net/http"
	"seaflows/internal/services"
	"strconv"

	"github.com/gin-gonic/gin"
)

type P2PHandler struct {
	mapService services.MapService
	rrdService services.RRDService
}

func NewP2PHandler(mapSrv services.MapService, rrdSrv services.RRDService) *P2PHandler {
	return &P2PHandler{
		mapService: mapSrv,
		rrdService: rrdSrv,
	}
}

func (h *P2PHandler) Get(ctx *gin.Context) {

	srcAsn := ctx.Param("src_as")
	dstAsn := ctx.Param("dst_as")
	sched := ctx.Param("schedule")
	proto, _ := strconv.Atoi(ctx.Param("proto"))

	srcMACs := h.mapService.GetMACs(srcAsn)
	dstMACs := h.mapService.GetMACs(dstAsn)

	if srcMACs == nil {
		errStr := fmt.Sprintf("[WARN] no source MACs found for ASN %s ", srcAsn)
		ctx.JSON(http.StatusNotFound, gin.H{"error": errStr})
		return
	}

	if dstMACs == nil {
		errStr := fmt.Sprintf("[WARN] no destination MACs found for ASN %s ", dstAsn)
		ctx.JSON(http.StatusNotFound, gin.H{"error": errStr})
		return
	}

	data, err := h.rrdService.GetMultipleFlows(srcMACs, dstMACs, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}
