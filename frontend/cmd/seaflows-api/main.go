package main

import (
	"log"
	"os"
	"seaflows/internal/handlers"
	"seaflows/internal/middleware"
	"seaflows/internal/rrd"
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

	rrdManager := rrd.NewManager(rrdRootDir, rrdGamma)
	flowHandler := handlers.NewFlowHandler(rrdManager)

	// setup Gin
	gin.DefaultWriter = os.Stdout
	gin.DefaultErrorWriter = os.Stderr
	r := gin.Default()
	r.SetTrustedProxies([]string{"127.0.0.1", "::1"})

	// define routes
	v1 := r.Group("/api/v1")
	v1.Use(middleware.APIKeyAuth())
	{
		v1.GET("/flow", flowHandler.Get)
	}

	// running Gin
	log.Printf("[INFO] API Server listening on port :%s", serverPort)
	err = r.Run(":" + serverPort)
	if err != nil {
		log.Printf("[WARN] Unable to start server")
		log.Fatalf("[CRIT] Shutdown due to error: %s", err)
	}
}
