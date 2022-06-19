INC = $(wildcard *.h)
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
EXE = run

CXX = UPCXX_GASNET_CONDUIT=udp upcxx
UPCEXEC = upcxx-run
LDFLAGS =  -O2 -o $(EXE)
UPCFLAGS = -shared-heap 256M -n 4 $(shell upcxx-nodes nodes)
CPPFLAGS = -Wall -ggdb -std=c++17

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ)
$(OBJ): $(SRC)

out:
	$(UPCEXEC) $(UPCFLAGS) ./$(EXE)

.PHONY: clean run

clean:
	rm $(OBJ) $(EXE)

nodes:
	$(shell /opt/nfs/config/station206_name_list.sh 1 16 > nodes)