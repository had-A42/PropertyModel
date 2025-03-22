#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "../src/constraint.h"
#include "../src/constraint_graph.h"
#include "../src/priority.h"
#include "../src/templates.h"
#include "../src/variable.h"

namespace NSDeltaBlue {
using IndexType = Templates::IndexType;
using StepType = NSPropertyModel::StepType;

using ConstraintGraph = NSPropertyModel::ConstraintGraph;
using Constraint = NSPropertyModel::Constraint;
using Method = NSPropertyModel::Method;
using Variable = NSPropertyModel::Variable;

using Constraint = NSPropertyModel::Constraint;
using Variable = NSPropertyModel::Variable;

using Constraints = std::vector<std::unique_ptr<Constraint>>;
using Variables = std::vector<std::unique_ptr<Variable>>;

class DeltaBlue {
public:
  static void Initialise(Constraints& constraints, Variables& variables,
                         ConstraintGraph& c_graph,
                         StepType& propagation_counter);

  static void AddConstraint(ConstraintGraph& c_graph,
                            Constraint* new_constraint,
                            StepType& propagation_counter);

  static void AddConstraintByIndex(ConstraintGraph& c_graph, IndexType index,
                                   StepType& propagation_counter);

  static void RemoveConstraint(ConstraintGraph& c_graph,
                               Constraint* constraint_to_remove,
                               StepType& propagation_counter);

  static void RemoveConstraintByIndex(ConstraintGraph& c_graph, IndexType index,
                                      StepType& propagation_counter);

  static void UpdateStayPriority(ConstraintGraph& c_graph, Constraint* stay,
                                 NSPropertyModel::Priority priority,
                                 StepType& propagation_counter);

private:
  static void UpdatingPropagation(Variable* variable,
                                  StepType& propagation_counter);

  static void UpdatingPropagationImpl(Variable* variable,
                                      StepType& propagation_counter);

  static void ReversePath(Variable* variable);
};
} // namespace NSDeltaBlue
