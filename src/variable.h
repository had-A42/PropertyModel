#pragma once

#include "priority.h"
#include "templates.h"

namespace NSPropertyModel {
class Constraint;

struct Variable {
  using IndexType = Templates::IndexType;
  using StepType = Templates::StepType;
  using ConstraintPtrs = std::vector<Constraint*>;

  enum class Type { Data, Value, Out };

  Variable(Type type, IndexType index, IndexType global_index);

  void UpdatePriority();
  void UpdateStep(StepType propagation_index);

  void SetDeterminedBy(Constraint* constraint);
  void SetDeterminedByNull();

  Constraint* GetStay();

  bool IsUpdatedInCurrentStep(StepType current_step);
  bool IsProcessing(StepType current_step);

  Type type;
  IndexType index;
  IndexType global_index;

  StepType last_propagation = StepType{0};
  Priority priority;
  Constraint* determined_by = nullptr;
  Constraint* stay;
  ConstraintPtrs involved_as_potential_output;
};

std::ostream& operator<<(std::ostream& out, const Variable::Type& type);
std::ostream& operator<<(std::ostream& out, const Variable& variable);
} // namespace NSPropertyModel
