#include "source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int run = 1;
const size_t SIZE = 10;

Item rfree(Item *elem) {
  Item ret = *elem;
  free(elem);
  return ret;
}

void print(Item *head) {
  printf("\"");
  do {
    printf("%c", head->let);
    // if (head->let == '\0')
    //   printf("\\0"); // debug
  } while ((head = head->next));
  printf("\"\n");
}

size_t input(Item *dest) {
  Item *current = dest;
  size_t len = 0;
  size_t clen;
  do {
    char buff[SIZE + 1];
    memset(buff, 0, SIZE + 1);
    scanf("%10[^\n]", buff);
    len += (clen = strlen(buff));
    for (size_t i = 0; i < clen; ++i) {
      if ((current->let == ' ' || current->let == '\0') &&
          (buff[i] == ' ' || buff[i] == '\t') && --len)
        continue;
      current = (current->next = calloc(1, sizeof(Item)));
      if (buff[i] == '\t')
        current->let = ' ';
      else
        current->let = buff[i];
    }
  } while (clen == SIZE);
  if (getchar() == EOF)
    run = 0;
  if (dest->next)
    *dest = rfree(dest->next);
  if (current->let == ' ')
    current->let = '\0';
  return len;
}

void listfree(Item *list) {
  if (list->next)
    listfree(list->next);
  free(list);
}

void proccess(Item *list) {
  Item *prev = 0, *prev2 = 0;
  while (list && list->next && list->next->let != '\0')
    if (list->let == '0' && list->next->let == '1') {
      prev2 = prev = list->next;
      list = list->next->next;
    } else if (list->let == ' ' && prev && prev->let != ' ') {
      prev = list;
      list = list->next;
    } else {
      *list = rfree(list->next);
    }
  if (prev && prev->let == ' ') {
    if (prev2) {
      listfree(prev2->next);
      prev2->next = 0;
      return;
    }
    prev->let = '\0';
    free(prev->next);
    prev->next = 0;
  }
  if (list) {
    if (prev) {
      free(prev->next);
      prev->next = 0;
    } else {
      list->let = '\0';
      free(list->next);
      list->next = 0;
    }
  }
}
