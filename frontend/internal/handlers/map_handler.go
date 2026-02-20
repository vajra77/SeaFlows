package handlers

import (
	"net/http"
	"seaflows/internal/services"
	"strings"

	"github.com/gin-gonic/gin"
)

type MapHandler struct {
	service services.MapService
}

func NewMapHandler(service services.MapService) *MapHandler {
	return &MapHandler{service: service}
}

func (h *MapHandler) GetMACs(ctx *gin.Context) {

	var data []string

	asnStr := strings.ToUpper(ctx.Query("as"))
	asn := strings.TrimPrefix(asnStr, "AS")
	if asn == "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "parameter `as' is required"})
		return
	}
	data = h.service.GetMACs(asn)

	ctx.JSON(http.StatusOK, data)
}

func (h *MapHandler) GetASNs(ctx *gin.Context) {
	var data []string

	ctx.JSON(http.StatusOK, data)
}
