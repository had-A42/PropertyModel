#pragma once

#include "constraint.h"
#include "priority.h"
#include "templates.h"
#include "variable.h"

namespace NSPropertyModel {
class ConstraintGraph {
  using IndexType = Templates::IndexType;
  using StepType = Templates::StepType;

  using Variable = detail::Variable;
  using Method = detail::Method;
  using Constraint = detail::Constraint;

  using ConstraintUPtrs = std::vector<std::unique_ptr<Constraint>>;
  using VariableUPtrs = std::vector<std::unique_ptr<Variable>>;

  using ConstraintPtrs = std::vector<Constraint*>;
  using SolutionGraphOnIndices = std::vector<std::vector<Constraint*>>;

public:
  ConstraintGraph() = default;

  void AddConstraint(Constraint&& constraint);
  void AddVariable(Variable&& variable);

  [[nodiscard]] Constraint* FindHighestPriorityBlockedConstraint() const;
  static void InsertStayToSolution(Constraint* constraint);
  void ExecutePlan(StepType& propagation_counter);

  ConstraintUPtrs& AllConstraints();
  [[nodiscard]] const ConstraintUPtrs& AllConstraints() const;

  Constraint* ConstraintByIndex(IndexType index);
  [[nodiscard]] const Constraint* ConstraintByIndex(IndexType index) const;

  VariableUPtrs& AllVariables();
  [[nodiscard]] const VariableUPtrs& AllVariables() const;

  Variable* VariableByIndex(IndexType index);
  [[nodiscard]] const Variable* VariableByIndex(IndexType index) const;

  IndexType ConstraintsSize();
  IndexType VariablesSize();

  void AttachLastAsStay(IndexType index);

private:
  void FormExecutionPlan(SolutionGraphOnIndices& solution,
                         ConstraintPtrs& execution_plan, Constraint* constraint,
                         StepType& propagation_counter);

  ConstraintUPtrs constraints_;
  VariableUPtrs variables_;
};
} // namespace NSPropertyModel
