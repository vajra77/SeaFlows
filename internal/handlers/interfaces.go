package handlers

import (
	"context"

	"github.com/gin-gonic/gin"
)

type FlowHandler interface {
	GetSingleFlow(ctx *gin.Context)
	GetP2PFlow(ctx *gin.Context)
}

type MapHandler interface {
	GetMACs(ctx *gin.Context)
	GetASNs(ctx *gin.Context)
}

type NetHandler interface {
	Listen(ctx context.Context) error
}
