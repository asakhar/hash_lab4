extern "C"
{
#include "main.h"
}
#include <catch.hpp>
#include <string>
#include <sstream>
#include <map>

char *str2charptr(std::string a)
{
  char *ret = new char[a.size() + 1];
  memcpy(ret, a.c_str(), a.size() + 1);
  return ret;
}

struct simples
{
  char *str;
  bool casted = 0;
  simples(const int num) : str{str2charptr(std::to_string(num))} {};
  simples(char const *cc) : str{str2charptr(cc)} {};
  simples(std::string s) : str{str2charptr(s)} {};
  operator char *()
  {
    casted = 1;
    return str;
  }
  ~simples()
  {
    if (!casted)
      delete[] str;
  }
  bool operator==(char *other) const
  {
    if (!other)
    {
      if (!str)
        return 1;
      else
        return 0;
    }
    return std::string{str} == other;
  }
  simples operator+(const int num)
  {
    auto s = std::string(str) + std::to_string(num);
    return simples(s);
  }
};

simples operator""_fs(char const *cc, size_t n)
{
  return simples{cc};
}

void cmp_tables(Item_t *tb, std::map<std::pair<int, int>, std::pair<std::string, bool>> &&map)
{
  for (int i = 0; i < TABLE_SIZE; ++i)
    if (tb[i].busy)
    {
      auto &val = map[{tb[i].key, tb[i].release}];
      REQUIRE(val.first == tb[i].info);
      REQUIRE(val.second == 0);
      val.second = 1;
    }
  for (auto i : map)
    CHECK(i.second.second == 1);
  free(tb);
}

void recycle_table(Item_t *tb)
{
  for (int i = 0; i < TABLE_SIZE; ++i)
    delete[] tb[i].info;
}

TEST_CASE("add values")
{
  Item_t table[TABLE_SIZE];
  memset(table, 0, TABLE_SIZE * sizeof(Item_t));
  for (auto i : std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12})
    CHECK(add_to_table(table, i, simples(i)) == i);
  CHECK(add_to_table(table, 0, ""_fs) == -1);
  recycle_table(table);
}

TEST_CASE("add similar values")
{
  Item_t table[TABLE_SIZE];
  memset(table, 0, TABLE_SIZE * sizeof(Item_t));
  for (auto i : std::vector<int>{0, 1, 0, 3, 0, 5, 0, 7, 0, 9, 1, 11, 1})
    CHECK(add_to_table(table, i, simples(i)) >= 0);
  CHECK(add_to_table(table, 0, ""_fs) == -1);
  recycle_table(table);
}

TEST_CASE("add & find similar values")
{
  Item_t table[TABLE_SIZE];
  memset(table, 0, TABLE_SIZE * sizeof(Item_t));
  auto arr = std::vector<int>{0, 1, 0, 3, 0, 5, 0, 7, 0, 9, 1, 11, 1};
  for (int i = 0; i < arr.size(); ++i)
    REQUIRE(add_to_table(table, arr[i], simples(arr[i]) + i) >= 0);
  cmp_tables(find_in_table(table, 0), {{{0, 0}, {"00", 0}},
                                       {{0, 1}, {"02", 0}},
                                       {{0, 2}, {"04", 0}},
                                       {{0, 3}, {"06", 0}},
                                       {{0, 4}, {"08", 0}}});
  cmp_tables(find_in_table(table, 1), {{{1, 0}, {"11", 0}},
                                       {{1, 1}, {"110", 0}},
                                       {{1, 2}, {"112", 0}}});

  recycle_table(table);
}