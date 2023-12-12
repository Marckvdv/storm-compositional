#pragma once

#include <utility>
#include "OpenMdpVisitor.h"
#include "storm-compose/models/ConcreteMdp.h"
#include "storm-compose/models/PrismModel.h"
#include "storm-compose/models/Reference.h"
#include "storm-compose/models/SequenceModel.h"
#include "storm-compose/models/SumModel.h"
#include "storm-compose/models/TraceModel.h"
#include "storm-compose/models/visitor/EntranceExitVisitor.h"

namespace storm {
namespace models {
namespace visitor {

template<typename ValueType>
class OpenMdpToDotVisitor : public OpenMdpVisitor<ValueType> {
   public:
    OpenMdpToDotVisitor(std::ostream& out) : out(out), scope() {}

    virtual ~OpenMdpToDotVisitor() {}

    std::string printScope(const models::visitor::Scope& scope) {
        std::stringstream result;
        result << "s";
        if (scope.scope.size() > 0) {
            result << scope.scope[0];
            for (size_t i = 1; i < scope.scope.size(); ++i) {
                result << "_" << scope.scope[i];
            }
        }
        return result.str();
    }

    std::string printNode(const models::visitor::Scope& scope, models::visitor::EntranceExit entranceExit) {
        std::stringstream result;
        switch (entranceExit) {
            case models::visitor::L_ENTRANCE:
                result << "lEn";
                break;
            case models::visitor::R_ENTRANCE:
                result << "rEn";
                break;
            case models::visitor::L_EXIT:
                result << "lEx";
                break;
            case models::visitor::R_EXIT:
                result << "rEx";
                break;
        }
        result << printScope(scope);
        return result.str();
    }

    std::string printTransition(const models::visitor::Scope& fromScope, models::visitor::EntranceExit fromEntranceExit,
                                const models::visitor::Scope& toScope, models::visitor::EntranceExit toEntranceExit,
                                bool mirrorDir = false) {
        std::stringstream result;
        if (!mirrorDir) {
            result << printNode(fromScope, fromEntranceExit);
            result << " -> ";
            result << printNode(toScope, toEntranceExit);
        } else {
            result << printNode(toScope, toEntranceExit);
            result << " -> ";
            result << printNode(fromScope, fromEntranceExit);
            result << "[dir=back]";
        }
        result << ";\n";
        return result.str();
    }

    std::string printColor(const models::visitor::Scope& scope) {
        size_t depth = scope.scope.size();
        float brightness = 1.0 / (depth + 1);
        uint8_t red = (uint8_t)255.f * (brightness);
        uint8_t green = (uint8_t)255.f * (1.f - brightness);
        uint8_t blue = (uint8_t)255.f * (1.f - brightness);

        std::stringstream result;
        result << "\"#";
        result << std::hex;
        result << std::setw(2) << std::setfill('0') << (int)red;
        result << std::setw(2) << std::setfill('0') << (int)green;
        result << std::setw(2) << std::setfill('0') << (int)blue;
        result << "\"";

        return result.str();
    }

    void visitRoot(OpenMdp<ValueType>& model) {
        out << "digraph {" << std::endl;
        model.accept(*this);
        out << "}" << std::endl;
    }

    virtual void visitPrismModel(PrismModel<ValueType>& model) override {}

    virtual void visitConcreteModel(ConcreteMdp<ValueType>& model) override {
        out << "subgraph cluster_" << printScope(scope) << "{" << std::endl;

        auto drawEntranceExit = [&](auto& entranceExit, std::string text, std::string extra = "") {
            for (size_t i = 0; i < entranceExit.size(); ++i) {
                scope.pushScope(i);
                out << text << printScope(scope);
                out << "[label=\"" << i << text << "\"" << extra << "];" << std::endl;
                scope.popScope();
            }
        };

        drawEntranceExit(model.getLEntrance(), "lEn", ", style=filled, fillcolor=red");
        drawEntranceExit(model.getLExit(), "lEx", ", style=filled, fillcolor=blue");
        drawEntranceExit(model.getREntrance(), "rEn", ", style=filled, fillcolor=lightblue");
        drawEntranceExit(model.getRExit(), "rEx", ", style=filled, fillcolor=orange");

        auto drawArrowsFromEntranceToExit = [&](auto& entrance, auto& exit, std::string entranceText, std::string exitText) {
            for (size_t i = 0; i < entrance.size(); ++i) {
                for (size_t j = 0; j < exit.size(); ++j) {
                    scope.pushScope(i);
                    out << entranceText << printScope(scope) << "->";
                    scope.popScope();
                    scope.pushScope(j);
                    out << exitText << printScope(scope) << "[style=invis];" << std::endl;
                    scope.popScope();
                }
            }
        };

        drawArrowsFromEntranceToExit(model.getLEntrance(), model.getLExit(), "lEn", "lEx");
        drawArrowsFromEntranceToExit(model.getLEntrance(), model.getRExit(), "lEn", "rEx");
        drawArrowsFromEntranceToExit(model.getREntrance(), model.getLExit(), "rEn", "lEx");
        drawArrowsFromEntranceToExit(model.getREntrance(), model.getRExit(), "rEn", "rEx");

        out << "label = mdp" << clusterCount << ";" << std::endl;
        out << "style = filled;" << std::endl;
        out << "fillcolor = " << printColor(scope);
        out << ";\n}" << std::endl << std::endl;
        clusterCount++;
    }

