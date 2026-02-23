package main

import (
	"context"
	"log"
	"os"
	"os/signal"
	"seaflows/internal/handlers"
	"seaflows/internal/models"
	"seaflows/internal/services"
	"strconv"
	"syscall"
	"time"

	"github.com/joho/godotenv"
)

func main() {
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	err := godotenv.Load()
	if err != nil {
		log.Println("[WARN] Unable to find .env file, using system variables")
	}

	rrdPath := os.Getenv("RRD_BASE_PATH")
	if rrdPath == "" {
		rrdPath = "/srv/rrd"
	}

	rrdCache := os.Getenv("RRD_CACHE_SOCKET")
	if rrdCache == "" {
		rrdCache = "/var/run/rrdcached.sock"
	}

	gammaStr := os.Getenv("RRD_GAMMA")
	rrdGamma, err := strconv.ParseFloat(gammaStr, 64)
	if err != nil {
		log.Printf("[WARN] RRD_GAMMA missing or invalid ('%s'), fallback to 1.0", gammaStr)
		rrdGamma = 1.0
	}

	listenAddress := os.Getenv("COLLECTOR_ADDRESS")
	if listenAddress == "" {
		listenAddress = ":6343"
	}

	storage := services.NewRRDService(rrdPath, rrdCache, models.RRDStep, rrdGamma)

	broker := services.NewSflowService(models.RRDFlushInterval*time.Second, storage)
	go broker.Start()

	handler := handlers.NewSFlowHandler(listenAddress, broker)

	if err := handler.Listen(ctx); err != nil {
		panic(err)
	}
}
