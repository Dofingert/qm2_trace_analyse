CXX      := g++-11
CXXFLAGS := -std=gnu++11 -pedantic-errors -Wall -Wextra -Werror
INCLUDE  := -I./

all: main piority_analyse

clean:
	rm main.o
	rm main
