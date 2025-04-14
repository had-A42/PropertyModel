#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

#include "../kernel/delta_blue.h"
#include "constraint.h"
#include "constraint_graph.h"
#include "templates.h"

namespace NSPropertyModel {

template<typename... DataArgs>
struct Data;

template<typename... ValueArgs>
struct Value;

template<typename... OutArgs>
struct Out;

template<typename Data, typename Value, typename Out>
class PropertyModel;

namespace detail {
template<typename T>
using SpecifyArithmeic =
    typename std::conditional<std::is_arithmetic_v<T>, T, T&&>::type;

template<typename Data, typename Value, typename Out>
class PropertyModelImpl;

template<typename Data, typename Value, typename Out>
class Builder;

template<typename... DataArgs, typename... ValueArgs, typename... OutArgs>
class PropertyModelImpl<Data<DataArgs...>, Value<ValueArgs...>,
                        Out<OutArgs...>> {
  using IndexType = Templates::IndexType;
  using StepType = Templates::StepType;

  using Action = std::function<void()>;
  using Constraints = std::vector<std::unique_ptr<Constraint>>;
  using Variables = std::vector<std::unique_ptr<Variable>>;

  using DataTuple = std::tuple<DataArgs...>;
  using ValueTuple = std::tuple<ValueArgs...>;
  using OutTuple = std::tuple<OutArgs...>;

  using DataTypes = Data<DataArgs...>;
  using ValueTypes = Value<ValueArgs...>;
  using OutTypes = Out<OutArgs...>;

  using Builder =
      detail::Builder<Data<DataArgs...>, Value<ValueArgs...>, Out<OutArgs...>>;
  using PropertyModel =
      PropertyModel<Data<DataArgs...>, Value<ValueArgs...>, Out<OutArgs...>>;

  template<typename MetaData>
  using SpecializedTypeof = SpecifyArithmeic<
      Templates::Type<MetaData, DataTuple, ValueTuple, OutTuple>>;
  template<typename MetaData>
  using Typeof = Templates::Type<MetaData, DataTuple, ValueTuple, OutTuple>;
  template<typename MetaData>
  using Refto = Templates::Ref<MetaData, DataTuple, ValueTuple, OutTuple>;
  template<typename MetaData>
  using ConstRefto = Templates::Ref<MetaData, DataTuple, ValueTuple, OutTuple>;
  template<typename Output, typename... Inputs>
  using Signature =
      Templates::Signature<DataTuple, ValueTuple, OutTuple, Output, Inputs...>;

  template<typename MetaData>
  static constexpr IndexType IndexGetter =
      Templates::GlobalIndex<MetaData, DataTuple, ValueTuple, OutTuple>;

  static constexpr IndexType data_size_ = Templates::Size<DataTypes>;
  static constexpr IndexType value_size_ = Templates::Size<ValueTypes>;
  static constexpr IndexType out_size_ = Templates::Size<OutTypes>;

  static constexpr Priority min_stay_priority = {Priority::Status::Stay,
                                                 Priority::Strength{0}};

  friend Builder;

public:
  PropertyModelImpl() = delete;

  PropertyModelImpl(SpecifyArithmeic<DataArgs>... data,
                    SpecifyArithmeic<ValueArgs>... value,
                    SpecifyArithmeic<OutArgs>... out)
      : data_(std::make_tuple(std::move(data)...)),
        value_(std::make_tuple(std::move(value)...)),
        out_(std::make_tuple(std::move(out)...)) {

    Templates::For<0, data_size_, 1>::template Do<InitDataVariableImpl>(this);
    Templates::For<0, value_size_, 1>::template Do<InitValueVariableImpl>(this);
    Templates::For<0, out_size_, 1>::template Do<InitOutVariableImpl>(this);
  };

  void Print() {
    std::cout << "Variable size: " << c_graph_.VariablesSize() << "\n";

    std::cout << "Constraint count: " << c_graph_.ConstraintsSize() << "\n";
    std::cout << "Variables: " << "\n";
    std::cout << "\tData: ";
    std::apply([](auto&&... args) { ((std::cout << args << " "), ...); },
               data_);

    std::cout << "\n\tValue: ";
    std::apply([](auto&&... args) { ((std::cout << args << " "), ...); },
               value_);

    std::cout << "\n\tOut: ";
    std::apply([](auto&&... args) { ((std::cout << args << " "), ...); }, out_);
    std::cout << "\n";

    std::cout << "Constraints: " << "\n";

    auto& all_constraints = c_graph_.AllConstraints();
    for (int i = 0; i < c_graph_.ConstraintsSize(); ++i) {
      std::cout << "\t" << i << " " << *all_constraints[i].get();
    }
  }

  void AddConstraint(IndexType constraint_index) {
    DeltaBlue::AddConstraintByIndex(c_graph_, constraint_index,
                                    propagation_counter_);
    c_graph_.ExecutePlan(propagation_counter_);
  }

  void RemoveConstraint(IndexType constraint_index) {
    DeltaBlue::RemoveConstraintByIndex(c_graph_, constraint_index,
                                       propagation_counter_);
    c_graph_.ExecutePlan(propagation_counter_);
  }

  template<typename MetaData>
  void Set(SpecializedTypeof<MetaData> value) {
    Refto<MetaData>::Get(data_, value_, out_) = std::move(value);

    Constraint* stay = c_graph_.VariableByIndex(IndexGetter<MetaData>)->stay;

    DeltaBlue::UpdateStayPriority(stay, current_stay_priority_,
                                  propagation_counter_);
    ++current_stay_priority_;

    c_graph_.ExecutePlan(propagation_counter_);
  }

private:
  template<typename MetaData>
  void SetBeforeExtract(SpecializedTypeof<MetaData> value) {
    Refto<MetaData>::Get(data_, value_, out_) = std::move(value);
  }

  void CreateInitialSolution() {
    c_graph_ = DeltaBlue::CreateInitialSolution(std::move(c_graph_),
                                                propagation_counter_);
    c_graph_.ExecutePlan(propagation_counter_);
  }

  void SetCurrentStayPriority(Priority priority) {
    assert(priority.status == Priority::Status::Stay);
    current_stay_priority_ = priority;
  }

  template<IndexType Index>
  struct InitDataVariableImpl {
    void operator()(PropertyModelImpl* impl) {
      impl->c_graph_.AddVariable(
          {Variable::Type::Data, Index, IndexGetter<Templates::Data<Index>>});
    }
  };

  template<IndexType Index>
  struct InitValueVariableImpl {
    void operator()(PropertyModelImpl* impl) {
      impl->c_graph_.AddVariable(
          {Variable::Type::Value, Index, IndexGetter<Templates::Value<Index>>});
    }
  };

  template<IndexType Index>
  struct InitOutVariableImpl {
    void operator()(PropertyModelImpl* impl) {
      impl->c_graph_.AddVariable(
          {Variable::Type::Out, Index, IndexGetter<Templates::Out<Index>>});
    }
  };

  template<typename Output, typename... Inputs>
  std::unique_ptr<Method> BindMethod(Signature<Output, Inputs...> func) {
    Action action = [this, func]() -> void {
      Refto<Output>::Get(data_, value_, out_) =
          func(ConstRefto<Inputs>::Get(data_, value_, out_)...);
    };

    Method method = {.action = action};
    method.out.push_back(c_graph_.VariableByIndex(IndexGetter<Output>));
    (method.in.push_back(c_graph_.VariableByIndex(IndexGetter<Inputs>)), ...);

    return std::make_unique<Method>(std::move(method));
  }

  template<typename MetaData>
  void AttachLastAsStay() {
    IndexType index = IndexGetter<MetaData>;
    c_graph_.AttachLastAsStay(index);
  }

  void ReceiveConstraint(Constraint&& constraint) {
    c_graph_.AddConstraint(std::move(constraint));
  }

  void CollectPotentialOutputs() {
    for (const auto& constraint : c_graph_.AllConstraints()) {
      for (const auto& method : constraint->methods) {
        Variable* output = GetOut(method.get());
        output->involved_as_potential_output.push_back(constraint.get());
      }
    }
  }

  template<typename MetaData>
  auto GetRef() {
    return &Refto<MetaData>::Get(data_, value_, out_);
  }

  DataTuple data_;
  ValueTuple value_;
  OutTuple out_;

  Priority current_stay_priority_ = min_stay_priority;
  ConstraintGraph c_graph_;
  StepType propagation_counter_ = StepType{0};
};
} // namespace detail
} // namespace NSPropertyModel
