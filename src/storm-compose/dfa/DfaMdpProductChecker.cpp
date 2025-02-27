#include "DfaMdpProductChecker.h"

#include "storm-compose/dfa/DfaMdpProduct.h"
#include "storm/builder/ExplicitModelBuilder.h"
#include "storm/adapters/RationalNumberAdapter.h"
#include "storm-parsers/parser/PrismParser.h"
#include "storm/utility/constants.h"

namespace storm {
namespace compose {
namespace dfa {

template<typename ValueType>
DfaMdpProductChecker<ValueType>::DfaMdpProductChecker(DfaMdpProduct dfaProduct)
    : dfaProduct(dfaProduct) {}

template<typename ValueType>
ValueType DfaMdpProductChecker<ValueType>::check() {
    constructProduct();
    return storm::utility::zero<ValueType>();
}

template<typename ValueType>
void DfaMdpProductChecker<ValueType>::constructProduct() {
    dfa::DfaMdpProductPrism productToPrism(dfaProduct.mapping);

    std::ostringstream f;
    productToPrism.writeProductAsPrism(dfaProduct.dfa, dfaProduct.program, f);
    std::string productAsString = f.str();

    auto program = storm::parser::PrismParser::parseFromString(productAsString, "product.prism");
    std::ofstream out("product.prism");
    out << program;
    out.close();
}

template class DfaMdpProductChecker<double>;
template class DfaMdpProductChecker<storm::RationalNumber>;

}  // namespace dfa
}  // namespace compose
}  // namespace storm
