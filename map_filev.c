#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

//#define _WIN32

#ifdef _WIN32
#define clrscr() system("cls")
#else
#define clrscr() system("clear")
#endif
#define pause() free(readline(stdin))

const size_t BUFFER_SIZE = 80;

//const size_t TABLE_SIZE = 9973; //const int TABLE_SIZE = 9973; // could be any prime
const size_t TABLE_SIZE = 13; // small table size for test

char *readline(FILE *fp)
{
  size_t size = BUFFER_SIZE;
  char *str;
  int ch;
  size_t len = 0;
  str = malloc(size);
  if (!str)
    return str;
  while (EOF != (ch = fgetc(fp)) && ch != '\n')
  {
    str[len++] = ch;
    if (len == size)
    {
      str = realloc(str, size += BUFFER_SIZE);
      if (!str)
        return str;
    }
  }
  str[len++] = '\0';
  return realloc(str, len);
}

int str2int(char *str)
{
  errno = 0;
  int num = 0, sign = 1;
  if (str[0] == '-')
  {
    sign = -1;
    str = str + 1;
  }
  for (int i = 0; i < strlen(str); ++i)
  {
    if (str[i] >= '0' && str[i] <= '9')
    {
      long tmp = (long)num * 10 + (long)str[i] - (long)'0';
      if (tmp > INT_MAX)
        errno = ERANGE;
      num = (int)tmp;
    }
    else
      errno = EINVAL;
    if (errno != 0)
      return 0;
  }
  return sign * num;
}

int readint(FILE *fp)
{
  char *res = readline(fp);
  int ret = str2int(res);
  free(res);
  return ret;
}

typedef struct Item
{
  int busy; // 0 - avaliable, 1 - busy, -1 - deleted
  int key;
  int release;
  int info_offset;
  int info_length;
} Item_t;

int hash(int key)
{
  return key % TABLE_SIZE;
}

int hash_step(int key)
{
  return key % (TABLE_SIZE - 1) + 1;
}

FILE *open_table(char *filename, int override)
{
  FILE *file;
  if (!override)
    file = fopen(filename, "rb+");
  if (override || !file)
  {
    file = fopen(filename, "wb+");
    char *fill = calloc(1, sizeof(Item_t));
    for (int i = 0; i < TABLE_SIZE; ++i)
      fwrite(fill, sizeof(Item_t), 1, file);
    fwrite(":", 1, 1, file);
    fflush(file);
    free(fill);
  }
  return file;
}

void send_to_file(FILE *file, int index, Item_t *item)
{
  fseek(file, index * sizeof(Item_t), SEEK_SET);
  fwrite(item, sizeof(Item_t), 1, file);
}

Item_t get_from_file(FILE *file, int index)
{
  Item_t ret;
  fseek(file, index * sizeof(Item_t), SEEK_SET);
  fread(&ret, sizeof(Item_t), 1, file);
  return ret;
}

char *get_info(FILE *file, Item_t *item)
{
  char *ret = calloc(item->info_length + 1, sizeof(char));
  fseek(file, item->info_offset, SEEK_SET);
  fread(ret, sizeof(char), item->info_length, file);
  return ret;
}

void add_info(FILE *file, Item_t *item, char *info)
{
  fseek(file, 0, SEEK_END);
  item->info_length = strlen(info);
  item->info_offset = ftell(file);
  fwrite(info, item->info_length, 1, file);
}

int add_to_table(FILE *table, int key, char *value)
{
  int stpos, i, step = hash_step(key), release = 0;
  stpos = i = hash(key);
  Item_t current = get_from_file(table, i);
  while (current.busy > 0)
  {
    if (current.key == key)
      ++release;
    i = hash(i + step);
    if (i == stpos)
      return -1;                       // table full
    current = get_from_file(table, i); // get next
  }
  current.busy = 1;
  current.key = key;
  current.release = release;
  add_info(table, &current, value);
  send_to_file(table, i, &current);
  return i;
}

FILE *f = 0;

FILE *find_in_table(FILE *table, int key)
{
  if (f)
    fclose(f);
  f = open_table("./.find_tmp", 1);
  int stpos, i, step = hash_step(key);
  stpos = i = hash(key);
  Item_t current = get_from_file(table, i);
  while (current.busy != 0)
  {
    if (current.key == key && current.busy == 1)
    {
      char *info_tmp = get_info(table, &current);
      add_info(f, &current, info_tmp); // !memleak! fixed
      free(info_tmp);
      send_to_file(f, i, &current);
    }
    i = hash(i + step);
    if (i == stpos)
      break;
    current = get_from_file(table, i);
  }
  return f;
}

int find_in_table_exact(FILE *table, int key, int release)
{
  int stpos, i, step = hash_step(key);
  stpos = i = hash(key);
  Item_t current = get_from_file(table, i);
  while (current.busy != 0)
  {
    if (current.key == key && current.busy == 1 && release == current.release)
      return i;
    i = hash(i + step);
    if (i == stpos)
      break;
    current = get_from_file(table, i);
  }
  return -1; // item not found
}

int remove_from_table(FILE *table, int key, int release)
{
  int stpos, i, step = hash_step(key);
  stpos = i = hash(key);
  Item_t current = get_from_file(table, i);
  while (current.busy != 0)
  {
    if (current.key == key && current.busy == 1 && release == current.release)
    {
      current.busy = -1;
      send_to_file(table, i, &current);
      return i;
    }
    i = hash(i + step);
    if (i == stpos)
      break;
    current = get_from_file(table, i);
  }
  return -1; // item not found
}

