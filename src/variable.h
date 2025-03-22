#pragma once

#include "priority.h"
#include "templates.h"

namespace NSPropertyModel {
struct Constraint;

using IndexType = Templates::IndexType;
using StepType = int;
using ConstraintPtrs = std::vector<Constraint*>;

struct Variable {
  enum class Type { Data, Value, Out };
  enum class Index : IndexType;

  Variable(Type type, Index index);

  void UpdatePriority();
  void UpdateStep(StepType propagation_index);

  void SetDeterminedBy(Constraint* constraint);
  void SetDeterminedByNull();

  Constraint* GetStay();

  bool IsUpdatedInCurrentStep(StepType current_step);
  bool IsProcessing(StepType current_step);

  Type type;
  Index index;

  StepType last_propagation = StepType{0};
  Priority priority;
  Constraint* determined_by = nullptr;
  Constraint* stay;
  ConstraintPtrs involved_as_potential_output;
};

template<typename T>
struct VariableMaker;

template<IndexType Index>
struct VariableMaker<Templates::Data<Index>> {
  static Variable Make() {
    return Variable(Variable::Type::Data, Variable::Index{Index});
  }
};

template<IndexType Index>
struct VariableMaker<Templates::Value<Index>> {
  static Variable Make() {
    return Variable(Variable::Type::Value, Variable::Index{Index});
  }
};

template<IndexType Index>
struct VariableMaker<Templates::Out<Index>> {
  static Variable Make() {
    return Variable(Variable::Type::Out, Variable::Index{Index});
  }
};

template<typename T, typename DataTypes, typename ValueTypes, typename OutTypes>
struct VariableIndexGetter;

template<IndexType Index, typename DataTypes, typename ValueTypes,
         typename OutTypes>
struct VariableIndexGetter<Templates::Data<Index>, DataTypes, ValueTypes,
                           OutTypes> {
  static IndexType Get() {
    return Index;
  }
};

template<IndexType Index, typename DataTypes, typename ValueTypes,
         typename OutTypes>
struct VariableIndexGetter<Templates::Value<Index>, DataTypes, ValueTypes,
                           OutTypes> {
  static IndexType Get() {
    return Index + Templates::Size<DataTypes>;
  }
};

template<IndexType Index, typename DataTypes, typename ValueTypes,
         typename OutTypes>
struct VariableIndexGetter<Templates::Out<Index>, DataTypes, ValueTypes,
                           OutTypes> {
  static IndexType Get() {
    return Index + Templates::Size<DataTypes> + Templates::Size<ValueTypes>;
  }
};
} // namespace NSPropertyModel
