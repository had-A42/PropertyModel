#include "constraint_graph.h"

namespace NSPropertyModel {
ConstraintGraph::ConstraintGraph(Constraints& constraints,
                                 Variables& variables) {
  InitConstraints(constraints);
  InitVariables(variables);
};

void ConstraintGraph::InitConstraints(Constraints& constraints) {
  for (auto& constraint : constraints) {
    constraints_.push_back(constraint.get());
  }
}

void ConstraintGraph::InitVariables(Variables& variables) {
  for (auto& variable : variables) {
    variables_.push_back(variable.get());
  }
}

void ConstraintGraph::AddConstraint(Constraint* constraint) {
  //            constraints_.push_back(constraint);
  constraint->state = Constraint::State::Unused;
}

void ConstraintGraph::Clear() {
  constraints_.clear();
  variables_.clear();
}

Constraint* ConstraintGraph::GetByIndex(IndexType index) {
  return constraints_[index];
}

Constraint* ConstraintGraph::FindLowestPriorityBlockedConstraint() {
  Constraint* candidate = nullptr;
  for (auto& constraint : constraints_) {
    if (constraint->IsBlocked() &&
        (candidate == nullptr || candidate->priority > constraint->priority)) {
      candidate = constraint;
    }
  }

  return candidate;
}

ConstraintPtrs& ConstraintGraph::GetAllConstraints() {
  return constraints_;
}
} // namespace NSPropertyModel