void print_table(FILE *table)
{
  int flag = 0;
  printf("%9s | %13s | %8s | %s\n", "position", "key", "release", "info");
  Item_t current;
  for (int i = 0; i < TABLE_SIZE; ++i)
    if ((current = get_from_file(table, i)).busy == 1)
    {
      char *info_tmp = get_info(table, &current);
      printf("%9d | %13d | %8d | %s\n", i, current.key, current.release, info_tmp); // !memleak! fixed
      free(info_tmp);
      flag = 1;
    }
  if (flag)
    return;
  clrscr();
  printf("Table is empty...\n");
}

// --------------------------------------------------- menu -------------------------------------------------------
void insert_case(FILE *table)
{
  printf("Insertion mode\n\n"
         "Enter insertion key>   ");
  int key = readint(stdin);
  while (key == 0 && errno != 0)
  {
    printf("Invalid input! Try again> ");
    key = readint(stdin);
  }
  printf("Enter insertion value> ");
  char *info = readline(stdin);
  int len = strlen(info);
  // if (len > 0)
  //   info[len - 1] = 0;
  int res = add_to_table(table, key, info);
  clrscr();
  if (res < 0)
    printf("Error! Table is full\n");
  else
    printf("Success! Inserted '%s' with key %d to position %d\n", info, key, res);
  free(info);
  printf("Press 'enter' to continue...");
  pause();
}

void remove_case(FILE *table)
{
  printf("Removal mode\n\n");
  printf("Enter key to remove>     ");
  int key = readint(stdin);
  while (key == 0 && errno != 0)
  {
    printf("Invalid input! Try again> ");
    key = readint(stdin);
  }
  printf("Enter release to remove> ");
  int release = readint(stdin);
  while (release == 0 && errno != 0)
  {
    printf("Invalid input! Try again> ");
    release = readint(stdin);
  }
  int res = remove_from_table(table, key, release);
  clrscr();
  if (res >= 0)
    printf("Success! Removed %d release of %d key from %d position\n", release, key, res);
  else
    printf("Error! Release %d with key %d not found!\n", release, key);
  printf("Press 'enter' to continue...");
  pause();
}

void find_case_exact(FILE *table)
{
  printf("Find exact mode\n\n"
         "Enter key to find>     ");
  int key = readint(stdin);
  while (key == 0 && errno != 0)
  {
    printf("Invalid input! Try again> ");
    key = readint(stdin);
  }
  printf("Enter release to find> ");
  int release = readint(stdin);
  while (release == 0 && errno != 0)
  {
    printf("Invalid input! Try again> ");
    release = readint(stdin);
  }
  int res = find_in_table_exact(table, key, release);
  clrscr();
  if (res >= 0)
  {
    Item_t item = get_from_file(table, res);
    char *info = get_info(table, &item);
    printf("Found info:\n'%s'\nof %d key and %d release at %d position\n", info, key, release, res);
    free(info);
  }
  else
    printf("Error! Release %d with key %d not found!\n", release, key);
}

void find_case_all(FILE *table)
{
  printf("Find all mode\n\n"
         "Enter key to find> ");
  int key = readint(stdin);
  while (key == 0 && errno != 0)
  {
    printf("Invalid input! Try again> ");
    key = readint(stdin);
  }
  clrscr();
  print_table(find_in_table(table, key));
}

void find_case(FILE *table)
{
  int run = 1;
  while (run)
  {
    clrscr();
    printf("Find mode\n\n"
           "Do you want to find exact release? (yes/no/go back)\n"
           "                                    _   _     _    \n\n"
           "Choice: ");
    char *choice = readline(stdin);
    clrscr();
    if (strlen(choice) != 1)
      goto skip_more_than_one_letter;
    switch (choice[0])
    {
    case 'y':
    case 'Y':
      find_case_exact(table);
      run = 0;
      break;
    case 'n':
    case 'N':
      find_case_all(table);
      run = 0;
      break;
    case 'b':
    case 'B':
      free(choice);
      return;
    default:
      break;
    }
  skip_more_than_one_letter:
    free(choice);
  }
  printf("Press 'enter' to continue...");
  pause();
}

void print_case(FILE *table)
{
  printf("Print mode\n\n");
  print_table(table);
  printf("Press 'enter' to continue...");
  pause();
}
// --------------------------------------------------- menu -------------------------------------------------------

int main(int argc, char **argv)
{
  char *filename;
  if (argc == 2)
    filename = argv[1];
  else
  {
    printf("Enter filename to be used as hashtable swap> ");
    filename = readline(stdin);
  }
  FILE *table = open_table(filename, 0);
  //free(filename);
  int run = 1;
  while (run)
  {
    clrscr();
    printf("Main mode\n\n"
           "Avaliable actions:\n"
           "insert remove find print quit\n"
           "_      _      _    _     _\n\n"
           "Enter action to perform: ");
    char *choice = readline(stdin);
    clrscr();
    if (strlen(choice) != 1)
      goto skip_more_than_one_letter;
    switch (choice[0])
    {
    case 'i':
      insert_case(table);
      break;
    case 'r':
      remove_case(table);
      break;
    case 'f':
      find_case(table);
      break;
    case 'p':
      print_case(table);
      break;
    case 'q':
      printf("HT Exit...");
      run = 0;
      break;
    default:
      break;
    }
  skip_more_than_one_letter:
    free(choice);
  }
  fclose(table);
  if (f)
    fclose(f);
}