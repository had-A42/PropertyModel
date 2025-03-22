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

using IndexType = Templates::IndexType;

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
  using IndexGetter =
      VariableIndexGetter<MetaData, DataTuple, ValueTuple, OutTuple>;

  friend Builder;

public:
  void Print() {
    std::cout << "Variable size: " << variables_.size() << "\n";

    std::cout << "Constraint count: " << constraints_.size() << "\n";
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

    for (int i = 0; i < constraints_.size(); ++i) {
      std::cout << "\t" << i << " " << *constraints_[i].get();
    }
    std::cout
        << "_____________________________________________________________\n";
  }

  PropertyModelImpl() = delete;

  template<class>
  struct TD;

  template<class...>
  struct List {};

  PropertyModelImpl(SpecifyArithmeic<DataArgs>... data,
                    SpecifyArithmeic<ValueArgs>... value,
                    SpecifyArithmeic<OutArgs>... out)
      : data_(std::make_tuple(std::move(data)...)),
        value_(std::make_tuple(std::move(value)...)),
        out_(std::make_tuple(std::move(out)...)) {

    //  TD<List<SpecifyArithmeic<DataArgs>...>> x;

    Templates::For<0, Templates::Size<DataTypes>,
                   1>::template Do<InitDataVariableImpl>(this);
    Templates::For<0, Templates::Size<ValueTypes>,
                   1>::template Do<InitValueVariableImpl>(this);
    Templates::For<0, Templates::Size<OutTypes>,
                   1>::template Do<InitOutVariableImpl>(this);
  };

  void Update() {
    char meta_marker;
    IndexType index;
    std::cout << "Enter meta marker and index (D/V/O): ";
    std::cin >> meta_marker;

    switch (meta_marker) {
    case 'D':
      std::cin >> index;
      if (index >= Templates::Size<DataTypes>) {
        std::cout << "Index out of range!\n";
      }
      //      std::cin >> std::get<index>(data_);
      NSDeltaBlue::DeltaBlue::UpdateStayPriority(c_graph_, GetStay(index),
                                                 current_stay_priority_,
                                                 propagation_counter_);
      break;
    case 'V':
      std::cin >> index;
      if (index >= Templates::Size<ValueTypes>) {
        std::cout << "Index out of range!\n";
      }
      //      std::cin >> std::get<index>(value_);
      NSDeltaBlue::DeltaBlue::UpdateStayPriority(
          c_graph_, GetStay(index + Templates::Size<DataTypes>),
          current_stay_priority_, propagation_counter_);
      break;
    case 'O':
      std::cin >> index;
      if (index >= Templates::Size<OutTypes>) {
        std::cout << "Index out of range!\n";
      }
      //      std::cin >> std::get<index>(out_);
      NSDeltaBlue::DeltaBlue::UpdateStayPriority(
          c_graph_,
          GetStay(index + Templates::Size<ValueTypes> +
                  Templates::Size<DataTypes>),
          current_stay_priority_, propagation_counter_);
      break;
    default:
      std::cout << "Invalid MetaData marker!\n";
    }
    ++current_stay_priority_;
  }

  void RemoveConstraint(IndexType constraint_index) {
    NSDeltaBlue::DeltaBlue::RemoveConstraintByIndex(c_graph_, constraint_index,
                                                    propagation_counter_);
  }

  void AddConstraint(IndexType constraint_index) {
    NSDeltaBlue::DeltaBlue::AddConstraintByIndex(c_graph_, constraint_index,
                                                 propagation_counter_);
  }

private:
  void InitConstraintGraph() {
    NSDeltaBlue::DeltaBlue::Initialise(constraints_, variables_, c_graph_,
                                       propagation_counter_);
  }

  void SetStayPriority(Priority priority) {
    assert(priority.status == Priority::Status::Stay);
    current_stay_priority_ = priority;
  }

  template<IndexType Index>
  struct InitDataVariableImpl {
    void operator()(PropertyModelImpl* impl) {
      impl->variables_.push_back(std::make_unique<Variable>(
          Variable::Type::Data, Variable::Index{Index}));
    }
  };

  template<IndexType Index>
  struct InitValueVariableImpl {
    void operator()(PropertyModelImpl* impl) {
      impl->variables_.push_back(std::make_unique<Variable>(
          Variable::Type::Value, Variable::Index{Index}));
    }
  };

  template<IndexType Index>
  struct InitOutVariableImpl {
    void operator()(PropertyModelImpl* impl) {
      impl->variables_.push_back(std::make_unique<Variable>(
          Variable::Type::Out, Variable::Index{Index}));
    }
  };

  template<typename Output, typename... Inputs>
  std::unique_ptr<Method> BindMethod(Signature<Output, Inputs...> func) {
    Action action = [this, func]() -> void {
      Refto<Output>::Get(data_, value_, out_) =
          func(ConstRefto<Inputs>::Get(data_, value_, out_)...);
    };

    Method method = {.action = action};
    method.out.push_back(variables_[IndexGetter<Output>::Get()].get());
    (method.in.push_back(variables_[IndexGetter<Inputs>::Get()].get()), ...);

    return std::make_unique<Method>(std::move(method));
  }

  template<typename MetaData>
  void AttachStay() {
    IndexType index = IndexGetter<MetaData>::Get();
    variables_[index].get()->stay = constraints_.back().get();
    variables_[index].get()->involved_as_potential_output.push_back(
        constraints_.back().get());
  }

  auto GetStay(IndexType index) {
    return variables_[index].get()->stay;
  }

  template<typename MetaData>
  void Set(SpecializedTypeof<MetaData> value) {
    Refto<MetaData>::Get(data_, value_, out_) = std::move(value);
    //    TD<List<SpecifyArithmeic<Templates::Type<Variable, DataTuple,
    //    ValueTuple, OutTuple>>>> x;
  }

  void ReceiveConstraint(Constraint&& c) {
    constraints_.push_back(std::make_unique<Constraint>(std::move(c)));
    Constraint* new_constraint = constraints_.back().get();
  }

  void HandleVariableEntries() {
    for (auto& constraint : constraints_) {
      for (auto& method : constraint.get()->methods) {
        Variable* output = method->GetOut();
        output->involved_as_potential_output.push_back(constraint.get());
      }
    }
  }

  template<typename MetaData>
  auto GetRef() {
    return Refto<MetaData>::Get(data_, value_, out_);
  }

  DataTuple data_;
  ValueTuple value_;
  OutTuple out_;

  Variables variables_;
  Constraints constraints_;

  Priority current_stay_priority_ = {Priority::Status::Stay,
                                     Priority::Strength{-1}};
  ConstraintGraph c_graph_;
  StepType propagation_counter_ = StepType{0};
};
} // namespace detail
} // namespace NSPropertyModel
