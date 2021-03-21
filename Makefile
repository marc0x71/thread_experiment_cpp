CXX=g++
CXXFLAGS=-g -Wall -MMD -std=c++17 -pthread
LDLIBS=-lm -pthread
LDFLAGS=-std=c++17

run: thread_experiment_cpp
	./thread_experiment_cpp

thread_experiment_cpp: thread_experiment_cpp.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) *.o *.d thread_experiment_cpp
-include $(wildcard *.d)