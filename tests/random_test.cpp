#include "../src/builder.h"
#include "../src/property_model.h"

#include <cmath>

namespace {
int f1_1(int b, int c) {
  return b + c;
}

int f1_2(int a, int b) {
  return a - b;
}

double f2(int a, double b) {
  return a * b;
}

std::string f3(double a, std::string s) {
  return s + std::to_string(a);
}

std::string f4(std::string a, std::string b) {
  return a + b[0];
}

using namespace NSPropertyModel;
using DataVars = Data<int, double, std::string>;
using ValueVars = Value<int, double, std::string>;
using OutVars = Out<int, std::string>;
using PropertyModel = PropertyModel<DataVars, ValueVars, OutVars>;
using Builder = PropertyModel::Builder;

void Query(PropertyModel& pm) {
  std::cout
      << "_____________________________________________________________\n";
  std::string query;
  IndexType index;
  std::cout << "Enter command (quit / update - update value / add - add "
               "constraint / remove - remove constraint / print - print system "
               "state): ";
  std::cin >> query;
  if (query == "quit") {
    std::cout << "The End.\n";
    exit(0);
  } else if (query == "update") {
    char meta_marker;
    IndexType index;
    std::cout << "Enter meta marker and index (D/V index): ";
    std::cin >> meta_marker;

    int value_int;
    double value_double;
    std::string value_string;
    switch (meta_marker) {
    case 'D':
      std::cin >> index;
      if (index >= 3) {
        std::cout << "Index out of range!\n";
      }
      std::cout << "Enter the value: ";
      switch (index) {
      case 0:
        std::cin >> value_int;
        pm->Set<Templates::Data<0>>(value_int);
        break;
      case 1:
        std::cin >> value_double;
        pm->Set<Templates::Data<1>>(value_double);
        break;
      case 2:
        std::cin >> value_string;
        pm->Set<Templates::Data<2>>(std::move(value_string));
        break;
      }
      break;
    case 'V':
      std::cin >> index;
      if (index >= 3) {
        std::cout << "Index out of range!\n";
      }
      std::cout << "Enter the value: ";
      switch (index) {
      case 0:
        std::cin >> value_int;
        pm->Set<Templates::Value<0>>(value_int);
        break;
      case 1:
        std::cin >> value_double;
        pm->Set<Templates::Value<1>>(value_double);
        break;
      case 2:
        std::cin >> value_string;
        pm->Set<Templates::Value<2>>(std::move(value_string));
        break;
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

void test() {
  std::string s1 = "abc";
  std::string s2 = "def";
  std::string s3 = "ijk";
  std::string s4 = "xyz";

  Builder b(3, 0.6, std::move(s1), 2, 1.6, std::move(s2), 4, std::move(s3));

  //        b.CallPrint();

  b.Set<Templates::Data<1>>(0.5);
  b.Set<Templates::Data<2>>(std::move(s4));

  int d = 5;
  b.Set<Templates::Data<0>>(d);

  //        b.CallPrint();

  b.AddNewConstraint(Priority::Strength{1});
  b.AddMethod<Templates::Out<0>, Templates::Data<0>, Templates::Value<0>>(f1_1);
  b.AddMethod<Templates::Data<0>, Templates::Out<0>, Templates::Value<0>>(f1_2);
  b.AddMethod<Templates::Value<0>, Templates::Out<0>, Templates::Data<0>>(f1_2);

  b.AddNewConstraint(Priority::Strength{2});
  b.AddMethod<Templates::Value<1>, Templates::Data<0>, Templates::Data<1>>(f2);

  b.AddNewConstraint(Priority::Strength{3});
  b.AddMethod<Templates::Value<2>, Templates::Data<1>, Templates::Data<2>>(f3);

  //        b.CallPrint();

  b.AddNewConstraint(Priority::Strength{0});
  b.AddMethod<Templates::Out<1>, Templates::Value<2>, Templates::Data<2>>(f4);

  auto pm = b.ExtractPM();

  pm->Print();

  while (true) {
    Query(pm);
  }
}
} // namespace

int main() {
  test();
}
