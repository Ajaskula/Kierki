# nazwa pliku wykonywalnego
TARGET = klient

#kompilator i opcje kompilatora
CXX = g++
CXXFLAGS = -Wall -g

# Pliki źródłowe
SOURCES = common.cpp klient.cpp

# Pliki obiektowe
OBJECTS = $(SOURCES:.cpp=.o)

# Domyślna reguła
all: $(TARGET)

# Reguła budowy pliku wykonywalnego
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

# Reguła budowy plików obiektowych
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

# Reguła czyszczenia katalogu
clean:
	rm -f $(OBJECTS) $(TARGET)

rebuild: clean all