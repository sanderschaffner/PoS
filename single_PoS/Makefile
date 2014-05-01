PLATFORM = linux64
INC      = /opt/gurobi562/linux64/include
CPP      = g++
CARGS    = -m64 -g
CPPLIB   = -L/opt/gurobi562/linux64/lib/ -lgurobi_c++ -lgurobi56

all: single_pos

run: run_single_pos

single_pos : single_pos
	$(CPP) $(CARGS) -o single_pos single_pos.cpp -I$(INC) $(CPPLIB) -lpthread -lm

run_single_pos: single_pos
	./single_pos

clean:
	rm -rf *.o *_c *_c++ *.class *.log *.rlp *.lp *.bas *.ilp single_pos
