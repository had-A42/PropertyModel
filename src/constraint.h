#pragma once

#include <cassert>
#include <functional>

#include "priority.h"
#include "templates.h"

namespace NSPropertyModel {

struct Variable;

struct Method {
  void Execute();

  Variable* GetOut();
  const Variable* GetOut() const;

  Priority GetOutPriority();

  std::function<void()> action;

  std::vector<Variable*> in;
  std::vector<Variable*> out;
};

std::ostream& operator<<(std::ostream& out, const Method& method);

class Constraint {
    using IndexType = Templates::IndexType;
    using StepType = Templates::StepType;
  static constexpr Priority min_stay_priority = {Priority::Status::Stay,
                                                 Priority::Strength{0}};
  static constexpr Priority max_regular_priority = {Priority::Status::Regular,
                                                    Priority::Strength{0}};

public:
  Constraint(Priority priority = max_regular_priority);

  Constraint(Priority priority, std::vector<std::unique_ptr<Method>> methods);

  void PushBackMethod(std::unique_ptr<Method> method);

  void UpdateStep(StepType propagation_index);

  bool IsExecutedInCurrentStep(StepType current_step);
  bool IsProcessing(StepType current_step);

  void Execute();

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

  // private:
  enum class State { Applied, Unused, Disabled };

  State state = State::Disabled;
  Method* selected_method = nullptr;
  Priority priority;
  StepType last_execution = StepType{0};
  std::vector<std::unique_ptr<Method>> methods;
};

std::ostream& operator<<(std::ostream& out, const Constraint::State& state);

std::ostream& operator<<(std::ostream& out, const Constraint& constraint);
} // namespace NSPropertyModel
