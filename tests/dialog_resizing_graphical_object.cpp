#include <iostream>

#include "../src/builder.h"
#include "../src/property_model.h"

namespace {
double AbsFromRelAndInit(double rel, double init) {
  return rel * init / 100;
}

double RelFromAbsAndInit(double abs, double init) {
  return abs * 100 / init;
}

double ReturnRel(double rel) {
  return rel;
}

std::string CreateOutput(double abs_height, double abs_width) {
  return std::to_string(abs_height) + " " + std::to_string(abs_width);
}

using namespace NSPropertyModel;
using DataVars = Data<double, double>;
using ValueVars = Value<double, double, double, double>;
using OutVars = Out<std::string>;
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

    switch (meta_marker) {
    case 'D':
      std::cin >> index;
      if (index >= 2) {
        std::cout << "Index out of range!\n";
      }
      std::cout << "Enter the value: ";
      double value;
      switch (index) {
      case 0:
        std::cin >> value;
        pm->Set<Templates::Data<0>>(value);
        break;
      case 1:
        std::cin >> value;
        pm->Set<Templates::Data<1>>(value);
        break;
      }
      break;
    case 'V':
      std::cin >> index;
      if (index >= 4) {
        std::cout << "Index out of range!\n";
      }
      std::cout << "Enter the value: ";
      switch (index) {
      case 0:
        std::cin >> value;
        pm->Set<Templates::Value<0>>(value);
        break;
      case 1:
        std::cin >> value;
        pm->Set<Templates::Value<1>>(value);
        break;
      case 2:
        std::cin >> value;
        pm->Set<Templates::Value<2>>(value);
        break;
      case 3:
        std::cin >> value;
        pm->Set<Templates::Value<3>>(value);
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

  Builder b(1500, 2100, 1500, 2100, 100, 100, "");

  b.CallPrint();

  b.AddNewConstraint(Priority::Strength{1});
  b.AddMethod<Templates::Value<0>, Templates::Value<2>, Templates::Data<0>>(
      AbsFromRelAndInit);
  b.AddMethod<Templates::Value<2>, Templates::Value<0>, Templates::Data<0>>(
      RelFromAbsAndInit);

  b.AddNewConstraint(Priority::Strength{2});
  b.AddMethod<Templates::Value<1>, Templates::Value<3>, Templates::Data<1>>(
      AbsFromRelAndInit);
  b.AddMethod<Templates::Value<3>, Templates::Value<1>, Templates::Data<1>>(
      RelFromAbsAndInit);

  b.AddNewConstraint(Priority::Strength{3});
  b.AddMethod<Templates::Value<2>, Templates::Value<3>>(ReturnRel);
  b.AddMethod<Templates::Value<3>, Templates::Value<2>>(ReturnRel);

  b.AddNewConstraint(Priority::Strength{0});
  b.AddMethod<Templates::Out<0>, Templates::Value<0>, Templates::Value<1>>(
      CreateOutput);

  PropertyModel pm = b.ExtractPM();

  while (true) {
    Query(pm);
  }
}

} // namespace

int main() {
  test();
}
