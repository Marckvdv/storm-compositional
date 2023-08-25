#pragma once

#include "OpenMdpVisitor.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SumModel.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class OpenMdpPrintVisitor : public OpenMdpVisitor<ValueType> {
    public:
    OpenMdpPrintVisitor(std::ostream& out) : out(out), scope() {

    }

    virtual ~OpenMdpPrintVisitor() {}

    void printIndentation(typename OpenMdp<ValueType>::Scope& scope) {
        for (size_t i = 0; i < scope.scope.size(); ++i) {
            out << '\t';
        }
    }

    virtual void visitPrismModel(PrismModel<ValueType>& model) override {
        printIndentation(scope);
        out << "PRISM(\"" << model.getPath() << "\")" << std::endl;
    }

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override {
        printIndentation(scope);
        out << "CONCRETE " << model.getMdp()->getNumberOfStates() << " STATES(";
        if (model.lEntrance.size() > 0)
            out << ">| " << model.lEntrance.size() << ",";
        if (model.rEntrance.size() > 0)
            out << "|< " << model.rEntrance.size() << ",";
        if (model.lExit.size() > 0)
            out << "<| " << model.lExit.size() << ",";
        if (model.rExit.size() > 0)
            out << "|> " << model.rExit.size();
        out << ")";
    }

    virtual void visitReference(Reference<ValueType>& reference) override {
        out << std::endl;
        printIndentation(scope);
        out << reference.getReference() << " (=";
        auto pointsTo = reference.getManager()->dereference(reference.getReference());
        pointsTo->accept(*this);
        out << ")";
    }

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override {
        printIndentation(scope);
        out << "SEQUENCE(";
        size_t i = 0;
        for (const auto& m : model.values) {
            scope.pushScope(i++);
            m->accept(*this);
            scope.popScope();
        }
        out << ")";
    }

    virtual void visitSumModel(SumModel<ValueType>& model) override {
        printIndentation(scope);
        out << "SUM(";
        size_t i = 0;
        for (const auto& m : model.values) {
            scope.pushScope(i++);
            m->accept(*this);
            scope.popScope();
        }
        out << ")";
    }

    private:
    std::ostream& out;
    typename OpenMdp<ValueType>::Scope scope;
};

template class OpenMdpPrintVisitor<double>;
template class OpenMdpPrintVisitor<storm::RationalNumber>;

}
}
}
