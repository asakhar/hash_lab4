#include "source.h"
#include "stdlib.h"

int main()
{
  do {
    Item *string = calloc(1, sizeof(Item));
    // printf("%d\n", input(string)); // debug
    input(string);
    //print(string); // debug
    proccess(string);
    print(string);
    listfree(string);
  } while (run);
}