# Makefile

all: collector exporter

collector:
	go build -o bin/seaflows-collector ./cmd/collector

exporter:
	go build -o bin/seaflows-exporter ./cmd/exporter

clean:
	go clean
	go clean -cache
	go clean -modcache
