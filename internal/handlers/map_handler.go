package handlers

import (
	"net/http"
	"seaflows/internal/services"
	"strings"

	"github.com/gin-gonic/gin"
)

type mapHandler struct {
	mapper services.AddressMapperService
}

func NewMapHandler(service services.AddressMapperService) MapHandler {
	return &mapHandler{mapper: service}
}

func (h *mapHandler) GetMACs(ctx *gin.Context) {

	var data []string

	asnStr := strings.ToUpper(ctx.Query("as"))
	asn := strings.TrimPrefix(asnStr, "AS")
	if asn == "" {
		ctx.JSON(http.StatusBadRequest, gin.H{"error": "parameter `as' is required"})
		return
	}
	data = h.mapper.GetMACs(asn)

	ctx.JSON(http.StatusOK, data)
}

func (h *mapHandler) GetASNs(ctx *gin.Context) {

	data := h.mapper.GetASNs()

	ctx.JSON(http.StatusOK, data)
}
