#include "priority.h"

namespace NSPropertyModel {
bool operator<(const Priority& lhs, const Priority& rhs) {
  if (lhs.status != rhs.status) {
    return lhs.status == Priority::Status::Stay;
  }
  if (lhs.status == Priority::Status::Stay)
    return lhs.strength < rhs.strength;
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
  if (priority.status == Priority::Status::Stay) {
    priority.strength = Priority::Strength{priority.strength + 1};
  } else if (priority.strength != 0) {
    priority.strength = Priority::Strength{priority.strength - 1};
  }
}

// сделать менее приоритетным на 1
void operator--(Priority& priority) {
  if (priority.status == Priority::Status::Regular) {
    priority.strength = Priority::Strength{priority.strength + 1};
  } else if (priority.strength != 0) {
    priority.strength = Priority::Strength{priority.strength - 1};
  }
}

std::ostream& operator<<(std::ostream& out, const Priority& priority) {
  switch (priority.status) {
  case Priority::Status::Stay:
    out << "Stay with strength = ";
    break;
  case Priority::Status::Regular:
    out << "Regular with strength = ";
  }

  out << priority.strength;

  return out;
}
} // namespace NSPropertyModel
