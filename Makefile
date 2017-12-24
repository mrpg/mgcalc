CXX = clang++
CXXFLAGS = -Wall -O3 -s -std=c++11 -lgmpxx -lgmp
INSTALL_PROGRAM = install
BINDIR = /usr/bin

all: mgcalc

clean:
	rm mgcalc

mgcalc: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o mgcalc

install: mgcalc
	$(INSTALL_PROGRAM) mgcalc $(BINDIR)/mgcalc
