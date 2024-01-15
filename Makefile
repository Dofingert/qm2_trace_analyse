CXX      := g++-11
CXXFLAGS := -std=gnu++11 -pedantic-errors -Wall -Wextra -Werror
INCLUDE  := -I./

all: main.o
	$(CXX) main.o -o main

clean:
	rm main.o
	rm main
