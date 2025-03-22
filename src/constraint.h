#pragma once

#include <cassert>
#include <functional>

#include "priority.h"
#include "templates.h"

namespace NSPropertyModel {
using IndexType = Templates::IndexType;
// static constexpr int k_non_ = -1;

struct Variable;

struct Method {
  void Execute();

  Variable* GetOut();

  std::function<void()> action;

  std::vector<Variable*> in;
  std::vector<Variable*> out;
};

struct Constraint {
  Constraint(Priority priority = Priority{Priority::Status::Regular,
                                          Priority::Strength{0}});

  Constraint(Priority priority, std::vector<std::unique_ptr<Method>> methods);

  auto operator[](IndexType index);

  const auto operator[](IndexType index) const;

  void PushBackMethod(std::unique_ptr<Method> method);

  Variable* GetSelectedMethodOut();
  void SetSelectedMethodNull();
  void SelectMethod(Method* constraint);
  void SelectMethodByIndex(IndexType index);

  bool IsStay();
  bool IsBlocked();
  bool IsRequired();
  bool IsReversiblePathSource();

  bool IsApplied();
  bool IsDisable();
  bool IsUnused();

  Method* PotentialOutputsMinMethod(Variable* variable);
  Priority PotentialOutputsMinPriority(Variable* variable);
  Method* OutputMinPriorityMethod();
  Variable* OutputMinPriorityVariable();

  void MarkApplied();
  void MarkUnused();
  void MarkDisabled();

  enum class State { Applied, Unused, Disabled };
  std::vector<std::unique_ptr<Method>> methods;
  State state = State::Disabled;
  Method* selected_method = nullptr;
  NSPropertyModel::Priority priority;
  //    std::vector<Method*> methods;
  //        std::vector<Method> methods;
};

std::ostream& operator<<(std::ostream& out, const Constraint& constraint);
} // namespace NSPropertyModel
