#include "../external/catch2/catch.hpp"
#include <iostream>
#include <mcheck.h>

extern "C" {
#include "source.h"
}

//namespace editstd{
#include <list>
//}

namespace std {
struct charlist : public std::list<char> {
  charlist(char const *str) : list<char>{} {
    std::string s(str);
    for (auto i : s)
      emplace_back(i);
  }
};
} // namespace std

Item *create_list(std::list<char> elements) {
  Item *list = new Item(), *start = list;
  for (auto elem : elements)
    (list = (list->next = new Item()))->let = elem;
  return start->next ? start->next : start;
}

Item *create_list(std::charlist elements) {
  Item *list = new Item(), *start = list;
  for (auto elem : elements)
    (list = (list->next = new Item()))->let = elem;
  return start->next ? start->next : start;
}

bool operator==(Item list1, Item list2) {
  Item *p1 = &list1, *p2 = &list2;
  do
    if (p1->let != p2->let) {
      std::cerr << "cmp err\n";
      return false;
    }
  while ((p1 = p1->next, p2 = p2->next) && p1);
  if (p1 || p2) {
    std::cerr << "len err\n";
    return false;
  }
  return true;
}

void no_op(__attribute__((unused)) enum mcheck_status status) {}

TEST_CASE("mem leak test") {
  mcheck(&no_op);
  Item *head = create_list("  fdgfhg123 asfdnq s01 01    "), *elem = head;

  std::vector<Item *> list_ptrs{};
  do
    list_ptrs.push_back(elem);
  while ((elem = elem->next));
  
  int * a = static_cast<int*>(calloc(1, sizeof(a)));
  proccess(head);
  listfree(head);
  for (auto item : list_ptrs) 
   CHECK(mprobe(item) != MCHECK_OK);
}

TEST_CASE("some random shit test") {
  Item *head = create_list("  fdgfhg123 asfdnq s01 01    ");
  proccess(head);
  CHECK(*head == *create_list("01 01"));
  listfree(head);
}

TEST_CASE("skips invalid") {
  Item *head = create_list("123");
  proccess(head);
  CHECK(*head == *create_list(""));
  listfree(head);
}

TEST_CASE("skips invalid & keeps valid") {
  Item *head = create_list("12301");
  proccess(head);
  CHECK(*head == *create_list("01"));
  listfree(head);
}

TEST_CASE("skip invalid & keeps many valid") {
  Item *head = create_list("12301111101010110101001");
  proccess(head);
  CHECK(*head == *create_list("01010101010101"));
  listfree(head);
}

TEST_CASE("spaces") {
  Item *head = create_list("1230 11111 0101 01101010 01");
  proccess(head);
  CHECK(*head == *create_list("0101 010101 01"));
  listfree(head);
}

TEST_CASE("spaces in the begining") {
  Item *head = create_list("   1230 11111 0101 01101010 01");
  proccess(head);
  CHECK(*head == *create_list("0101 010101 01"));
  listfree(head);
}

TEST_CASE("spaces in the begining & begin with valid") {
  Item *head = create_list("   0101 01101010 01asa");
  proccess(head);
  CHECK(*head == *create_list("0101 010101 01"));
  listfree(head);
}

TEST_CASE("many spaces in the middle & begin with valid") {
  Item *head = create_list("   0101 011          01010 01asa");
  proccess(head);
  CHECK(*head == *create_list("0101 01 0101 01"));
  listfree(head);
}
