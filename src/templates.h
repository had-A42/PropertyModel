#pragma once

#include <functional>
#include <tuple>

using IndexType = int;

// виды переменных

template<IndexType Size>
struct Data {};

template<IndexType Size>
struct Value {};

template<IndexType Size>
struct Outs {};

// Get - "достать тип из контейнера"

template<IndexType index, typename List>
struct GetHelper;

template<typename FirstArg, typename... Args>
struct GetHelper<0, std::tuple<FirstArg, Args...>> {
  using type = FirstArg;
};

template<IndexType index, typename FirstArg, typename... Args>
struct GetHelper<index, std::tuple<FirstArg, Args...>> {
  using type = typename GetHelper<index - 1, std::tuple<Args...>>::type;
};

template<IndexType index, typename List>
using Get = typename GetHelper<index, List>::type;

// Type - "достать тип из Data<1>, например"

/* я бы хотел не прописывать DataTuple, ValueTuple, OutsTuple в шаблоне, но
 * для этого, кажется, нужно перенести все эти классы во внутрь билдера,
 * не уверен, насколько это хорошая идея
 * */
template<typename T, typename DataTuple, typename ValueTuple,
         typename OutsTuple>
struct TypeHelper;

template<IndexType index, typename DataTuple, typename ValueTuple,
         typename OutsTuple>
struct TypeHelper<Data<index>, DataTuple, ValueTuple, OutsTuple> {
  using type = Get<index, DataTuple>;
};

template<IndexType index, typename DataTuple, typename ValueTuple,
         typename OutsTuple>
struct TypeHelper<Value<index>, DataTuple, ValueTuple, OutsTuple> {
  using type = Get<index, ValueTuple>;
};

template<IndexType index, typename DataTuple, typename ValueTuple,
         typename OutsTuple>
struct TypeHelper<Outs<index>, DataTuple, ValueTuple, OutsTuple> {
  using type = Get<index, OutsTuple>;
};

template<typename T, typename DataTuple, typename ValueTuple,
         typename OutsTuple>
using Type = typename TypeHelper<T, DataTuple, ValueTuple, OutsTuple>::type;

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         typename... Args>
struct SignatureHelper;

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         typename Output, typename... Inputs>
struct SignatureHelper<DataTuple, ValueTuple, OutsTuple, Output, Inputs...> {
  using type = std::function<Type<Output, DataTuple, ValueTuple, OutsTuple>(
      Type<Inputs, DataTuple, ValueTuple, OutsTuple>...)>;
};

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         typename... Args>
using Signature =
    typename SignatureHelper<DataTuple, ValueTuple, OutsTuple, Args...>::type;

// Ref<Data<1>> -> cсылка на первый элемент DataTuple
template<typename T, typename DataTuple, typename ValueTuple,
         typename OutsTuple>
struct Ref;

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         IndexType index>
struct Ref<Data<index>, DataTuple, ValueTuple, OutsTuple> {
  static auto& Get(DataTuple& data, ValueTuple& value, OutsTuple& outs) {
    return std::get<index>(data);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         IndexType index>
struct Ref<Value<index>, DataTuple, ValueTuple, OutsTuple> {
  static auto& Get(DataTuple& data, ValueTuple& value, OutsTuple& outs) {
    return std::get<index>(value);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         IndexType index>
struct Ref<Outs<index>, DataTuple, ValueTuple, OutsTuple> {
  static auto& Get(DataTuple& data, ValueTuple& value, OutsTuple& outs) {
    return std::get<index>(outs);
  }
};

// ConstRef<Data<1>> -> константная cсылка на первый элемент DataTuple
template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         typename T>
struct ConstRef;

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         IndexType index>
struct ConstRef<Data<index>, DataTuple, ValueTuple, OutsTuple> {
  static const auto& Get(DataTuple& data, ValueTuple& value, OutsTuple& outs) {
    return std::get<index>(data);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         IndexType index>
struct ConstRef<Value<index>, DataTuple, ValueTuple, OutsTuple> {
  static const auto& Get(DataTuple& data, ValueTuple& value, OutsTuple& outs) {
    return std::get<index>(value);
  }
};

template<typename DataTuple, typename ValueTuple, typename OutsTuple,
         IndexType index>
struct ConstRef<Outs<index>, DataTuple, ValueTuple, OutsTuple> {
  static const auto& Get(DataTuple& data, ValueTuple& value, OutsTuple& outs) {
    return std::get<index>(outs);
  }
};
