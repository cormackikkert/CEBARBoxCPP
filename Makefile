CC = g++
CFLAGS = -std=c++17 -Wall -g -O3 -mavx2 -fopenmp -static -I/usr/local/include/antlr4-runtime
LIBS = -L/usr/local/lib -lminisat -L./Prover/IpasirProver -lipasir -lantlr4-runtime -I/usr/local/include/antlr4-runtime

SRCDIR = .
SOURCES = $(shell find $(SRCDIR) -name "*.cpp")
OBJECTS_MAIN = $(filter-out ./ltlmain.o, $(SOURCES:.cpp=.o))
OBJECTS_LTLMAIN = $(filter-out ./main.o, $(SOURCES:.cpp=.o))
EXECUTABLE_MAIN = kaleidoscope
EXECUTABLE_LTLMAIN = lumen

all: $(EXECUTABLE_MAIN) $(EXECUTABLE_LTLMAIN)

$(EXECUTABLE_MAIN): $(OBJECTS_MAIN)
	$(CC) $(CFLAGS) $^ -o $(EXECUTABLE_MAIN) $(LIBS)

$(EXECUTABLE_LTLMAIN): $(OBJECTS_LTLMAIN)
	$(CC) $(CFLAGS) $^ -o $(EXECUTABLE_LTLMAIN) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -I/usr/local/include/antlr4-runtime -c $< -o $@

clean:
	rm -f $(OBJECTS_MAIN) $(EXECUTABLE_MAIN) $(OBJECTS_LTLMAIN) $(EXECUTABLE_LTLMAIN)
