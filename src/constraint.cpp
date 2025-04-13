#include "constraint.h"
#include "variable.h"

namespace NSPropertyModel {
void Method::Execute() {
  action();
};

Variable* Method::GetOut() {
  return out[0];
}

const Variable* Method::GetOut() const {
  return out[0];
}

Priority Method::GetOutPriority() {
  return GetOut()->priority;
}
Method::VariablePtrs& Method::GetInVariables() {
  return in;
}

std::ostream& operator<<(std::ostream& out, const Method& method) {
  std::cout << "has out " << *method.GetOut();
  return out;
}

Constraint::Constraint(Priority priority) : priority(priority) {};

Constraint::Constraint(Priority priority,
                       std::vector<std::unique_ptr<Method>> methods)
    : priority(priority), methods(std::move(methods)) {};

void Constraint::PushBackMethod(std::unique_ptr<Method> method) {
  methods.push_back(std::move(method));
}

void Constraint::UpdateStep(StepType propagation_index) {
  last_execution = propagation_index;
}

bool Constraint::IsExecutedInCurrentStep(StepType current_step) {
  return current_step + 1 == last_execution;
}

bool Constraint::IsProcessing(StepType current_step) {
  return current_step == last_execution;
}

void Constraint::Execute() {
  assert(selected_method != nullptr);
  selected_method->Execute();
}

bool Constraint::IsStay() {
  return priority.status == Priority::Status::Stay;
}

bool Constraint::IsBlocked() {
  return (state == State::Unused) &&
         (OutputMinPriorityVariable()->priority < priority);
}

Variable* Constraint::GetSelectedMethodOut() {
  assert(selected_method != nullptr);
  return selected_method->GetOut();
}

bool Constraint::IsRequired() {
  return priority.strength == 0 && priority.status == Priority::Status::Regular;
}

bool Constraint::IsReversiblePathSource() {
  return priority == GetSelectedMethodOut()->priority;
}

bool Constraint::IsApplied() {
  return state == State::Applied;
};

bool Constraint::IsDisable() {
  return state == State::Disabled;
};

bool Constraint::IsUnused() {
  return state == State::Unused;
};

Method* Constraint::PotentialOutputsMinMethod(Variable* variable) {
  Priority min_priority = max_regular_priority;
  Method* min_priority_method = nullptr;
  for (const auto& method : methods) {
    if (method->GetOut() != variable &&
        min_priority > method->GetOutPriority()) {
      min_priority = method->GetOutPriority();
      min_priority_method = method.get();
    }
  }

  return min_priority_method;
}

Priority Constraint::PotentialOutputsMinPriority(Variable* variable) {
  Method* method = PotentialOutputsMinMethod(variable);
  if (method == nullptr) {
    return max_regular_priority;
  } else {
    return method->GetOutPriority();
  }
}

Method* Constraint::OutputMinPriorityMethod() {
  Priority min_priority = max_regular_priority;
  Method* min_priority_method = nullptr;
  for (const auto& method : methods) {
    if (min_priority >= method->GetOutPriority()) {
      min_priority = method->GetOutPriority();
      min_priority_method = method.get();
    }
  }
  return min_priority_method;
}

Variable* Constraint::OutputMinPriorityVariable() {
  return OutputMinPriorityMethod()->GetOut();
}

void Constraint::SelectMethod(Method* constraint) {
  selected_method = constraint;
}

void Constraint::SelectMethodByIndex(IndexType index) {
  selected_method = methods[index].get();
}

void Constraint::MarkApplied() {
  state = State::Applied;
}

void Constraint::MarkUnused() {
  assert(selected_method == nullptr);
  state = State::Unused;
}

void Constraint::MarkDisabled() {
  assert(selected_method == nullptr);
  state = State::Disabled;
}

void Constraint::SetSelectedMethodNull() {
  selected_method = nullptr;
}

std::ostream& operator<<(std::ostream& out, const Constraint::State& state) {
  switch (state) {
  case Constraint::State::Applied:
    out << "Applied";
    break;
  case Constraint::State::Unused:
    out << "Unused";
    break;
  case Constraint::State::Disabled:
    out << "Disabled";
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, const Constraint& constraint) {
  out << constraint.priority << ". ";
  out << constraint.state << ". ";

  if (constraint.selected_method) {
    out << "Selected_metod " << *constraint.selected_method << " ";
  }

  out << "\n";
  return out;
}
} // namespace NSPropertyModel
