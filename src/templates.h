#pragma once

#include <functional>
#include <tuple>

namespace Templates {
using IndexType = int;
using StepType = int;

// виды переменных

template<IndexType Size>
struct Data {};

template<IndexType Size>
struct Value {};

template<IndexType Size>
struct Out {};

// Get - "достать тип из контейнера"

namespace detail {
template<IndexType index, typename TypeContainer>
struct GetHelper;

template<template<typename...> typename TypeContainer, typename FirstArg,
         typename... Args>
struct GetHelper<0, TypeContainer<FirstArg, Args...>> {
  using type = FirstArg;
};

template<IndexType index, template<typename...> typename TypeContainer,
         typename FirstArg, typename... Args>
struct GetHelper<index, TypeContainer<FirstArg, Args...>> {
  using type = typename GetHelper<index - 1, TypeContainer<Args...>>::type;
};
} // namespace detail

template<IndexType index, typename TypeContainer>
using Get = typename detail::GetHelper<index, TypeContainer>::type;

// Type - "достать тип из Data<1>, например"
namespace detail {
template<typename T, typename DataTuple, typename ValueTuple, typename OutTuple>
struct TypeHelper;

template<IndexType index, typename DataTuple, typename ValueTuple,
         typename OutTuple>
struct TypeHelper<Data<index>, DataTuple, ValueTuple, OutTuple> {
  using type = Get<index, DataTuple>;
};

template<IndexType index, typename DataTuple, typename ValueTuple,
         typename OutTuple>
struct TypeHelper<Value<index>, DataTuple, ValueTuple, OutTuple> {
  using type = Get<index, ValueTuple>;
};

template<IndexType index, typename DataTuple, typename ValueTuple,
         typename OutTuple>
struct TypeHelper<Out<index>, DataTuple, ValueTuple, OutTuple> {
  using type = Get<index, OutTuple>;
};
} // namespace detail

template<typename T, typename DataTuple, typename ValueTuple, typename OutTuple>
using Type =
    typename detail::TypeHelper<T, DataTuple, ValueTuple, OutTuple>::type;

namespace detail {
template<typename DataTuple, typename ValueTuple, typename OutTuple,
         typename... Args>
struct SignatureHelper;

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         typename Output, typename... Inputs>
struct SignatureHelper<DataTuple, ValueTuple, OutTuple, Output, Inputs...> {
  using type = std::function<Type<Output, DataTuple, ValueTuple, OutTuple>(
      Type<Inputs, DataTuple, ValueTuple, OutTuple>...)>;
};
} // namespace detail

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         typename... Args>
using Signature = typename detail::SignatureHelper<DataTuple, ValueTuple,
                                                   OutTuple, Args...>::type;

// PushFront
namespace detail {
template<auto Element, typename ElementContainer>
struct PushFrontHelper;

template<typename ElementType,
         template<typename, ElementType...> typename ElementContainer,
         ElementType Element, ElementType... Elements>
struct PushFrontHelper<Element, ElementContainer<ElementType, Elements...>> {
  using type = ElementContainer<ElementType, Element, Elements...>;
};
} // namespace detail

template<auto Element, typename ElementContainer>
using PushFront = detail::PushFrontHelper<Element, ElementContainer>::type;

// MakeSequence - последовательность для For
namespace detail {
template<typename ElementType, ElementType... Elements>
struct List {};

template<IndexType from, IndexType to, IndexType step, bool in_range>
struct MakeSequenceHelper {
  using type =
      PushFront<from, typename MakeSequenceHelper<from + step, to, step,
                                                  (from + step < to)>::type>;
};

template<IndexType from, IndexType to, IndexType step>
struct MakeSequenceHelper<from, to, step, false> {
  using type = List<IndexType>;
};

template<IndexType from, IndexType to, IndexType step>
using MakeSequence =
    detail::MakeSequenceHelper<from, to, step, (from < to)>::type;

// For

template<typename Sequence>
struct ForHelper;

template<typename ElementType,
         template<typename, ElementType...> typename ElementContainer,
         ElementType... Elements>
struct ForHelper<ElementContainer<ElementType, Elements...>> {
  template<template<IndexType> typename Function, typename... ArgsType>
  static void Do(ArgsType&&... args) {
    (Function<Elements>()(std::forward<ArgsType>(args)...), ...);
  }
};
} // namespace detail

template<IndexType from, IndexType to, IndexType step>
struct For : detail::ForHelper<detail::MakeSequence<from, to, step>> {};

// Size

namespace detail {
template<typename ElementContainer>
struct SizeHelper;

template<template<typename...> typename TypeContainer, typename... Args>
struct SizeHelper<TypeContainer<Args...>> {
  static constexpr IndexType value = sizeof...(Args);
};
} // namespace detail

template<typename ElementContainer>
static constexpr IndexType Size = detail::SizeHelper<ElementContainer>::value;

// Ref<Data<1>> -> cсылка на первый элемент DataTuple
template<typename T, typename DataTuple, typename ValueTuple, typename OutTuple>
struct Ref;

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         IndexType index>
struct Ref<Data<index>, DataTuple, ValueTuple, OutTuple> {
  static auto& Get(DataTuple& data, ValueTuple& value, OutTuple& out) {
    return std::get<index>(data);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         IndexType index>
struct Ref<Value<index>, DataTuple, ValueTuple, OutTuple> {
  static auto& Get(DataTuple& data, ValueTuple& value, OutTuple& out) {
    return std::get<index>(value);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         IndexType index>
struct Ref<Out<index>, DataTuple, ValueTuple, OutTuple> {
  static auto& Get(DataTuple& data, ValueTuple& value, OutTuple& out) {
    return std::get<index>(out);
  }
};

// ConstRef<Data<1>> -> константная cсылка на первый элемент DataTuple
template<typename DataTuple, typename ValueTuple, typename OutTuple, typename T>
struct ConstRef;

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         IndexType index>
struct ConstRef<Data<index>, DataTuple, ValueTuple, OutTuple> {
  static const auto& Get(DataTuple& data, ValueTuple& value, OutTuple& Out) {
    return std::get<index>(data);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         IndexType index>
struct ConstRef<Value<index>, DataTuple, ValueTuple, OutTuple> {
  static const auto& Get(DataTuple& data, ValueTuple& value, OutTuple& Out) {
    return std::get<index>(value);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutTuple,
         IndexType index>
struct ConstRef<Out<index>, DataTuple, ValueTuple, OutTuple> {
  static const auto& Get(DataTuple& data, ValueTuple& value, OutTuple& Out) {
    return std::get<index>(Out);
  }
};

namespace detail {
template<typename T, typename DataTypes, typename ValueTypes, typename OutTypes>
struct VariableIndexGetter;

template<IndexType Index, typename DataTypes, typename ValueTypes,
         typename OutTypes>
struct VariableIndexGetter<Templates::Data<Index>, DataTypes, ValueTypes,
                           OutTypes> {
  static constexpr IndexType value = Index;
};

template<IndexType Index, typename DataTypes, typename ValueTypes,
         typename OutTypes>
struct VariableIndexGetter<Templates::Value<Index>, DataTypes, ValueTypes,
                           OutTypes> {
  static constexpr IndexType value = Index + Templates::Size<DataTypes>;
};

template<IndexType Index, typename DataTypes, typename ValueTypes,
         typename OutTypes>
struct VariableIndexGetter<Templates::Out<Index>, DataTypes, ValueTypes,
                           OutTypes> {
  static constexpr IndexType value =
      Index + Templates::Size<DataTypes> + Templates::Size<ValueTypes>;
};
} //  namespace detail

template<typename MetaData, typename DataTypes, typename ValueTypes,
         typename OutTypes>
static constexpr IndexType GlobalIndex =
    detail::VariableIndexGetter<MetaData, DataTypes, ValueTypes,
                                OutTypes>::value;
} // namespace Templates
