typedef struct strlist
{
  char let;
  struct strlist *next;
} Item;

extern int run;

extern void print(Item *head);
extern unsigned long input(Item *dest);
extern void listfree(Item *list);
extern void proccess(Item *list);
extern Item rfree(Item *elem);