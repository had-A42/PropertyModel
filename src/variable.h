#pragma once

#include "priority.h"
#include "templates.h"

namespace NSPropertyModel::detail {
struct Constraint;

struct Variable {
  using IndexType = Templates::IndexType;
  using StepType = Templates::StepType;
  using ConstraintPtrs = std::vector<Constraint*>;

  enum class Type { Data, Value, Out };

  Variable(Type type, IndexType index, IndexType global_index);

  Type type;
  IndexType index;
  IndexType global_index;

  StepType last_propagation = StepType{0};
  Priority priority;
  Constraint* determined_by = nullptr;
  Constraint* stay = nullptr;
  ConstraintPtrs involved_as_potential_output;
};

void UpdatePriority(Variable* variable);
void UpdateStep(Variable* variable, Variable::StepType propagation_index);

void SetDeterminedBy(Variable* variable, Constraint* constraint);
void SetDeterminedByNull(Variable* variable);

Constraint* GetStay(Variable* variable);
const Constraint* GetStay(const Variable* variable);

bool IsUpdatedInCurrentStep(const Variable* variable,
                            Variable::StepType current_step);
bool IsProcessing(const Variable* variable, Variable::StepType current_step);

std::ostream& operator<<(std::ostream& out, const Variable::Type& type);
std::ostream& operator<<(std::ostream& out, const Variable& variable);
} // namespace NSPropertyModel::detail
