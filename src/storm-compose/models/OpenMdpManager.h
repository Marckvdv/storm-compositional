#pragma once

#include "OpenMdp.h"

#include <unordered_map>

namespace storm {
namespace models {

template<typename ValueType>
class OpenMdp;

template<typename ValueType>
class OpenMdpManager {
    public:
    OpenMdpManager();

    std::shared_ptr<OpenMdp<ValueType>> dereference(const std::string& name);
    void setRoot(std::shared_ptr<OpenMdp<ValueType>> root);
    void setReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference);
    void addReference(const std::string& name, std::shared_ptr<OpenMdp<ValueType>> reference);

    private:
    std::shared_ptr<OpenMdp<ValueType>> root;
    std::unordered_map<std::string, std::shared_ptr<OpenMdp<ValueType>>> references;
};

}
}
