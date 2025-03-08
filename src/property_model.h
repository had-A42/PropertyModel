#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <tuple>
#include <utility>

#include "templates.h"

template<typename DataTuple, typename ValueTuple, typename OutsTuple>
class PropertyModelImpl;

template<typename DataTuple, typename ValueTuple, typename OutsTuple>
class PropertyModel;

namespace secrets {
template<typename DataTuple, typename ValueTuple, typename OutsTuple>
class Builder {};
} // namespace secrets

template<typename... DataArgs, typename... ValueArgs, typename... OutsArgs>
class PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
                        std::tuple<OutsArgs...>> {
public:
  using Action = std::function<void()>;
  using Constraint = std::vector<Action>;
  using Constraints = std::vector<Constraint>;

  using DataTuple = std::tuple<DataArgs...>;
  using ValueTuple = std::tuple<ValueArgs...>;
  using OutsTuple = std::tuple<OutsArgs...>;

  using Builder =
      secrets::Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
                       std::tuple<OutsArgs...>>;
  using PropertyModel =
      PropertyModel<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
                    std::tuple<OutsArgs...>>;

  friend PropertyModel;
  friend Builder;

  void Print() {
    std::cout << "Data: ";
    std::apply([](auto&&... args) { ((std::cout << args << " "), ...); },
               data_);

    std::cout << "\nValue: ";
    std::apply([](auto&&... args) { ((std::cout << args << " "), ...); },
               value_);

    std::cout << "\nOut: ";
    std::apply([](auto&&... args) { ((std::cout << args << " "), ...); },
               outs_);
    std::cout << "\n";

    std::cout << "Constraint count: " << constraints_.size() << "\n";
    for (auto& constraint : constraints_) {
      std::cout << "Constraint size: " << constraint.size() << "\n";
    }
    std::cout << "\n";
  }

  PropertyModelImpl() = delete;

  /*
   * я понимаю, что конструктор обязан быть приватным, но тогда код не
   * компилируется и говорит, что нет прав доступа, даже если все друг другу
   * друзья
   */
  explicit PropertyModelImpl(DataArgs... data, ValueArgs... value,
                             OutsArgs... outs)
      : data_(std::make_tuple(data...)), value_(std::make_tuple(value...)),
        outs_(std::make_tuple(outs...)) {};

private:
  // нужно захватить окружение внутри Impl
  template<typename Output, typename... Inputs>
  Action BindMethod(
      Signature<DataTuple, ValueTuple, OutsTuple, Output, Inputs...> func) {
    return [this, func]() -> void {
      Ref<Output, DataTuple, ValueTuple, OutsTuple>::Get(data_, value_, outs_) =
          func(ConstRef<Inputs, DataTuple, ValueTuple, OutsTuple>::Get(
              data_, value_, outs_)...);
    };
  }

  template<typename Variable>
  void Set(Type<Variable, DataTuple, ValueTuple, OutsTuple>&& value) {
    Ref<Variable, DataTuple, ValueTuple, OutsTuple>::Get(data_, value_, outs_) =
        value;
  }

  void ReceiveConstraint(Constraint&& c) {
    constraints_.push_back(std::move(c));
  }

  DataTuple data_;
  ValueTuple value_;
  OutsTuple outs_;

  Constraints constraints_;
};

template<typename... DataArgs, typename... ValueArgs, typename... OutsArgs>
class PropertyModel<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
                    std::tuple<OutsArgs...>> {
public:
  using Builder =
      secrets::Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
                       std::tuple<OutsArgs...>>;
  using PMImpl =
      PropertyModelImpl<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
                        std::tuple<OutsArgs...>>;

  friend Builder;

  PMImpl* operator->() {
    return impl_.get();
  }

  const PMImpl* operator->() const {
    return impl_.get();
  }

private:
  explicit PropertyModel(DataArgs... data, ValueArgs... value,
                         OutsArgs... outs) {
    impl_ = std::make_unique<PMImpl>(data..., value..., outs...);
  }

  std::unique_ptr<PMImpl> impl_;
};

namespace secrets {
template<typename... DataArgs, typename... ValueArgs, typename... OutsArgs>
class Builder<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
              std::tuple<OutsArgs...>> {
public:
  using PropertyModel =
      PropertyModel<std::tuple<DataArgs...>, std::tuple<ValueArgs...>,
                    std::tuple<OutsArgs...>>;

  using Action = std::function<void()>;
  using Constraint = std::vector<Action>;
  using Constraints = std::vector<Constraint>;

  using DataTuple = std::tuple<DataArgs...>;
  using ValueTuple = std::tuple<ValueArgs...>;
  using OutsTuple = std::tuple<OutsArgs...>;

  explicit Builder(DataArgs... data, ValueArgs... value, OutsArgs... outs)
      : property_model_(data..., value..., outs...) {};

  void AddNewConstraint() {
    property_model_->ReceiveConstraint(std::move(new_constraint_));
    new_constraint_ = Constraint();
  }

  template<typename Output, typename... Inputs>
  void AddMethod(
      Signature<DataTuple, ValueTuple, OutsTuple, Output, Inputs...> func) {
    Action action =
        property_model_->template BindMethod<Output, Inputs...>(func);
    new_constraint_.push_back(action);
  }

  PropertyModel&& GetPM() {
    return std::move(property_model_);
  }

  template<typename Variable>
  void Set(Type<Variable, DataTuple, ValueTuple, OutsTuple>&& value) {
    property_model_->template Set<Variable>(std::move(value));
  }

  void CallPrint() {
    property_model_->Print();
  }

private:
  PropertyModel property_model_;
  Constraint new_constraint_;
};

} // namespace secrets
