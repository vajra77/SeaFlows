package handlers

import (
	"fmt"
	"net/http"
	"seaflows/internal/services"
	"strconv"
	"strings"

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

	srcMACs := h.mapService.GetMACs(srcAsn)
	dstMACs := h.mapService.GetMACs(dstAsn)

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

	data, err := h.rrdService.GetMultipleFlows(srcMACs, dstMACs, proto, sched)
	if err != nil {
		ctx.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}

	ctx.JSON(http.StatusOK, data)
}
