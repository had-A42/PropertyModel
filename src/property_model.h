#pragma once

#include "property_model_impl.h"
#include "templates.h"

namespace NSPropertyModel {
template<typename Data, typename Value, typename Out>
class PropertyModel;

template<typename... DataArgs, typename... ValueArgs, typename... OutArgs>
class PropertyModel<Data<DataArgs...>, Value<ValueArgs...>, Out<OutArgs...>> {
  using PMImpl =
      detail::PropertyModelImpl<Data<DataArgs...>, Value<ValueArgs...>,
                                Out<OutArgs...>>;
  using DataTypes = Data<DataArgs...>;
  using ValueTypes = Value<ValueArgs...>;
  using OutTypes = Out<OutArgs...>;

  template<typename MetaData>
  using SpecializedTypeof = detail::SpecifyArithmeic<
      Templates::Type<MetaData, DataTypes, ValueTypes, OutTypes>>;

public:
  using Builder = detail::Builder<DataTypes, ValueTypes, OutTypes>;
  friend Builder;

  PMImpl* operator->() {
    return impl_.get();
  }

  const PMImpl* operator->() const {
    return impl_.get();
  }

private:
  PropertyModel(detail::SpecifyArithmeic<DataArgs>... data,
                detail::SpecifyArithmeic<ValueArgs>... value,
                detail::SpecifyArithmeic<OutArgs>... Out)
      : impl_(std::make_unique<PMImpl>(std::move(data)..., std::move(value)...,
                                       std::move(Out)...)) {
  }

  std::unique_ptr<PMImpl> impl_;
};
} // namespace NSPropertyModel
