package main

import (
	"log"
	"os"
	"seaflows/internal/handlers"
	"seaflows/internal/middleware"
	"seaflows/internal/services"
	"strconv"

	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"
)

func main() {

	err := godotenv.Load()
	if err != nil {
		log.Println("[WARN] Unable to find .env file, using system variables")
	}

	rrdRootDir := os.Getenv("RRD_ROOT_DIR")
	rrdGamma, _ := strconv.ParseFloat(os.Getenv("RRD_GAMMA"), 64)
	serverPort := os.Getenv("SERVER_PORT")
	ixfUrl := os.Getenv("IXF_URL")

	mapSrv, err := services.NewMapService(ixfUrl)
	if err != nil {
		log.Fatal("[CRIT] unable to initialize map service")
	}

	rrdSrv := services.NewRRDService(rrdRootDir, rrdGamma)
	flowHdr := handlers.NewFlowHandler(rrdSrv)
	p2pHdr := handlers.NewP2PHandler(mapSrv, rrdSrv)

	// setup Gin
	gin.DefaultWriter = os.Stdout
	gin.DefaultErrorWriter = os.Stderr
	r := gin.Default()
	if err := r.SetTrustedProxies([]string{"127.0.0.1", "::1"}); err != nil {
		log.Fatalf("[CRIT] Unable to set trusted proxies: %s", err)
	}

	// define routes
	v1 := r.Group("/api/v1")
	v1.Use(middleware.APIKeyAuth())
	{
		v1.GET("/flow", flowHdr.Get)
		v1.GET("/p2p", p2pHdr.Get)
	}

	// running Gin
	log.Printf("[INFO] API Server listening on port :%s", serverPort)
	err = r.Run(":" + serverPort)
	if err != nil {
		log.Printf("[WARN] Unable to start server")
		log.Fatalf("[CRIT] Shutdown due to error: %s", err)
	}
}
