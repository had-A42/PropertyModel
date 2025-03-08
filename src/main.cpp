#include "property_model.h"

#include <cmath>

int f1(int b, int c) {
  return b + c;
}

int f2(int a, double b) {
  return a * std::ceil(b);
}

double f3(int a, double d) {
  return std::ceil(d) + a;
}

int main() {
  PropertyModel<std::tuple<int, double>, std::tuple<int, double>,
                std::tuple<int>>::Builder b(3, 0.6, 2, 1.6, 4);

  b.CallPrint();

  b.Set<Data<1>>(0.5);

  b.CallPrint();

  b.AddMethod<Outs<0>, Data<0>, Value<0>>(f1);
  b.AddMethod<Value<0>, Data<0>, Data<1>>(f2);
  b.AddMethod<Value<1>, Data<0>, Data<1>>(f3);
  b.AddNewConstraint();

  b.CallPrint();

  b.AddNewConstraint();

  auto pm = b.GetPM();

  pm->Print();

  return 0;
}