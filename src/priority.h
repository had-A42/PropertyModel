#pragma once

#include <iostream>

namespace NSPropertyModel {
struct Priority {
  // Упорядочивание по численному значению силы:
  //       Stay           ||         Regular
  //   0  1  2  3 ... inf    inf ... 3  2  1  0
  // --|--|--|--|--|---|------|---|--|--|--|--|---->
  // наименее приоритетное              наиболее приоритетное
  enum class Status { Regular, Stay };
  enum Strength : int;

  Status status;
  Strength strength;
};

bool operator<(const Priority& lhs, const Priority& rhs);

bool operator==(const Priority& lhs, const Priority& rhs);

bool operator!=(const Priority& lhs, const Priority& rhs);

bool operator>(const Priority& lhs, const Priority& rhs);

bool operator<=(const Priority& lhs, const Priority& rhs);

bool operator>=(const Priority& lhs, const Priority& rhs);

void operator++(Priority& priority);

void operator--(Priority& priority);

std::ostream& operator<<(std::ostream& out, const Priority& priority);
} // namespace NSPropertyModel
