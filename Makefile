CXX      := g++
CXXFLAGS := -std=c++14 -O3 -pedantic-errors -Wall -Wextra -Werror
INCLUDE  := -I./

all: main piority_analyse

piority_analyse: decoder.o piority_analyse.o
	$(CXX) decoder.o piority_analyse.o -o piority_analyse

clean:
	rm *.o
	rm main
