#include "priority.h"

namespace NSPropertyModel {
//       Stay        ||         Regular
//  -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6
// --|--|--|--|--|--|--|--|--|--|--|--|--|--|-->
//
// Упорядочивание по численному значению силы:
// -1 -2 -3 -4 ... -inf inf ... 4 3 2 1 0
bool operator<(const Priority& lhs, const Priority& rhs) {
  if (lhs.status != rhs.status) {
    return lhs.status == Priority::Status::Stay;
  }
  return lhs.strength > rhs.strength;
}

bool operator==(const Priority& lhs, const Priority& rhs) {
  return (lhs.status == rhs.status) && (lhs.strength == rhs.strength);
}

bool operator!=(const Priority& lhs, const Priority& rhs) {
  return !(lhs == rhs);
}

bool operator>(const Priority& lhs, const Priority& rhs) {
  return rhs < lhs;
}

bool operator<=(const Priority& lhs, const Priority& rhs) {
  return !(lhs > rhs);
}

bool operator>=(const Priority& lhs, const Priority& rhs) {
  return !(lhs < rhs);
}

// сделать приоритетнее на 1
void operator++(Priority& priority) {
  priority.strength = Priority::Strength{priority.strength - 1};
}

// сделать менее приоритетным на 1
void operator--(Priority& priority) {
  priority.strength = Priority::Strength{priority.strength + 1};
}

std::ostream& operator<<(std::ostream& out, const Priority& priority) {
  switch (priority.status) {
  case Priority::Status::Stay:
    out << "Stay ";
    break;
  case Priority::Status::Regular:
    out << "Regular with strength = ";
  }

  out << priority.strength;

  return out;
}
} // namespace NSPropertyModel