    virtual void visitSequenceModel(SequenceModel<ValueType>& model) override {
        out << "subgraph cluster_" << printScope(scope) << "{" << std::endl;

        size_t i = 0;
        for (const auto& m : model.getValues()) {
            scope.pushScope(i++);
            m->accept(*this);
            scope.popScope();
        }

        for (size_t i = 0; i < model.getValues().size() - 1; ++i) {
            const auto& lhs = model.getValues()[i];
            const auto& rhs = model.getValues()[i + 1];

            // rExit -> lEntrance
            scope.pushScope(i);
            auto lhsRExits = lhs->collectEntranceExit(OpenMdp<ValueType>::R_EXIT, scope);
            scope.popScope();
            scope.pushScope(i + 1);
            auto rhsLEntrances = rhs->collectEntranceExit(OpenMdp<ValueType>::L_ENTRANCE, scope);
            scope.popScope();
            STORM_LOG_ASSERT(lhsRExits.size() == rhsLEntrances.size(), "arity mismatch " << lhsRExits.size() << " vs " << rhsLEntrances.size());
            for (size_t j = 0; j < lhsRExits.size(); ++j) {
                auto rExit = lhsRExits[j];
                auto lEntrance = rhsLEntrances[j];
                out << printTransition(rExit.scope, OpenMdp<ValueType>::R_EXIT, lEntrance.scope, OpenMdp<ValueType>::L_ENTRANCE);
            }

            // rEntrance <- lExit
            scope.pushScope(i + 1);
            auto rhsLExits = rhs->collectEntranceExit(OpenMdp<ValueType>::L_EXIT, scope);
            scope.popScope();
            scope.pushScope(i);
            auto lhsREntrances = lhs->collectEntranceExit(OpenMdp<ValueType>::R_ENTRANCE, scope);
            scope.popScope();
            STORM_LOG_ASSERT(rhsLExits.size() == lhsREntrances.size(), "arity mismatch");
            for (size_t j = 0; j < rhsLExits.size(); ++j) {
                auto lExit = rhsLExits[j];
                auto rEntrance = lhsREntrances[j];

                out << printTransition(lExit.scope, OpenMdp<ValueType>::L_EXIT, rEntrance.scope, OpenMdp<ValueType>::R_ENTRANCE, true);
            }
        }

        out << "label = sequence;" << std::endl;
        out << "style = filled;" << std::endl;
        out << "fillcolor = ";
        out << printColor(scope);
        out << ";\n}" << std::endl;
    }

    virtual void visitSumModel(SumModel<ValueType>& model) override {
        out << "subgraph cluster_" << printScope(scope) << "{" << std::endl;

        size_t i = 0;
        for (const auto& m : model.getValues()) {
            scope.pushScope(i++);
            m->accept(*this);
            scope.popScope();
        }

        out << "label = sum;" << std::endl;
        out << "style = filled;" << std::endl;
        out << "fillcolor = " << printColor(scope);
        out << ";\n}" << std::endl;
    }

    virtual void visitTraceModel(TraceModel<ValueType>& model) override {
        model.value->accept(*this);

        auto lEntrances = model.value->collectEntranceExit(OpenMdp<ValueType>::L_ENTRANCE, scope);
        auto rEntrances = model.value->collectEntranceExit(OpenMdp<ValueType>::R_ENTRANCE, scope);
        auto lExits = model.value->collectEntranceExit(OpenMdp<ValueType>::L_EXIT, scope);
        auto rExits = model.value->collectEntranceExit(OpenMdp<ValueType>::R_EXIT, scope);

        STORM_LOG_ASSERT(model.left <= rEntrances.size() && model.left <= lExits.size(), "left greater than available entrance/exits");
        STORM_LOG_ASSERT(model.right <= lEntrances.size() && model.right <= rExits.size(), "right greater than available entrance/exits");

        for (size_t i = 0; i < model.left; ++i) {
            out << printTransition(lExits[i].scope, OpenMdp<ValueType>::L_EXIT, rEntrances[i].scope, OpenMdp<ValueType>::R_ENTRANCE, true);
        }

        for (size_t i = 0; i < model.right; ++i) {
            out << printTransition(rExits[i].scope, OpenMdp<ValueType>::R_EXIT, lEntrances[i].scope, OpenMdp<ValueType>::L_ENTRANCE, true);
        }
    }

   private:
    std::ostream& out;
    size_t clusterCount = 0;
    typename OpenMdp<ValueType>::Scope scope;
};

template class OpenMdpToDotVisitor<double>;
template class OpenMdpToDotVisitor<storm::RationalNumber>;

}  // namespace visitor
}  // namespace models
}  // namespace storm
