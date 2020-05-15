#include "main.h"

//#define _WIN32

#ifdef _WIN32
#define clrscr() system("cls")
#else
#define clrscr() system("clear")
#endif
#define pause() free(readline(stdin))

//const int TABLE_SIZE = 9973; // could be any prime - doesn't work in visual studio

char *readline(FILE *fp)
{
  size_t size = BUFFER_SIZE;
  char *str;
  int ch;
  size_t len = 0;
  str = calloc(1ul, size);
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
      long tmp = (long)num * 10l + (long)str[i] - (long)'0';
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

int hash(int key)
{
  return key % TABLE_SIZE;
}

int hash_step(int key)
{
  return key % (TABLE_SIZE - 1) + 1; // добавляем 1, чтобы шаг не был равен 0
  //return 1; // for no double-hashing v
}

int add_to_table(Item_t *table, int key, char *value)
{
  int stpos, i, step = hash_step(key), release = 0;
  stpos = i = hash(key);
  while (table[i].busy > 0)
  {
    if (table[i].key == key)
      ++release;
    i = hash(i + step);
    if (i == stpos)
      return -1; // table full
  }
  table[i].busy = 1;
  table[i].key = key;
  table[i].release = release;
  table[i].info = value;
  return i;
}

Item_t *find_in_table(Item_t *table, int key)
{
  Item_t *new_tab = calloc(TABLE_SIZE, sizeof(Item_t));
  int stpos, i, step = hash_step(key);
  stpos = i = hash(key);
  while (table[i].busy != 0)
  {
    if (table[i].key == key && table[i].busy == 1)
    {
      new_tab[i].key = key;
      new_tab[i].info = table[i].info;
      new_tab[i].release = table[i].release;
      new_tab[i].busy = 1;
    }
    i = hash(i + step);
    if (i == stpos)
      break;
  }
  return new_tab;
}

int find_in_table_exact(Item_t *table, int key, int release)
{
  int stpos, i, step = hash_step(key);
  stpos = i = hash(key);
  while (table[i].busy != 0)
  {
    if (table[i].key == key && table[i].busy == 1 && release == table[i].release)
      return i;
    i = hash(i + step);
    if (i == stpos)
      break;
  }
  return -1; // item not found
}

int remove_from_table(Item_t *table, int key, int release)
{
  int res = find_in_table_exact(table, key, release);
  if (res >= 0)
  {
    table[res].busy = -1;
    free(table[res].info);
  }
  return res;
}

void print_table(Item_t *table)
{
  int flag = 0;
  printf("%9s | %13s | %8s | %s\n", "position", "key", "release", "info");
  for (int i = 0; i < TABLE_SIZE; ++i)
    if (table[i].busy == 1)
    {
      printf("%9d | %13d | %8d | %s\n", i, table[i].key, table[i].release, table[i].info);
      flag = 1;
    }
  if (flag)
    return;
  clrscr();
  printf("Table is empty...\n");
}

// --------------------------------------------------- menu -------------------------------------------------------
void insert_case(Item_t *table)
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
  // if (len > 0)     // еще тут этих строк не должно быть по-идее, с ними одна буква удаляется из строки
  //   info[len - 1] = 0;
  int res = add_to_table(table, key, info);
  clrscr();
  if (res < 0)
  {
    printf("Error! Table is full\n");
    free(info);
  }
  else
    printf("Success! Inserted \'%s\' with key %d to position %d\n", info, key, res);
  printf("Press 'enter' to continue...");
  pause();
}

void remove_case(Item_t *table)
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

void find_case_exact(Item_t *table)
{
  printf("Find exact mode\n\n");
  printf("Enter key to find>     ");
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
    printf("Found info:\n'%s'\nof %d key and %d release at %d position\n", table[res].info, key, release, res);
  else
    printf("Error! Release %d with key %d not found!\n", release, key);
}

void find_case_all(Item_t *table)
{
  printf("Find all mode\n\n"
         "Enter key to find> ");
  int key = readint(stdin);
  while (key == 0 && errno != 0)
  {
    printf("Invalid input! Try again> ");
    key = readint(stdin);
  }
  Item_t *new_table = find_in_table(table, key);
  clrscr();
  print_table(new_table);
  free(new_table);
}

void find_case(Item_t *table)
{
  int run = 1;
  while (run)
  {
    clrscr();
    printf("Find mode\n\n"
           "Do you want to find exact release? (yes/no/go back)\n"
           "                                    _   _     _    \n"
           "Choice: ");
    char *choice = readline(stdin);
    clrscr();
    if (strlen(choice) != 1)
      goto skip_more_than_one_letter;
    switch (choice[0]) // если choice[0] == 'y' или 'Y' переходит в позицию 216/217
                       // аналогично для других букв: 'n'|'N' -> 222
                       // 'b'|'B' -> 227, при любом другом значении переходит в default
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
      return;
    default:
      break;
    }
  skip_more_than_one_letter:
    free(choice);
  }
  printf("Press 'enter' to continue...");
  getchar();
}

void print_case(Item_t *table)
{
  printf("Print mode\n\n");
  print_table(table);
  printf("Press 'enter' to continue...");
  getchar();
}
// --------------------------------------------------- menu -------------------------------------------------------
//#undef TEST
#ifndef TEST
int main()
{
  Item_t table[TABLE_SIZE];
  memset(table, 0, TABLE_SIZE * sizeof(Item_t)); // обнуляем вектор таблицы, чтобы сделать busy == 0 везде
  int run = 1;
  while (run)
  {
    clrscr();
    printf("Main mode\n\n"
           "Avaliable actions:\n"
           "insert remove find print quit\n"
           "_      _      _    _     _\n"
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
}
#endif