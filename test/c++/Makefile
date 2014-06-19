PLATFORM = linux64
INC      = /opt/gurobi562/linux64/include
CPP      = g++
CARGS    = -m64 -g
CPPLIB   = -L/opt/gurobi562/linux64/lib/ -lgurobi_c++ -lgurobi56

all: test_1 test_2 sudoku

run: run_test_1 run_test_2 sudoku

test_1 : test_1.cpp
	$(CPP) $(CARGS) -o test_1 test_1.cpp -I$(INC) $(CPPLIB) -lpthread -lm

test_2 : test_2.cpp
	$(CPP) $(CARGS) -o test_2 test_2.cpp -I$(INC) $(CPPLIB) -lpthread -lm

sudoku : sudoku.cpp
	$(CPP) $(CARGS) -o sudoku sudoku.cpp -I$(INC) $(CPPLIB) -lpthread -lm

dense : dense_c++.cpp
	$(CPP) $(CARGS) -o dense dense_c++.cpp -I$(INC) $(CPPLIB) -lpthread -lm

facility : facility_c++.cpp
	$(CPP) $(CARGS) -o facility facility_c++.cpp -I$(INC) $(CPPLIB) -lpthread -lm

run_test_1: test_1
	./test_1

run_test_2: test_2
	./test_2

run_sudoku: sudoku
	./sudoku < data/sudoku1

run_dense: dense
	./dense

run_facility: facility
	./facility

clean:
	rm -rf *.o *_c *_c++ *.class *.log *.rlp *.lp *.bas *.ilp
