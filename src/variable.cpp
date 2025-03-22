#include "variable.h"
#include "constraint.h"

namespace NSPropertyModel {

Variable::Variable(Type type, Index index)
    : type(type), index(index), determined_by(nullptr) {};

void Variable::UpdatePriority() {
  priority = std::min({determined_by->priority,
                       determined_by->PotentialOutputsMinPriority(this)});
}

void Variable::UpdateStep(StepType propagation_index) {
  last_propagation = propagation_index;
}

void Variable::SetDeterminedBy(Constraint* constraint) {
  determined_by = constraint;
}

void Variable::SetDeterminedByNull() {
  determined_by = nullptr;
}

Constraint* Variable::GetStay() {
  return stay;
}

bool Variable::IsUpdatedInCurrentStep(StepType current_step) {
  return current_step + 1 == last_propagation;
}

bool Variable::IsProcessing(StepType current_step) {
  return current_step == last_propagation;
}
} // namespace NSPropertyModel
