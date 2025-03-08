struct Priopity {
  bool is_stay;
  int value;

  bool operator<(const Priopity& other) const {
    if (is_stay != other.is_stay) {
      return is_stay > other.is_stay;
    }
    return value < other.value;
  }

  bool operator==(const Priopity& other) const {
    return (is_stay == other.is_stay) && (value == other.value);
  }

  bool operator!=(const Priopity& other) const {
    return !(*this == other);
  }

  bool operator>(const Priopity& other) const {
    return other < *this;
  }

  bool operator<=(const Priopity& other) const {
    return !(*this > other);
  }

  bool operator>=(const Priopity& other) const {
    return !(*this < other);
  }
};
