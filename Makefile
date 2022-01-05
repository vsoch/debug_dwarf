all:
	gofmt -s -w .
	go build -o debug_dwarf
	
build:
	go build -o debug_dwarf	

run:
	go run main.go
