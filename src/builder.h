#pragma once

#include "property_model_impl.h"

namespace NSPropertyModel::detail {
template<typename... DataArgs, typename... ValueArgs, typename... OutArgs>
class Builder<Data<DataArgs...>, Value<ValueArgs...>, Out<OutArgs...>> {
  using IndexType = Templates::IndexType;
  using StepType = Templates::StepType;

  using DataTypes = Data<DataArgs...>;
  using ValueTypes = Value<ValueArgs...>;
  using OutTypes = Out<OutArgs...>;

  using PropertyModel = PropertyModel<DataTypes, ValueTypes, OutTypes>;

  template<typename Output, typename... Inputs>
  using Signature =
      Templates::Signature<DataTypes, ValueTypes, OutTypes, Output, Inputs...>;

  template<typename MetaData>
  using SpecializedTypeof = SpecifyArithmeic<
      Templates::Type<MetaData, DataTypes, ValueTypes, OutTypes>>;

  static constexpr Priority min_stay_priority = {Priority::Status::Stay,
                                                 Priority::Strength{0}};
  static constexpr Priority max_regular_priority = {Priority::Status::Regular,
                                                    Priority::Strength{0}};

public:
  Builder(SpecifyArithmeic<DataArgs>... data,
          SpecifyArithmeic<ValueArgs>... value,
          SpecifyArithmeic<OutArgs>... out)
      : property_model_(std::move(data)..., std::move(value)...,
                        std::move(out)...),
        new_constraint_(max_regular_priority) {};

  void AddNewConstraint(Priority::Strength strength) {
    AddNewConstraintImpl(Priority{Priority::Status::Regular, strength});
  }

  template<typename Output, typename... Inputs>
  void AddMethod(Signature<Output, Inputs...> func) {
    std::unique_ptr<Method> action =
        property_model_->template BindMethod<Output, Inputs...>(
            std::move(func));
    new_constraint_.PushBackMethod(std::move(action));
  }

  PropertyModel ExtractPM() {
    AddNewConstraint(Priority::Strength{0});
    AddAllStay();
    property_model_->SetCurrentStayPriority(current_stay_priority_);
    property_model_->CollectPotentialOutputs();
    property_model_->CreateInitialSolution();
    return std::move(property_model_);
  }

  template<typename MetaData>
  void Set(SpecializedTypeof<MetaData> value) {
    property_model_->template SetBeforeExtract<MetaData>(std::move(value));
  }

  void CallPrint() {
    property_model_->Print();
  }

private:
  void AddNewConstraintImpl(Priority priority) {
    if (!new_constraint_.methods.empty()) {
      property_model_->ReceiveConstraint(std::move(new_constraint_));
    }
    new_constraint_ = Constraint(priority);
  }

  template<typename MetaData>
  struct AddStayImpl {
    void operator()(Builder* builder) {
      Templates::Type<MetaData, DataTypes, ValueTypes, OutTypes>* value =
          builder->property_model_->template GetRef<MetaData>();
      builder->AddNewConstraintImpl(builder->current_stay_priority_);
      builder->AddMethod<MetaData>([value]() { return *value; });

      // очищаем буффер new_constraint чтобы привязать stay к переменной
      builder->AddNewConstraint(Priority::Strength{0});
      builder->property_model_->template AttachLastAsStay<MetaData>();
      ++builder->current_stay_priority_;
    }
  };

  template<typename MetaData>
  friend struct AddStayImpl;

  template<IndexType Index>
  struct AddDataStayImpl {
    void operator()(Builder* builder) {
      AddStayImpl<Templates::Data<Index>>()(builder);
    }
  };

  template<IndexType Index>
  struct AddValueStayImpl {
    void operator()(Builder* builder) {
      AddStayImpl<Templates::Value<Index>>()(builder);
    }
  };

  template<IndexType Index>
  struct AddOutStayImpl {
    void operator()(Builder* builder) {
      AddStayImpl<Templates::Out<Index>>()(builder);
    }
  };

  void AddAllStay() {
    using namespace Templates;
    For<0, Size<OutTypes>, 1>::template Do<AddOutStayImpl>(this);
    AddNewConstraint(Priority::Strength{0});
    For<0, Size<ValueTypes>, 1>::template Do<AddValueStayImpl>(this);
    For<0, Size<DataTypes>, 1>::template Do<AddDataStayImpl>(this);
  };

  Priority current_stay_priority_ = min_stay_priority;
  PropertyModel property_model_;
  Constraint new_constraint_;
};

} // namespace NSPropertyModel::detail
