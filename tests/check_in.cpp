#include <cmath>

#include "pm.h"

namespace {
int AddDays(int check_in, int nights) {
  return check_in + nights;
}

int RemoveDays(int check_out, int nights) {
  return check_out - nights;
}

int DayDifference(int check_in, int check_out) {
  return check_out - check_in;
}

bool IsValidDates(int check_in, int nights, int check_out) {
  return check_out - check_in == nights && check_out > check_in &&
         check_in >= 0;
}

int HowMuch(bool is_valid, int nights) {
  return is_valid ? nights * 777 : -1;
}

int CheckInShouldBeZero() {
  return 0;
}

using namespace NSPropertyModel;
class Application {
  // check_in, nights, check_out
  using DataVars = Data<int, int, int>;
  // valid_dates or not
  using ValueVars = Value<bool>;
  // cost if valid, -1 if invalid
  using OutVars = Out<int>;
  using PropertyModel = PropertyModel<DataVars, ValueVars, OutVars>;
  using Builder = PropertyModel::Builder;
  using IndexType = Templates::IndexType;

public:
  Application() {
    Builder b(0, 5, 5, false, -1);

    b.AddNewConstraint(Priority::Strength{2});
    b.AddMethod<Templates::Data<2>, Templates::Data<0>, Templates::Data<1>>(
        AddDays);
    b.AddMethod<Templates::Data<0>, Templates::Data<2>, Templates::Data<1>>(
        RemoveDays);
    b.AddMethod<Templates::Data<1>, Templates::Data<0>, Templates::Data<2>>(
        DayDifference);

    b.AddNewConstraint(Priority::Strength{1});
    b.AddMethod<Templates::Value<0>, Templates::Data<0>, Templates::Data<1>,
                Templates::Data<2>>(IsValidDates);

    b.AddNewConstraint(Priority::Strength{0});
    b.AddMethod<Templates::Out<0>, Templates::Value<0>, Templates::Data<1>>(
        HowMuch);

    b.AddNewConstraint(Priority::Strength{0});
    b.AddMethod<Templates::Data<0>>(CheckInShouldBeZero);

    pm = b.ExtractPM();

      pm->Print();
  }
  void run() {
    std::string query;
    PrintIntroductoryCommand();
    while (std::cin >> query) {
      Query(query);
      PrintIntroductoryCommand();
    }
  }

private:
  static void PrintIntroductoryCommand() {
    std::cout
        << "_____________________________________________________________\n";
    std::cout
        << "Enter command (quit / update - update value / add - add "
           "constraint / remove - remove constraint / print - print system "
           "state): ";
  }

  void Query(std::string& query) {
    IndexType index;
    if (query == "quit") {
      std::cout << "The End.\n";
      exit(0);
    } else if (query == "update") {
      char meta_marker;
      std::cout << "Enter meta marker and index (D index): ";
      std::cin >> meta_marker;

      switch (meta_marker) {
      case 'D':
        std::cin >> index;
        switch (index) {
        case 0:
          int check_in;
          std::cout << "Enter check in date: ";
          std::cin >> check_in;
          pm->Set<Templates::Data<0>>(check_in);
          break;
        case 1:
          int night;
                std::cout << "Enter nights count: ";
          std::cin >> night;
          pm->Set<Templates::Data<1>>(night);
          break;
        case 2:
          int check_out;
                std::cout << "Enter check out date: ";
          std::cin >> check_out;
          pm->Set<Templates::Data<2>>(check_out);
          break;
          default:
              std::cout << "Index out of range!\n";
        }
        break;
      default:
        std::cout << "Invalid MetaData marker!\n";
      }
    } else if (query == "remove") {
      std::cout << "Enter the constraint index: ";
      std::cin >> index;
      pm->RemoveConstraint(index);
    } else if (query == "add") {
      std::cout << "Enter the constraint index: ";
      std::cin >> index;
      pm->AddConstraint(index);
    } else if (query == "print") {
      pm->Print();
    } else {
      std::cout << "Invalid command!\n";
    }
  }
  PropertyModel pm;
};
} // namespace

int main() {
    try {
        Application app;
        app.run();
    } catch(...) {
        NSExcept::react();
    }
    return 0;
}
