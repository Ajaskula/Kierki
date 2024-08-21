CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Source files
SRCS = cards.cpp common.cpp klient.cpp kierki-klient.cpp # kierki-serwer.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable
TARGET = kierki-klient # kierki-serwer

# Default rule
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)
