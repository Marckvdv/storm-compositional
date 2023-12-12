#pragma once

#include "OpenMdpVisitor.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"
#include "storm-compose/storage/EntranceExit.h"

namespace storm {
namespace models {
namespace visitor {

struct Scope {
    std::vector<size_t> scope;

    void pushScope(size_t s) {
        scope.push_back(s);
    }

    size_t popScope() {
        size_t r = scope.back();
        scope.pop_back();
        return r;
    }

    void increaseScope() {
        ++scope.back();
    }

    void decreaseScope() {
        --scope.back();
    }

    void appendScope(const Scope& other) {
        scope.insert(std::end(scope), std::begin(other.scope), std::end(other.scope));
    }

    bool operator<(Scope const& other) const {
        return scope < other.scope;
    }
};

template<typename ValueType>
class EntranceExitVisitor : public OpenMdpVisitor<ValueType> {
   public:
    struct ConcreteEntranceExit {
        ConcreteMdp<ValueType> const* mdp;
        size_t state;
        Scope scope;
    };

    EntranceExitVisitor(storage::EntranceExit entranceExit = storage::R_EXIT) : entranceExit(entranceExit) {}
    virtual ~EntranceExitVisitor() override {}

    void setEntranceExit(storage::EntranceExit entranceExit) {
        this->entranceExit = entranceExit;
        reset();
    }

    void reset() {
        collected = {};
        scope = {};
        currentComponentId = 0;
    }

    virtual void visitPrismModel(PrismModel<ValueType>& model) override {}

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override {
        std::vector<size_t> const* src = nullptr;

        if (entranceExit == storage::L_ENTRANCE)
            src = &model.getLEntrance();
        else if (entranceExit == storage::R_ENTRANCE)
            src = &model.getREntrance();
        else if (entranceExit == storage::L_EXIT)
            src = &model.getLExit();
        else if (entranceExit == storage::R_EXIT)
            src = &model.getRExit();

        // Doing some pointer magic, double check
        std::vector<ConcreteEntranceExit> entries;
        size_t i = 0;
        for (auto entry : *src) {
            scope.pushScope(i);
            ConcreteEntranceExit newEntry{&model, entry, scope};
            scope.popScope();
            entries.push_back(newEntry);
            ++i;
        }
        collected = entries;
    }

    virtual void visitReference(Reference<ValueType>& reference) override {
        auto openMdp = reference.getManager()->dereference(reference.getReference());
        openMdp->accept(*this);
    }

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override {
        auto& values = model.getValues();
        if (values.size() == 0) {
            STORM_LOG_ASSERT(false, "something went wrong");
            return;
        } else if (values.size() == 1) {
            scope.pushScope(0);
            values[0]->accept(*this);
            scope.popScope();
            return;
        }

        if (entranceExit == storage::L_ENTRANCE || entranceExit == storage::L_EXIT) {
            scope.pushScope(0);
            values[0]->accept(*this);
            scope.popScope();
        } else {
            size_t idx = values.size() - 1;
            scope.pushScope(idx);
            values[idx]->accept(*this);
            scope.popScope();
        }
    }

    virtual void visitSumModel(SumModel<ValueType>& model) override {
        auto& values = model.getValues();
        if (values.size() == 0) {
            STORM_LOG_ASSERT(false, "something went wrong");
            return;
        }

        std::vector<ConcreteEntranceExit> entries;
        size_t i = 0;
        for (const auto& v : values) {
            scope.pushScope(i);
            v->accept(*this);
            scope.popScope();
            entries.insert(std::end(entries), std::begin(collected), std::end(collected));
            ++i;
        }
        collected = entries;
    }

    virtual void visitTraceModel(TraceModel<ValueType>& model) override {
        std::vector<ConcreteEntranceExit> entries;
        size_t left = model.getLeft(), right = model.getRight();
        model.getValue()->accept(*this);
        if (entranceExit == storage::L_ENTRANCE || entranceExit == storage::R_EXIT) {
            entries.insert(std::begin(entries), std::begin(collected) + right, std::end(collected));
        } else if (entranceExit == storage::R_ENTRANCE || entranceExit == storage::L_EXIT) {
            entries.insert(std::begin(entries), std::begin(collected) + left, std::end(collected));
        }
        collected = entries;
    }

    std::vector<ConcreteEntranceExit>& getCollected() {
        return collected;
    }

   private:
    bool collectIntermediate;
    std::vector<ConcreteEntranceExit> collected;
    storage::EntranceExit entranceExit;
    Scope scope = {};
    size_t currentComponentId = 0;
};

template class EntranceExitVisitor<double>;
template class EntranceExitVisitor<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
