package handlers

import (
	"context"

	"github.com/gin-gonic/gin"
)

type APIHandler interface {
	GetSingleFlow(ctx *gin.Context)
	GetP2PFlow(ctx *gin.Context)
	GetMACs(ctx *gin.Context)
	GetASNs(ctx *gin.Context)
}

type NetHandler interface {
	Listen(ctx context.Context) error
}
