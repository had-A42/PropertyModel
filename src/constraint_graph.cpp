#include "constraint_graph.h"

namespace NSPropertyModel {

void ConstraintGraph::AddConstraint(Constraint&& constraint) {
  constraints_.push_back(std::make_unique<Constraint>(std::move(constraint)));
}

void ConstraintGraph::AddVariable(Variable&& variable) {
  variables_.push_back(std::make_unique<Variable>(std::move(variable)));
}

ConstraintGraph::Constraint*
ConstraintGraph::FindLowestPriorityBlockedConstraint() const {
  Constraint* candidate = nullptr;
  for (const auto& constraint : constraints_) {
    if (IsBlocked(constraint.get()) &&
        (candidate == nullptr || candidate->priority > constraint->priority)) {
      candidate = constraint.get();
    }
  }

  return candidate;
}

ConstraintGraph::ConstraintUPtrs& ConstraintGraph::AllConstraints() {
  return constraints_;
}

const ConstraintGraph::ConstraintUPtrs&
ConstraintGraph::AllConstraints() const {
  return constraints_;
}

ConstraintGraph::Constraint*
ConstraintGraph::ConstraintByIndex(ConstraintGraph::IndexType index) {
  return constraints_[index].get();
}
const ConstraintGraph::Constraint*
ConstraintGraph::ConstraintByIndex(ConstraintGraph::IndexType index) const {
  return constraints_[index].get();
}

ConstraintGraph::VariableUPtrs& ConstraintGraph::AllVariables() {
  return variables_;
}

const ConstraintGraph::VariableUPtrs& ConstraintGraph::AllVariables() const {
  return variables_;
}

ConstraintGraph::Variable*
ConstraintGraph::VariableByIndex(ConstraintGraph::IndexType index) {
  return variables_[index].get();
}
const ConstraintGraph::Variable*
ConstraintGraph::VariableByIndex(ConstraintGraph::IndexType index) const {
  return variables_[index].get();
}

void ConstraintGraph::InsertStayToSolution(Constraint* constraint) {
  SelectMethodByIndex(constraint, 0);
  MarkApplied(constraint);
  SetDeterminedBy(GetSelectedMethodOut(constraint), constraint);
  UpdatePriority(GetSelectedMethodOut(constraint));
}

void ConstraintGraph::ExecutePlan(StepType& propagation_counter) {
  ++propagation_counter;
  /*
   * Тут возникла проблема, что без перехода на индексы и формирования явного
   * графа решения не получается вычислить план исполнения с сохранением
   * асимптотики O(V+E)
   */
  SolutionGraphOnIndices solution(VariablesSize());
  for (const auto& constraint : AllConstraints()) {
    if (IsApplied(constraint.get())) {
      for (const auto& variable : GetInVariables(constraint->selected_method)) {
        solution[variable->global_index].push_back(constraint.get());
      }
    }
  }

  ConstraintPtrs execution_plan;
  for (const auto& constraint : constraints_) {
    if (IsApplied(constraint.get())) {
      FormExecutionPlan(solution, execution_plan, constraint.get(),
                        propagation_counter);
    }
  }

  std::reverse(execution_plan.begin(), execution_plan.end());

  for (const auto& constraint : execution_plan) {
    ExecuteConstraint(constraint);
  }
  ++propagation_counter;
}

ConstraintGraph::IndexType ConstraintGraph::ConstraintsSize() {
  return constraints_.size();
}

ConstraintGraph::IndexType ConstraintGraph::VariablesSize() {
  return variables_.size();
}

void ConstraintGraph::AttachLastAsStay(IndexType index) {
  VariableByIndex(index)->stay = constraints_.back().get();
}

void ConstraintGraph::FormExecutionPlan(SolutionGraphOnIndices& solution,
                                        ConstraintPtrs& execution_plan,
                                        Constraint* constraint,
                                        StepType& propagation_counter) {
  assert(constraint != nullptr);
  UpdateStep(constraint, propagation_counter);
  Variable* output = GetSelectedMethodOut(constraint);

  for (const auto& next_constraints : solution[output->global_index]) {
      if (IsProcessing(next_constraints, propagation_counter)) {
          throw std::runtime_error("Cycle is detected in constraint system");
      }

    if (!IsExecutedInCurrentStep(next_constraints, propagation_counter)) {
      FormExecutionPlan(solution, execution_plan, next_constraints,
                        propagation_counter);
    }
  }

  UpdateStep(constraint, propagation_counter + 1);
  execution_plan.push_back(constraint);
}
} // namespace NSPropertyModel
