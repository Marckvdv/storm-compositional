#include <exceptions/IllegalFunctionCallException.h>
#include "storm/storage/expressions/Variable.h"
#include "storm/storage/expressions/ExpressionManager.h"

namespace storm {
    namespace expressions {
        Variable::Variable() : manager(nullptr) {
            // Intentionally left empty.
        }

        Variable::Variable(std::shared_ptr<ExpressionManager const> const& manager, uint_fast64_t index) : manager(manager.get()), index(index) {
            // Intentionally left empty.
        }

        Variable::~Variable() {
            // Intentionally left empty.
        }

        bool Variable::operator==(Variable const& other) const {
#ifndef NDEBUG
            return &this->getManager() == &other.getManager() && index == other.index;
#else
            return index == other.index;
#endif
        }

        bool Variable::operator!=(Variable const& other) const {
            return !(*this == other);
        }

        bool Variable::operator<(Variable const& other) const {
            return this->getIndex() < other.getIndex();
        }

        storm::expressions::Expression Variable::getExpression() const {
            return storm::expressions::Expression(*this);
        }

        uint_fast64_t Variable::getIndex() const {
            return index;
        }

        uint_fast64_t Variable::getOffset() const {
            return this->getManager().getOffset(index);
        }

        std::string const& Variable::getName() const {
            return this->getManager().getVariableName(index);
        }

        Type const& Variable::getType() const {
            return this->getManager().getVariableType(index);
        }

        ExpressionManager const& Variable::getManager() const {
            STORM_LOG_ASSERT(manager != nullptr, "The variable does not have a manager.");
            return *manager;
        }

        bool Variable::hasBooleanType() const {
            return this->getType().isBooleanType();
        }

        bool Variable::hasIntegerType() const {
            return this->getType().isIntegerType();
        }

        bool Variable::hasBitVectorType() const {
            return this->getType().isBitVectorType();
        }

        bool Variable::hasRationalType() const {
            return this->getType().isRationalType();
        }

        bool Variable::hasNumericalType() const {
            return this->getType().isNumericalType();
        }

        void Variable::setArraySizes(std::vector<std::size_t> & arraySizes) {
            STORM_LOG_THROW(this->getType().isArrayType(), storm::exceptions::IllegalFunctionCallException, "Setting arraysize for variable that is not an array is not possible");
            this->arraySizes = arraySizes;
        }

        std::size_t Variable::getArraySize(uint_fast64_t number) const {
            STORM_LOG_THROW(this->getType().isArrayType(), storm::exceptions::IllegalFunctionCallException, "Getting arraysize for variable that is not an array is not possible");
            return this->arraySizes.at(number);
        }
    }
}
