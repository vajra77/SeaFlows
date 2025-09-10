package main

import (
	"frontend/go/handlers"
	"github.com/go-chi/chi/v5"
	"log"
	"net/http"
)

func main() {
	router := chi.NewRouter()

	router.Get("/api/v2/data/flow", handlers.HandleGetFlow)

	http.ListenAndServe(":8080", r)
}
