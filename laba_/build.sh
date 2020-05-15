g++ -o test -x c++ ccode/test.cpp -x none catch.o -x c ccode/source.c -lmcheck
g++ -o main -x c ccode/source.c -x c ccode/main.c
./test
