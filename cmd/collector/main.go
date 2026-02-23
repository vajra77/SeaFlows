package main

import (
	"context"
	"log"
	"os"
	"os/signal"
	"strconv"
	"syscall"
	"time"

	"seaflows/internal/handlers"
	"seaflows/internal/services"

	"github.com/joho/godotenv"
)

func main() {
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	err := godotenv.Load()
	if err != nil {
		log.Println("[WARN] Unable to find .env file, using system variables")
	}

	rrdPath := os.Getenv("RRD_ROOT_PATH")
	if rrdPath == "" {
		rrdPath = "/srv/rrd"
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

	listenAddress := os.Getenv("SFLOW_LISTEN_ADDRESS")
	if listenAddress == "" {
		listenAddress = ":6343"
	}

	storage := services.NewRRDService(rrdPath, rrdStep, rrdGamma)

	broker := services.NewBrokerService(60*time.Second, storage)
	go broker.Start()

	handler := handlers.NewSFlowHandler(listenAddress, broker)

	if err := handler.Listen(ctx); err != nil {
		panic(err)
	}
}
