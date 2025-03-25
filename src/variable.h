#pragma once

#include "priority.h"
#include "templates.h"

namespace NSPropertyModel {
struct Constraint;

using IndexType = Templates::IndexType;
using StepType = Templates::StepType;
using ConstraintPtrs = std::vector<Constraint*>;

struct Variable {
  enum class Type { Data, Value, Out };
  //  enum class Index : IndexType;

  Variable(Type type, IndexType index, IndexType global_index);
  //        Variable(Type type, IndexType index, IndexType global_index, );

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

} // namespace NSPropertyModel
