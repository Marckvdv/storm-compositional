#pragma once

#include "OpenMdp.h"
#include "storm/models/sparse/Mdp.h"

#include <unordered_map>

namespace storm {
namespace models {

template<typename ValueType>
class OpenMdp;

template<typename ValueType>
class OpenMdpManager {
    public:
    OpenMdpManager();

    std::shared_ptr<OpenMdp<ValueType>> dereference(const std::string& name) const;
    void setRoot(std::shared_ptr<OpenMdp<ValueType>> root);
    std::shared_ptr<OpenMdp<ValueType>> getRoot() const;
    void setReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference);
    void addReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference);
    void constructConcreteMdps();

    private:
    std::shared_ptr<OpenMdp<ValueType>> root;
    std::unordered_map<std::string, std::shared_ptr<OpenMdp<ValueType>>> references;
};

}
}
