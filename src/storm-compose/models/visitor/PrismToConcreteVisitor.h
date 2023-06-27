#pragma once

#include "OpenMdpVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class PrismToConcreteVisitor : public OpenMdpVisitor<ValueType> {
    public:
    PrismToConcreteVisitor() {
    }

    virtual ~PrismToConcreteVisitor() {}

    virtual void visitPrismModel(const PrismModel<ValueType>& model) override {
        // TODO
    }
};

template class PrismToConcreteVisitor<double>;
template class PrismToConcreteVisitor<storm::RationalNumber>;

}
}
}
