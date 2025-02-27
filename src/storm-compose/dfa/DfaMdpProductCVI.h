#ifndef DFAMDPPRODUCTCVI_H_
#define DFAMDPPRODUCTCVI_H_

#include "storm-compose/storage/ValueVector.h"
#include "storm-compose/dfa/DfaMdpProduct.h"

namespace storm {
namespace compose {
namespace dfa {

template<typename ValueType>
class DfaMdpProductCVI {
public:
    DfaMdpProductCVI(DfaMdpProduct product);

private:
    void constructValueVector();

    storm::storage::ValueVector<ValueType> valueVector;
    DfaMdpProduct dfaProduct;
};

}  // namespace dfa
}  // namespace compose
}  // namespace storm

#endif  // DFAMDPPRODUCTCVI_H_
