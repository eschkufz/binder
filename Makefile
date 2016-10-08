all:
	g++ -std=c++11 -Werror -Wextra -Wall -pedantic -I. tools/database.cc -o bin/database -lhiredis
	g++ -std=c++11 -Werror -Wextra -Wall -pedantic -I. tools/example.cc -o bin/example -lhiredis

clean:
	rm -f bin/example bin/database
