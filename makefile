CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Source files
CLIENT_SRCS = cards.cpp common.cpp klient.cpp kierki-klient.cpp
SERVER_SRCS = cards.cpp common.cpp server.cpp kierki-serwer.cpp
# SRCS = cards.cpp common.cpp klient.cpp kierki-klient.cpp  #server.cpp kierki-serwer.cpp

# Object files
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
# OBJS = $(SRCS:.cpp=.o)

# Executable
# TARGET = kierki-klient  #kierki-serwer
CLIENT_TARGET = kierki-klient
SERVER_TARGET = kierki-serwer

# Default rule
all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS)
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)
# $(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_TARGET) $(SERVER_TARGET)
