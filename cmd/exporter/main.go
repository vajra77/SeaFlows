package main

import (
	"log"
	"net/http"
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
	if rrdRootDir == "" {
		rrdRootDir = "/srv/rrd"
	}

	rrdStep := os.Getenv("RRD_STEP")
	if rrdStep == "" {
		rrdStep = "300"
	}

	gammaStr := os.Getenv("RRD_GAMMA")
	rrdGamma, err := strconv.ParseFloat(gammaStr, 64)
	if err != nil {
		log.Printf("[WARN] RRD_GAMMA missing or invalid ('%s'), fallback to 1.0", gammaStr)
		rrdGamma = 1.0
	}

	listenAddr := os.Getenv("EXPORTER_LISTEN_ADDRESS")
	if listenAddr == "" {
		listenAddr = ":8080"
	}

	ixfUrl := os.Getenv("IXF_URL")
	if ixfUrl == "" {
		log.Fatal("[WARN] IXF_URL missing or invalid")
	}

	mapSrv, err := services.NewMapService(ixfUrl)
	if err != nil {
		log.Fatal("[CRIT] unable to initialize map service")
	}

	rrdSrv := services.NewRRDService(rrdRootDir, rrdStep, rrdGamma)
	mapHdlr := handlers.NewMapHandler(mapSrv)
	flowHdlr := handlers.NewFlowHandler(rrdSrv, mapSrv)

	// setup Gin
	gin.DefaultWriter = os.Stdout
	gin.DefaultErrorWriter = os.Stderr
	gin.SetMode(gin.ReleaseMode)

	r := gin.Default()
	if err := r.SetTrustedProxies([]string{"127.0.0.1", "::1"}); err != nil {
		log.Fatalf("[CRIT] Unable to set trusted proxies: %s", err)
	}

	// ping route
	r.GET("/ping", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"status":  "ok",
			"message": "pong",
			"version": "1.0.0",
		})
	})

	// define routes
	v1 := r.Group("/api/v1")
	v1.Use(middleware.APIKeyAuth())
	{
		v1.GET("/flow/mac", flowHdlr.GetSingleFlow)
		v1.GET("/flow/p2p", flowHdlr.GetP2PFlow)
		v1.GET("/map/macs", mapHdlr.GetMACs)
		v1.GET("/map/asns", mapHdlr.GetASNs)
	}

	// running Gin
	log.Printf("[INFO] API Server listening on: %s", listenAddr)
	err = r.Run(listenAddr)
	if err != nil {
		log.Printf("[WARN] Unable to start server")
		log.Fatalf("[CRIT] Shutdown due to error: %s", err)
	}
}
