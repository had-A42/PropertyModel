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

void ConstraintGraph::ExecutePlan(StepType& propagation_counter) {
  ++propagation_counter;
  /*
   * Тут возникла проблема, что без перехода на индексы и формирования явного
   * графа решения не получается вычислить план исполнения с сохранением
   * асимптотики O(V+E)
   */
  SolutionGraphOnIndices solution(variables_.size());
  for (auto constraint : constraints_) {
    if (constraint->IsApplied()) {
      for (auto variable : constraint->selected_method->in) {
        solution[variable->global_index].push_back(constraint);
      }
    }
  }

  std::vector<Constraint*> execution_plan;
  for (auto constraint : constraints_) {
    if (constraint->IsApplied()) {
      FormExecutionPlan(solution, execution_plan, propagation_counter,
                        constraint);
    }
  }

  std::reverse(execution_plan.begin(), execution_plan.end());

  for (auto constraint : execution_plan) {
    constraint->Execute();
  }
  ++propagation_counter;
}

void ConstraintGraph::FormExecutionPlan(
    SolutionGraphOnIndices& solution, std::vector<Constraint*>& execution_plan,
    StepType& propagation_counter, Constraint* constraint) {
  constraint->UpdateStep(propagation_counter);
  Variable* output = constraint->GetSelectedMethodOut();
  for (auto next_constraints : solution[output->global_index]) {
    assert(!next_constraints->IsProcessing(propagation_counter));
    if (!next_constraints->IsExecutedInCurrentStep(propagation_counter)) {
      FormExecutionPlan(solution, execution_plan, propagation_counter,
                        next_constraints);
    }
  }
  constraint->UpdateStep(propagation_counter + 1);
  execution_plan.push_back(constraint);
}
} // namespace NSPropertyModel
