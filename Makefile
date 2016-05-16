CC=g++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=
COMMON_SOURCES=helpers.cpp
COMMON_OBJECTS=$(COMMON_SOURCES:.cpp=.o)
SERVER_SOURCES=server.cpp ServerFunctions.cpp
SERVER_OBJECTS=$(SERVER_SOURCES:.cpp=.o)
SERVER=server
CLIENT_SOURCES=client.cpp
CLIENT_OBJECTS=$(CLIENT_SOURCES:.cpp=.o)
CLIENT=client

all: $(COMMON_OBJECTS) $(SERVER) $(CLIENT)

$(SERVER): $(COMMON_OBJECTS) $(SERVER_OBJECTS)
	$(CC) $(LDFLAGS) $(COMMON_OBJECTS) $(SERVER_OBJECTS) -o $@

$(CLIENT): $(COMMON_OBJECTS) $(CLIENT_OBJECTS)
	$(CC) $(LDFLAGS) $(COMMON_OBJECTS) $(CLIENT_OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o server client
	rm -f /tmp/cs_pipe
	rm -f /tmp/cs_channel
