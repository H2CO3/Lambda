all:
	xcrun -sdk macosx clang++ -std=c++14 -Wall -Wextra -pedantic -o lambda *.cc -O0 -g
