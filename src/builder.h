#pragma once

#include "property_model_impl.h"

namespace NSPropertyModel {
namespace detail {
static constexpr int min_priority_strength_ = -1;
template<typename... DataArgs, typename... ValueArgs, typename... OutArgs>
class Builder<Data<DataArgs...>, Value<ValueArgs...>, Out<OutArgs...>> {
  using DataTypes = Data<DataArgs...>;
  using ValueTypes = Value<ValueArgs...>;
  using OutTypes = Out<OutArgs...>;

  using PropertyModel = PropertyModel<DataTypes, ValueTypes, OutTypes>;
  //  using Builder = Builder<DataTypes, ValueTypes, OutTypes>;

  template<typename Output, typename... Inputs>
  using Signature =
      Templates::Signature<DataTypes, ValueTypes, OutTypes, Output, Inputs...>;

  template<typename MetaData>
  using SpecializedTypeof = SpecifyArithmeic<
      Templates::Type<MetaData, DataTypes, ValueTypes, OutTypes>>;

public:
  Builder(detail::SpecifyArithmeic<DataArgs>... data,
          detail::SpecifyArithmeic<ValueArgs>... value,
          detail::SpecifyArithmeic<OutArgs>... out)
      : property_model_(std::move(data)..., std::move(value)...,
                        std::move(out)...),
        new_constraint_(
            Priority{Priority::Status::Regular, Priority::Strength{0}}) {};

  void AddNewConstraint(Priority priority = Priority{Priority::Status::Regular,
                                                     Priority::Strength{0}}) {
    new_constraint_.priority = priority;
    assert(new_constraint_.methods.size() != 0);
    property_model_->ReceiveConstraint(std::move(new_constraint_));
    new_constraint_ = Constraint();
  }

  template<typename Output, typename... Inputs>
  void AddMethod(Signature<Output, Inputs...> func) {
    std::unique_ptr<Method> action =
        property_model_->template BindMethod<Output, Inputs...>(
            std::move(func));
    new_constraint_.PushBackMethod(std::move(action));
  }

  PropertyModel&& ExtractPM() {
    AddAllStay();
    property_model_->SetStayPriority(current_stay_priority_);
    property_model_->HandleVariableEntries();
    property_model_->InitConstraintGraph();
    return std::move(property_model_);
  }

  template<typename MetaData>
  void Set(SpecializedTypeof<MetaData> value) {
    property_model_->template Set<MetaData>(std::move(value));
  }

  void CallPrint() {
    property_model_->Print();
  }

private:
  template<typename MetaData>
  struct AddStayImpl {
    void operator()(Builder* builder) {
      auto value = builder->property_model_->template GetRef<MetaData>();
      builder->AddMethod<MetaData>([&value]() { return value; });
      builder->AddNewConstraint(builder->current_stay_priority_);
      builder->property_model_->template AttachStay<MetaData>();
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
    Templates::For<0, Templates::Size<DataTypes>,
                   1>::template Do<AddDataStayImpl>(this);
    Templates::For<0, Templates::Size<ValueTypes>,
                   1>::template Do<AddValueStayImpl>(this);
    Templates::For<0, Templates::Size<OutTypes>,
                   1>::template Do<AddOutStayImpl>(this);
  };

  Priority current_stay_priority_ = {
      Priority::Status::Stay, Priority::Strength{min_priority_strength_}};
  PropertyModel property_model_;
  Constraint new_constraint_;
};

} // namespace detail
} // namespace NSPropertyModel
