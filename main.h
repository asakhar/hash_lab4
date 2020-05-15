#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

const size_t BUFFER_SIZE = 80;

const size_t TABLE_SIZE = 13;

extern char *readline(FILE *fp);
extern int str2int(char *str);
extern int readint(FILE *fp);
typedef struct Item
{
  int busy; // 0 - avaliable, 1 - busy, -1 - deleted
  int key;
  int release;
  char *info;
} Item_t;
extern int hash(int key);
extern int hash_step(int key);
extern int add_to_table(Item_t *table, int key, char *value);
extern Item_t *find_in_table(Item_t *table, int key);
extern int find_in_table_exact(Item_t *table, int key, int release);
extern int remove_from_table(Item_t *table, int key, int release);
extern void print_table(Item_t *table);