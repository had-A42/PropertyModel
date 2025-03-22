#include "constraint.h"
#include "variable.h"

namespace NSPropertyModel {
void Method::Execute() {
  action();
};

Variable* Method::GetOut() {
  return out[0];
}

Constraint::Constraint(Priority priority) : priority(priority) {};

Constraint::Constraint(Priority priority,
                       std::vector<std::unique_ptr<Method>> methods)
    : priority(priority), methods(std::move(methods)) {};

void Constraint::PushBackMethod(std::unique_ptr<Method> method) {
  methods.push_back(std::move(method));
}

auto Constraint::operator[](IndexType index) {
  return methods[index].get();
}

const auto Constraint::operator[](IndexType index) const {
  return methods[index].get();
}

bool Constraint::IsStay() {
  return priority.status == NSPropertyModel::Priority::Status::Stay;
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
  return priority.strength == 0;
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
  NSPropertyModel::Priority min_priority = {
      NSPropertyModel::Priority::Status::Regular,
      NSPropertyModel::Priority::Strength{0}};
  Method* min_priority_method = nullptr;
  for (auto& method : methods) {
    if (method->GetOut() != variable &&
        min_priority > method->GetOut()->priority) {
      min_priority = method->GetOut()->priority;
      min_priority_method = method.get();
    }
  }

  return min_priority_method;
}

Priority
Constraint::PotentialOutputsMinPriority(NSPropertyModel::Variable* variable) {
  Method* method = PotentialOutputsMinMethod(variable);
  if (method == nullptr) {
    return Priority{Priority{Priority::Status::Regular, Priority::Strength{0}}};
  } else {
    return method->GetOut()->priority;
  }
}

Method* Constraint::OutputMinPriorityMethod() {
  NSPropertyModel::Priority min_priority = {
      NSPropertyModel::Priority::Status::Regular,
      NSPropertyModel::Priority::Strength{0}};
  Method* min_priority_method = nullptr;
  for (auto& method : methods) {
    if (min_priority > method->GetOut()->priority) {
      min_priority = method->GetOut()->priority;
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
enum class State { Applied, Unused, Disabled };

std::ostream& operator<<(std::ostream& out, const Constraint& constraint) {
  switch (constraint.priority.status) {
  case Priority::Status::Stay:
    out << "Stay";
    break;
  case Priority::Status::Regular:
    out << "Regular";
  }

  out << " with strength = " << constraint.priority.strength
      << "; method count = " << constraint.methods.size() << " ; Now is ";

  switch (constraint.state) {
  case Constraint::State::Applied:
    out << "Applied";
    break;
  case Constraint::State::Unused:
    out << "Unused";
    break;
  case Constraint::State::Disabled:
    out << "Disabled";
    break;
  }

  out << ", selected_metod = " << constraint.selected_method << " ";

  if (constraint.selected_method) {
    out << "determining Variable ";
    switch (constraint.selected_method->GetOut()->type) {
    case Variable::Type::Data:
      out << "Data<"
          << static_cast<int>(constraint.selected_method->GetOut()->index)
          << ">";
      break;
    case Variable::Type::Value:
      out << "Value<"
          << static_cast<int>(constraint.selected_method->GetOut()->index)
          << ">";
      break;
    case Variable::Type::Out:
      out << "Out<"
          << static_cast<int>(constraint.selected_method->GetOut()->index)
          << ">";
    }
  }
  std::cout << "\n";

  return out;
}

} // namespace NSPropertyModel
