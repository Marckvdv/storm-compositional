#pragma once

#include <boost/optional.hpp>
#include <memory>
#include <string>
#include <vector>

#include "OpenMdpManager.h"
#include "storage/geometry/Polytope.h"
#include "storm-compose/models/visitor/BidirectionalReachabilityResult.h"
#include "storm-compose/models/visitor/OpenMdpVisitor.h"
#include "storm/adapters/RationalNumberAdapter.h"

/*
 What is an OpenMdp?

 It will be the abstract base class for everything which is part of an OpenMdp.
 In Haskell style pseudocode:

 data OpenMdp =
   ConcreteMdp([State], [State], [State], [State]) |
   Sum([OpenMdp]) |
   Sequence([OpenMdp]) |
   Trace(OpenMdp, Int, Int) |
   Reference(String)

 Additionally, everything can be named so that we can use references (by name)
*/

namespace storm {
namespace models {

template<typename ValueType>
class OpenMdpManager;

template<typename ValueType>
class ConcreteMdp;

namespace visitor {
template<typename ValueType>
class OpenMdpVisitor;
template<typename ValueType>
class BidirectionalReachabilityResult;
}  // namespace visitor

// template<typename ValueType>
// class Reference;

template<typename ValueType>
class OpenMdp : public std::enable_shared_from_this<OpenMdp<ValueType>> {
   public:
    virtual ~OpenMdp() = 0;
    OpenMdp(std::weak_ptr<OpenMdpManager<ValueType>> manager);

    bool hasName();
    std::string getName();
    void setName(std::string const& name);

    /// Follow references until a non-reference type is found
    /// Does not check for loops
    std::shared_ptr<OpenMdp<ValueType>> followReferences();
    std::shared_ptr<OpenMdpManager<ValueType>> getManager();
    std::shared_ptr<OpenMdpManager<ValueType>> const getManager() const;

    virtual bool isConcreteMdp() const;
    virtual bool isSum() const;
    virtual bool isSequence() const;
    virtual bool isTrace() const;
    virtual bool isReference() const;
    virtual bool isPrismModel() const;

    std::shared_ptr<OpenMdp<ValueType>> toOpenMdp();
    virtual bool isRightward() const = 0;
    virtual void initializeParetoCurve();

    // visitor pattern for easier recursion of the OpenMdp structure
    virtual void accept(visitor::OpenMdpVisitor<ValueType>& visitor) = 0;

   protected:
    boost::optional<std::string> name;
    std::weak_ptr<OpenMdpManager<ValueType>> manager;

    typedef storm::storage::geometry::Polytope<ValueType> PolytopeType;
    boost::optional<std::pair<std::shared_ptr<PolytopeType>, std::shared_ptr<PolytopeType>>> paretoCurve;
};

}  // namespace models
}  // namespace storm
