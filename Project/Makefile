PLATFORM = linux64
INC      = /opt/gurobi562/linux64/include
CPP      = g++
CARGS    = -m64 -g
CPPLIB   = -L/opt/gurobi562/linux64/lib/ -lgurobi_c++ -lgurobi56

all: 3players 3players_v0

run: run_3players run_v0

3players :
	$(CPP) $(CARGS) -o 3players 3players.cpp -I$(INC) $(CPPLIB) -lpthread -lm

3players_v2 :
	$(CPP) $(CARGS) -o 3players_v0 3players_v0.cpp -I$(INC) $(CPPLIB) -lpthread -lm

run_3players: 3players
	./3players

run_v0: 3players_v0
	./3players_v0

clean:
	rm -rf *.o *_c *_c++ *.class *.log *.rlp *.lp *.bas *.ilp 3players
