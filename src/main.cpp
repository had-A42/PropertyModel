#include "builder.h"
#include "property_model.h"

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
} // namespace

using namespace NSPropertyModel;

int main() {
  using DataVars = Data<int, double, std::string>;
  using ValueVars = Value<int, double, std::string>;
  using OutVars = Out<int, std::string>;
  using PropertyModel = PropertyModel<DataVars, ValueVars, OutVars>;
  using Builder = PropertyModel::Builder;

  std::string s1 = "abc";
  std::string s2 = "def";
  std::string s3 = "ijk";
  std::string s4 = "xyz";

  Builder b(3, 0.6, std::move(s1), 2, 1.6, std::move(s2), 4, std::move(s3));

  //  std::cout << "Строка: " << s << "\n";

  b.CallPrint();

  b.Set<Templates::Data<1>>(0.5);

  b.Set<Templates::Data<2>>(std::move(s4));

  int d = 5;
  b.Set<Templates::Data<0>>(d);

  b.CallPrint();

  b.AddMethod<Templates::Out<0>, Templates::Data<0>, Templates::Value<0>>(f1_1);
  b.AddMethod<Templates::Data<0>, Templates::Out<0>, Templates::Value<0>>(f1_2);
  b.AddMethod<Templates::Value<0>, Templates::Out<0>, Templates::Data<0>>(f1_2);
  b.AddNewConstraint(
      Priority{Priority::Status::Regular, Priority::Strength{1}});

  b.AddMethod<Templates::Value<1>, Templates::Data<0>, Templates::Data<1>>(f2);
  b.AddNewConstraint(
      Priority{Priority::Status::Regular, Priority::Strength{2}});

  b.AddMethod<Templates::Value<2>, Templates::Data<1>, Templates::Data<2>>(f3);
  b.AddNewConstraint(
      Priority{Priority::Status::Regular, Priority::Strength{3}});

  b.CallPrint();

  b.AddMethod<Templates::Out<1>, Templates::Value<2>, Templates::Data<2>>(f4);

  b.AddNewConstraint();

  auto pm = b.ExtractPM();

  pm->Print();

  char query;
  IndexType index;
  while (true) {
    std::cout << "Enter command (q - quit, u - update value, a - add "
                 "constraint, r - remove constraint, c - check system): ";
    std::cin >> query;
    switch (query) {
    case 'q':
      std::cout << "The End.\n";
      break;
    case 'u':
      pm->Update();
      break;
    case 'r':
      std::cout << "Enter the constraint index: ";
      std::cin >> index;
      pm->RemoveConstraint(index);
      break;
    case 'a':
      std::cout << "Enter the constraint index: ";
      std::cin >> index;
      pm->AddConstraint(index);
      break;
    case 'c':
      pm->Print();
      break;
    default:
      std::cout << "Invalid command!\n";
    }
    std::cout
        << "_____________________________________________________________\n";
  }
  return 0;
}
