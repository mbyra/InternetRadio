odbiornik = sikradio-sender
nadajnik = sikradio-receiver
OBJ = DataDownloader.o err.o MenuAgent.o parameters.o parsers.o Receiver.o RequestGatherer.o RetransmissionRequester.o Sender.o StationFinder.o
TARGET: sender receiver

CXX	= g++
CXXFLAGS= -pthread -lrt -Wall -O2 -Wextra -std=c++17
LFLAGS = -Wall -pthread -lrt

sender: $(nadajnik).o $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(nadajnik) $(nadajnik).o $(OBJ)

receiver: $(odbiornik).o $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(odbiornik) $(odbiornik).o $(OBJ)

$(nadajnik).o: $(OBJ)
	$(CXX) -c $(CXXFLAGS) $(nadajnik).cpp

$(odbiornik).o: $(OBJ)
	$(CXX) -c $(CXXFLAGS) $(odbiornik).cpp


DataDownloader.o: DataDownloader.cpp
	$(CXX) -c $(CXXFLAGS) DataDownloader.cpp -o DataDownloader.o

err.o: err.cpp
	$(CXX) -c $(CXXFLAGS) err.cpp -o err.o

MenuAgent.o: MenuAgent.cpp
	$(CXX) -c $(CXXFLAGS) MenuAgent.cpp -o MenuAgent.o

parameters.o: parameters.cpp
	$(CXX) -c $(CXXFLAGS) parameters.cpp -o parameters.o

parsers.o: parsers.cpp
	$(CXX) -c $(CXXFLAGS) parsers.cpp -o parsers.o

Receiver.o: Receiver.cpp
	$(CXX) -c $(CXXFLAGS) Receiver.cpp -o Receiver.o

RequestGatherer.o: RequestGatherer.cpp
	$(CXX) -c $(CXXFLAGS) RequestGatherer.cpp -o RequestGatherer.o

RetransmissionRequester.o: RetransmissionRequester.cpp
	$(CXX) -c $(CXXFLAGS) RetransmissionRequester.cpp -o RetransmissionRequester.o

Sender.o: Sender.cpp
	$(CXX) -c $(CXXFLAGS) Sender.cpp -o Sender.o

StationFinder.o: StationFinder.cpp
	$(CXX) -c $(CXXFLAGS) StationFinder.cpp -o StationFinder.o


.PHONY: clean TARGET

clean: 
	rm -f $(odbiornik) $(nadajnik) *.o *~ *.bak

