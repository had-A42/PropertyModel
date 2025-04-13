#include "variable.h"
#include "constraint.h"

namespace NSPropertyModel::detail {

Variable::Variable(Type type, IndexType index, IndexType global_index)
    : type(type), index(index), global_index(global_index),
      determined_by(nullptr) {};

void UpdatePriority(Variable* variable) {
    assert(variable != nullptr);
  variable->priority = std::min(
      {variable->determined_by->priority,
       PotentialOutputsMinPriority(variable->determined_by, variable)});

}

void UpdateStep(Variable* variable, Variable::StepType propagation_index) {
    assert(variable != nullptr);
  variable->last_propagation = propagation_index;
}

void SetDeterminedBy(Variable* variable, Constraint* constraint) {
    assert(variable != nullptr);
    assert(constraint != nullptr);
  variable->determined_by = constraint;
}

void SetDeterminedByNull(Variable* variable) {
    assert(variable != nullptr);
  variable->determined_by = nullptr;
}

Constraint* GetStay(Variable* variable) {
    assert(variable != nullptr);
  return variable->stay;
}

const Constraint* GetStay(const Variable* variable) {
    assert(variable != nullptr);
  return variable->stay;
}

bool IsUpdatedInCurrentStep(const Variable* variable,
                            Variable::StepType current_step) {
    assert(variable != nullptr);
  return current_step + 1 == variable->last_propagation;
}

bool IsProcessing(const Variable* variable, Variable::StepType current_step) {
    assert(variable != nullptr);
  return current_step == variable->last_propagation;
}

std::ostream& operator<<(std::ostream& out, const Variable::Type& type) {
  switch (type) {
  case (Variable::Type::Data):
    std::cout << "Data";
    break;
  case (Variable::Type::Value):
    std::cout << "Value";
    break;
  case (Variable::Type::Out):
    std::cout << "Out";
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, const Variable& variable) {
  std::cout << variable.type << "<" << variable.index << ">";
  return out;
}
} // namespace NSPropertyModel::detail
