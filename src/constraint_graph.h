#pragma once

#include "constraint.h"
#include "priority.h"
#include "templates.h"
#include "variable.h"

namespace NSPropertyModel {
class ConstraintGraph {
    using IndexType = Templates::IndexType;
    using StepType = Templates::StepType;

  using Constraints = std::vector<std::unique_ptr<Constraint>>;
  using Variables = std::vector<std::unique_ptr<Variable>>;

  using ConstraintPtrs = std::vector<Constraint*>;
  using VariablesPtrs = std::vector<Variable*>;

  using SolutionGraphOnIndices = std::vector<std::vector<Constraint*>>;

public:
  ConstraintGraph() = default;

  ConstraintGraph(Constraints& constraints, Variables& variables);

  void InitConstraints(Constraints& constraints);
  void InitVariables(Variables& variables);

  //  void AddConstraint(Constraint* constraint);

  void Clear();

  Constraint* GetByIndex(IndexType index);
  Constraint* FindLowestPriorityBlockedConstraint();

  ConstraintPtrs& GetAllConstraints();

  void ExecutePlan(StepType& propagation_counter);

private:
  void FormExecutionPlan(SolutionGraphOnIndices& solution,
                         ConstraintPtrs& execution_plan,
                         StepType& propagation_counter, Constraint* constraint);

  ConstraintPtrs constraints_;
  VariablesPtrs variables_;
};
} // namespace NSPropertyModel
