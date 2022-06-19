INC = $(wildcard *.h)
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
EXE = run

CXX = UPCXX_GASNET_CONDUIT=udp upcxx
MPIEXEC = upcxx-run
LDFLAGS =  -O2 -o $(EXE)
MPIFLAGS = -shared-heap 256M -n 4
CPPFLAGS = -Wall -ggdb -std=c++17

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ)
$(OBJ): $(SRC)

out:
	$(MPIEXEC) $(MPIFLAGS) ./$(EXE)

.PHONY: clean run

clean:
	rm $(OBJ) $(EXE)