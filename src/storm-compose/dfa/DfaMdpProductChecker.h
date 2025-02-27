#ifndef DFAMDPPRODUCTCHECKER_H_
#define DFAMDPPRODUCTCHECKER_H_

#include "DfaMdpProductPrism.h"
#include "storm-compose/parser/DfaParser.h"
#include "storm-compose/dfa/DfaMdpProduct.h"

namespace storm {
namespace compose {
namespace dfa {

template <typename ValueType>
class DfaMdpProductChecker {
public:
    DfaMdpProductChecker(DfaMdpProduct dfaProduct);

    ValueType check();

private:
    void constructProduct();

    DfaMdpProduct dfaProduct;
};

}  // namespace dfa
}  // namespace compose
}  // namespace storm

#endif  // DFAMDPPRODUCTCHECKER_H_
