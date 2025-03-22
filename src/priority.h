#pragma once

#include <iostream>

namespace NSPropertyModel {
struct Priority {
  enum class Status { Regular, Stay };
  Status status;
  enum Strength : int;
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
