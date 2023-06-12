#pragma once

#include <string>
#include <boost/optional.hpp>
#include <memory>

#include "storm/adapters/RationalNumberAdapter.h"
#include "OpenMdpManager.h"

/*
 What is an OpenMdp?

 It will be the abstract base class for everything which is part of an OpenMdp.
 In Haskell style pseudocode:

 data OpenMdp =
   ConcreteMdp([State], [State], [State], [State]) |
   Sum([OpenMdp], Map String String) |
   Sequence([OpenMdp], Map String String) |
   Trace(OpenMdp, Map String String) |
   Reference(String)

 Additionally, everything can be named so that we can use references (by name)
*/

namespace storm {
namespace models {

template<typename ValueType>
class OpenMdpManager;

//template<typename ValueType>
//class Reference;

template<typename ValueType>
class OpenMdp : public std::enable_shared_from_this<OpenMdp<ValueType>> {
    public:
    virtual ~OpenMdp() = 0;
    OpenMdp(OpenMdpManager<ValueType> &manager);

    bool hasName();
    std::string getName();

    /// Follow references until a non-reference type is found
    /// Does not check for loops
    std::shared_ptr<OpenMdp<ValueType>> followReferences();
    OpenMdpManager<ValueType>& getManager();

    virtual bool isConcreteMdp();
    virtual bool isSum();
    virtual bool isSequence();
    virtual bool isTrace();
    virtual bool isReference();
    virtual bool isPrismModel();
    std::shared_ptr<OpenMdp<ValueType>> toOpenMdp();

    private:
        boost::optional<std::string> name;
        OpenMdpManager<ValueType> &manager;
};

template class OpenMdp<storm::RationalNumber>;
template class OpenMdp<double>;

}
}
