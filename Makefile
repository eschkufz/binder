all:
	g++ -std=c++11 -Werror -Wextra -Wall -pedantic -I. tools/example.cc -o bin/example

clean:
	rm -f bin/example
