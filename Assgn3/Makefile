
all: producer consumer main Main

producer:producer.cpp
	g++ producer.cpp -o producer
	
consumer:consumer.cpp
	g++ consumer.cpp -o consumer

main:
	g++ main.cpp -o main
	
Main: main consumer producer
	./main
	
OPTIMISER: main consumer producer
	./main -optimize
CLEAN:
	rm main consumer producer op*
