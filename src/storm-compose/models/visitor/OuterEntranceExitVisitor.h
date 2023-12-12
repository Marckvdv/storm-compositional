#pragma once

#include "OpenMdpVisitor.h"
#include "exceptions/InvalidOperationException.h"
#include "exceptions/NotSupportedException.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class OuterEntranceExitVisitor : public OpenMdpVisitor<ValueType> {
   public:
    OuterEntranceExitVisitor(storage::EntranceExit entranceExit = storage::R_EXIT) : entranceExit(entranceExit) {
        STORM_LOG_THROW(false, storm::exceptions::InvalidOperationException, "do not use");
    }
    virtual ~OuterEntranceExitVisitor() override {}

    void setEntranceExit(storage::EntranceExit entranceExit) {
        this->entranceExit = entranceExit;
        reset();
    }

    void reset() {
        currentComponentId = 0;
        collect = true;
    }

    virtual void visitPrismModel(PrismModel<ValueType>& model) override {}

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override {
        if (collect) {
            std::vector<size_t> const* entries;
            if (entranceExit == storage::L_ENTRANCE)
                entries = &model.getLEntrance();
            if (entranceExit == storage::R_ENTRANCE)
                entries = &model.getREntrance();
            if (entranceExit == storage::L_EXIT)
                entries = &model.getLExit();
            if (entranceExit == storage::R_EXIT)
                entries = &model.getRExit();

            for (size_t i = 0; i < entries->size(); ++i) {
                weightMapping.insert({{currentComponentId, {entranceExit, i}}, weightMapping.size()});
            }
        }
        ++currentComponentId;
    }

    virtual void visitReference(Reference<ValueType>& reference) override {
        auto openMdp = reference.getManager()->dereference(reference.getReference());
        openMdp->accept(*this);
    }

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override {
        auto& values = model.getValues();

        if (collect) {
            for (size_t i = 0; i < values.size(); ++i) {
                const auto& v = values[i];
                collect = ((entranceExit == storage::L_ENTRANCE || entranceExit == storage::L_EXIT) && i == 0) ||
                          ((entranceExit == storage::R_ENTRANCE || entranceExit == storage::R_EXIT) && i == values.size() - 1);
                v->accept(*this);
            }
        } else {
            for (auto& v : values) {
                v->accept(*this);
            }
        }
    }

    virtual void visitSumModel(SumModel<ValueType>& model) override {
        auto& values = model.getValues();
        for (auto& v : values) {
            v->accept(*this);
        }
    }

    virtual void visitTraceModel(TraceModel<ValueType>& model) override {
        STORM_LOG_THROW(false, storm::exceptions::NotSupportedException, "Trace currently not supported");
    }

    std::map<std::pair<size_t, storage::Position>, size_t>& getWeightMapping() {
        return weightMapping;
    }

   private:
    bool collect = true;
    std::map<std::pair<size_t, storage::Position>, size_t> weightMapping;
    storage::EntranceExit entranceExit;
    Scope scope = {};
    size_t currentComponentId = 0;
};

template class OuterEntranceExitVisitor<double>;
template class OuterEntranceExitVisitor<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
