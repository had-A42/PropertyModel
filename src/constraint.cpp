#include "constraint.h"
#include "variable.h"

namespace NSPropertyModel::detail {
void ExecuteMethod(const Method* method) {
  method->action();
};

Variable* GetOut(Method* method) {
  return method->out[0];
}

const Variable* GetOut(const Method* method) {
  assert(method != nullptr);
  return method->out[0];
}

const Priority GetOutPriority(const Method* method) {
    if (method == nullptr) {
        return Constraint::max_regular_priority;
    } else {
        return GetOut(method)->priority;
    }
}

const Method::VariablePtrs& GetInVariables(const Method* method) {
  assert(method != nullptr);
  return method->in;
}

std::ostream& operator<<(std::ostream& out, const Method& method) {
  std::cout << "has out " << *method.out[0];
  return out;
}

Constraint::Constraint(Priority priority) : priority(priority) {};

Constraint::Constraint(Priority priority,
                       std::vector<std::unique_ptr<Method>> methods)
    : priority(priority), methods(std::move(methods)) {};

void PushBackMethod(Constraint* constraint, std::unique_ptr<Method> method) {
  assert(constraint != nullptr);
  constraint->methods.push_back(std::move(method));
}

void UpdateStep(Constraint* constraint,
                Constraint::StepType propagation_index) {
  assert(constraint != nullptr);
  constraint->last_execution = propagation_index;
}

bool IsExecutedInCurrentStep(const Constraint* constraint,
                             Constraint::StepType current_step) {
  assert(constraint != nullptr);
  return current_step + 1 == constraint->last_execution;
}

bool IsProcessing(const Constraint* constraint,
                  Constraint::StepType current_step) {
  assert(constraint != nullptr);
  return current_step == constraint->last_execution;
}

void ExecuteConstraint(const Constraint* constraint) {
  assert(constraint != nullptr);
  assert(constraint->selected_method != nullptr);
  ExecuteMethod(constraint->selected_method);
}

Variable* GetSelectedMethodOut(const Constraint* constraint) {
  assert(constraint != nullptr);
  assert(constraint->selected_method != nullptr);
  return GetOut(constraint->selected_method);
}

void SetSelectedMethodNull(Constraint* constraint) {
  assert(constraint != nullptr);
  constraint->selected_method = nullptr;
}

void SelectMethod(Constraint* constraint, Method* method) {
  assert(constraint != nullptr);
  constraint->selected_method = method;
}

void SelectMethodByIndex(Constraint* constraint, Constraint::IndexType index) {
  assert(constraint != nullptr);
  constraint->selected_method = constraint->methods[index].get();
}

bool IsStay(const Constraint* constraint) {
  assert(constraint != nullptr);
  return constraint->priority.status == Priority::Status::Stay;
}

bool IsBlocked(const Constraint* constraint) {
  assert(constraint != nullptr);
  return (constraint->state == Constraint::State::Unused) &&
         (OutputMinPriorityVariable(constraint)->priority <
          constraint->priority);
}

bool IsRequired(const Constraint* constraint) {
  assert(constraint != nullptr);
  return constraint->priority.strength == 0 &&
         constraint->priority.status == Priority::Status::Regular;
}

bool IsReversiblePathSource(const Constraint* constraint) {
  assert(constraint != nullptr);
  return constraint->priority == GetSelectedMethodOut(constraint)->priority;
}

bool IsApplied(const Constraint* constraint) {
  assert(constraint != nullptr);
  return constraint->state == Constraint::State::Applied;
};

bool IsDisable(const Constraint* constraint) {
  assert(constraint != nullptr);
  return constraint->state == Constraint::State::Disabled;
};

bool IsUnused(const Constraint* constraint) {
  assert(constraint != nullptr);
  return constraint->state == Constraint::State::Unused;
};

Method* PotentialOutputsMinMethod(const Constraint* constraint,
                                  const Variable* variable) {
  assert(constraint != nullptr);
  assert(variable != nullptr);
  Priority min_priority = Constraint::max_regular_priority;
  Method* min_priority_method = nullptr;
  for (const auto& method : constraint->methods) {
    Method* method_ptr = method.get();
    if (GetOut(method_ptr) != variable &&
        min_priority > GetOutPriority(method_ptr)) {
      min_priority = GetOutPriority(method_ptr);
      min_priority_method = method.get();
    }
  }

  return min_priority_method;
}

Priority PotentialOutputsMinPriority(const Constraint* constraint,
                                     const Variable* variable) {
  assert(constraint != nullptr);
  assert(variable != nullptr);
  const Method* method = PotentialOutputsMinMethod(constraint, variable);
  return GetOutPriority(method);
}

Method* OutputMinPriorityMethod(const Constraint* constraint) {
  assert(constraint != nullptr);
  Priority min_priority = Constraint::max_regular_priority;
  Method* min_priority_method = nullptr;
  for (const auto& method : constraint->methods) {
    Method* method_ptr = method.get();
    if (min_priority >= GetOutPriority(method_ptr)) {
      min_priority = GetOutPriority(method_ptr);
      min_priority_method = method.get();
    }
  }
  return min_priority_method;
}

Variable* OutputMinPriorityVariable(const Constraint* constraint) {
  assert(constraint != nullptr);
  return GetOut(OutputMinPriorityMethod(constraint));
}

void MarkApplied(Constraint* constraint) {
  assert(constraint != nullptr);
  constraint->state = Constraint::State::Applied;
}

void MarkUnused(Constraint* constraint) {
  assert(constraint != nullptr);
  assert(constraint->selected_method == nullptr);
  constraint->state = Constraint::State::Unused;
}

void MarkDisabled(Constraint* constraint) {
  assert(constraint != nullptr);
  assert(constraint->selected_method == nullptr);
  constraint->state = Constraint::State::Disabled;
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
} // namespace NSPropertyModel::detail
