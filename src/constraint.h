#pragma once

#include <cassert>
#include <functional>

#include "priority.h"
#include "templates.h"

namespace NSPropertyModel::detail {

struct Variable;

struct Method {
  using VariablePtrs = std::vector<Variable*>;

  std::function<void()> action;
  VariablePtrs in;
  VariablePtrs out;
};

void ExecuteMethod(const Method* method);

Variable* GetOut(Method* method);
const Variable* GetOut(const Method* method);

Priority GetOutPriority(const Method* method);
const Method::VariablePtrs& GetInVariables(const Method* method);

std::ostream& operator<<(std::ostream& out, const Method& method);

struct Constraint {
  static constexpr Priority min_stay_priority = {Priority::Status::Stay,
                                                 Priority::Strength{0}};
  static constexpr Priority max_regular_priority = {Priority::Status::Regular,
                                                    Priority::Strength{0}};

  using IndexType = Templates::IndexType;
  using StepType = Templates::StepType;

  enum class State { Applied, Unused, Disabled };

  State state = State::Disabled;
  Method* selected_method = nullptr;
  Priority priority;
  StepType last_execution = StepType{0};
  std::vector<std::unique_ptr<Method>> methods;
};

std::ostream& operator<<(std::ostream& out, const Constraint::State& state);
std::ostream& operator<<(std::ostream& out, const Constraint& constraint);

void PushBackMethod(Constraint* constraint, std::unique_ptr<Method> method);

void UpdateStep(Constraint* constraint, Constraint::StepType propagation_index);

bool IsExecutedInCurrentStep(const Constraint* constraint,
                             Constraint::StepType current_step);
bool IsProcessing(const Constraint* constraint,
                  Constraint::StepType current_step);

void ExecuteConstraint(const Constraint* constraint);

Variable* GetSelectedMethodOut(const Constraint* constraint);

void SetSelectedMethodNull(Constraint* constraint);
void SelectMethod(Constraint* constraint, Method* method);
void SelectMethodByIndex(Constraint* constraint, Constraint::IndexType index);

bool IsStay(const Constraint* constraint);
bool IsBlocked(const Constraint* constraint);
bool IsRequired(const Constraint* constraint);
bool IsReversiblePathSource(const Constraint* constraint);

bool IsApplied(const Constraint* constraint);
bool IsDisable(const Constraint* constraint);
bool IsUnused(const Constraint* constraint);

Method* PotentialOutputsMinMethod(const Constraint* constraint,
                                  const Variable* variable);
Priority PotentialOutputsMinPriority(const Constraint* constraint,
                                     const Variable* variable);
Method* OutputMinPriorityMethod(const Constraint* constraint);
Variable* OutputMinPriorityVariable(const Constraint* constraint);

void MarkApplied(Constraint* constraint);
void MarkUnused(Constraint* constraint);
void MarkDisabled(Constraint* constraint);
} // namespace NSPropertyModel::detail
