#include "src/modelchecker/prctl/helper/SparseDtmcPrctlHelper.h"

#include "src/modelchecker/csl/helper/SparseCtmcCslHelper.h"

#include "src/utility/macros.h"
#include "src/utility/vector.h"
#include "src/utility/graph.h"


#include "src/solver/LinearEquationSolver.h"

#include "src/exceptions/InvalidStateException.h"
#include "src/exceptions/InvalidPropertyException.h"

namespace storm {
    namespace modelchecker {
        namespace helper {
            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeBoundedUntilProbabilities(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, uint_fast64_t stepBound, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                std::vector<ValueType> result(transitionMatrix.getRowCount(), storm::utility::zero<ValueType>());
                
                // If we identify the states that have probability 0 of reaching the target states, we can exclude them in the further analysis.
                storm::storage::BitVector maybeStates = storm::utility::graph::performProbGreater0(backwardTransitions, phiStates, psiStates, true, stepBound);
                maybeStates &= ~psiStates;
                STORM_LOG_INFO("Found " << maybeStates.getNumberOfSetBits() << " 'maybe' states.");
                
                if (!maybeStates.empty()) {
                    // We can eliminate the rows and columns from the original transition probability matrix that have probability 0.
                    storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, maybeStates, maybeStates, true);
                    
                    // Create the vector of one-step probabilities to go to target states.
                    std::vector<ValueType> b = transitionMatrix.getConstrainedRowSumVector(maybeStates, psiStates);
                    
                    // Create the vector with which to multiply.
                    std::vector<ValueType> subresult(maybeStates.getNumberOfSetBits());
                    
                    // Perform the matrix vector multiplication as often as required by the formula bound.
                    std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> solver = linearEquationSolverFactory.create(submatrix);
                    solver->performMatrixVectorMultiplication(subresult, &b, stepBound);
                    
                    // Set the values of the resulting vector accordingly.
                    storm::utility::vector::setVectorValues(result, maybeStates, subresult);
                }
                storm::utility::vector::setVectorValues<ValueType>(result, psiStates, storm::utility::one<ValueType>());
                
                return result;
            }
        
            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeUntilProbabilities(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, storm::storage::BitVector const& phiStates, storm::storage::BitVector const& psiStates, bool qualitative, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                // We need to identify the states which have to be taken out of the matrix, i.e.
                // all states that have probability 0 and 1 of satisfying the until-formula.
                std::pair<storm::storage::BitVector, storm::storage::BitVector> statesWithProbability01 = storm::utility::graph::performProb01(backwardTransitions, phiStates, psiStates);
                storm::storage::BitVector statesWithProbability0 = std::move(statesWithProbability01.first);
                storm::storage::BitVector statesWithProbability1 = std::move(statesWithProbability01.second);
                
                // Perform some logging.
                storm::storage::BitVector maybeStates = ~(statesWithProbability0 | statesWithProbability1);
                STORM_LOG_INFO("Found " << statesWithProbability0.getNumberOfSetBits() << " 'no' states.");
                STORM_LOG_INFO("Found " << statesWithProbability1.getNumberOfSetBits() << " 'yes' states.");
                STORM_LOG_INFO("Found " << maybeStates.getNumberOfSetBits() << " 'maybe' states.");
                
                // Create resulting vector.
                std::vector<ValueType> result(transitionMatrix.getRowCount());
                
                // Check whether we need to compute exact probabilities for some states.
                if (qualitative) {
                    // Set the values for all maybe-states to 0.5 to indicate that their probability values are neither 0 nor 1.
                    storm::utility::vector::setVectorValues<ValueType>(result, maybeStates, ValueType(0.5));
                } else {
                    if (!maybeStates.empty()) {
                        // In this case we have have to compute the probabilities.
                        
                        // We can eliminate the rows and columns from the original transition probability matrix.
                        storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, maybeStates, maybeStates, true);
                        
                        // Converting the matrix from the fixpoint notation to the form needed for the equation
                        // system. That is, we go from x = A*x + b to (I-A)x = b.
                        submatrix.convertToEquationSystem();
                        
                        // Initialize the x vector with 0.5 for each element. This is the initial guess for
                        // the iterative solvers. It should be safe as for all 'maybe' states we know that the
                        // probability is strictly larger than 0.
                        std::vector<ValueType> x(maybeStates.getNumberOfSetBits(), ValueType(0.5));
                        
                        // Prepare the right-hand side of the equation system. For entry i this corresponds to
                        // the accumulated probability of going from state i to some 'yes' state.
                        std::vector<ValueType> b = transitionMatrix.getConstrainedRowSumVector(maybeStates, statesWithProbability1);
                        
                        // Now solve the created system of linear equations.
                        std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> solver = linearEquationSolverFactory.create(submatrix);
                        solver->solveEquationSystem(x, b);
                        
                        // Set values of resulting vector according to result.
                        storm::utility::vector::setVectorValues<ValueType>(result, maybeStates, x);
                    }
                }
                
                // Set values of resulting vector that are known exactly.
                storm::utility::vector::setVectorValues<ValueType>(result, statesWithProbability0, storm::utility::zero<ValueType>());
                storm::utility::vector::setVectorValues<ValueType>(result, statesWithProbability1, storm::utility::one<ValueType>());
                
                return result;
            }
            
            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeNextProbabilities(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector const& nextStates, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                // Create the vector with which to multiply and initialize it correctly.
                std::vector<ValueType> result(transitionMatrix.getRowCount());
                storm::utility::vector::setVectorValues(result, nextStates, storm::utility::one<ValueType>());
                
                // Perform one single matrix-vector multiplication.
                std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> solver = linearEquationSolverFactory.create(transitionMatrix);
                solver->performMatrixVectorMultiplication(result);
                return result;
            }
            
            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeCumulativeRewards(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, RewardModelType const& rewardModel, uint_fast64_t stepBound, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                // Compute the reward vector to add in each step based on the available reward models.
                std::vector<ValueType> totalRewardVector = rewardModel.getTotalRewardVector(transitionMatrix);
                
                // Initialize result to either the state rewards of the model or the null vector.
                std::vector<ValueType> result = rewardModel.getTotalStateActionRewardVector(transitionMatrix.getRowCount(), transitionMatrix.getRowGroupIndices());
                
                // Perform the matrix vector multiplication as often as required by the formula bound.
                std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> solver = linearEquationSolverFactory.create(transitionMatrix);
                solver->performMatrixVectorMultiplication(result, &totalRewardVector, stepBound);
                
                return result;
            }
            
            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeInstantaneousRewards(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, RewardModelType const& rewardModel, uint_fast64_t stepCount, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                // Only compute the result if the model has a state-based reward this->getModel().
                STORM_LOG_THROW(rewardModel.hasStateRewards() || rewardModel.hasStateActionRewards(), storm::exceptions::InvalidPropertyException, "Missing reward model for formula. Skipping formula.");
                
                // Initialize result to state rewards of the model.
                std::vector<ValueType> result = rewardModel.getTotalStateActionRewardVector(transitionMatrix.getRowCount(), transitionMatrix.getRowGroupIndices());
                
                // Perform the matrix vector multiplication as often as required by the formula bound.
                std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> solver = linearEquationSolverFactory.create(transitionMatrix);
                solver->performMatrixVectorMultiplication(result, nullptr, stepCount);
                
                return result;
            }

            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeReachabilityRewards(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, RewardModelType const& rewardModel, storm::storage::BitVector const& targetStates, bool qualitative, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                return computeReachabilityRewards(transitionMatrix, backwardTransitions, [&] (uint_fast64_t numberOfRows, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector const& maybeStates) { return rewardModel.getTotalRewardVector(numberOfRows, transitionMatrix, maybeStates); }, targetStates, qualitative, linearEquationSolverFactory);
            }
            
            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeReachabilityRewards(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, std::vector<ValueType> const& totalStateRewardVector, storm::storage::BitVector const& targetStates, bool qualitative, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                return computeReachabilityRewards(transitionMatrix, backwardTransitions,
                                                  [&] (uint_fast64_t numberOfRows, storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector const& maybeStates) {
                                                      std::vector<ValueType> result(numberOfRows);
                                                      storm::utility::vector::selectVectorValues(result, maybeStates, totalStateRewardVector);
                                                      return result;
                                                  },
                                                  targetStates, qualitative, linearEquationSolverFactory);
            }
          
            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeReachabilityRewards(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::SparseMatrix<ValueType> const& backwardTransitions, std::function<std::vector<ValueType>(uint_fast64_t, storm::storage::SparseMatrix<ValueType> const&, storm::storage::BitVector const&)> const& totalStateRewardVectorGetter, storm::storage::BitVector const& targetStates, bool qualitative, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                                
                // Determine which states have a reward of infinity by definition.
                storm::storage::BitVector trueStates(transitionMatrix.getRowCount(), true);
                storm::storage::BitVector infinityStates = storm::utility::graph::performProb1(backwardTransitions, trueStates, targetStates);
                infinityStates.complement();
                storm::storage::BitVector maybeStates = ~targetStates & ~infinityStates;
                STORM_LOG_INFO("Found " << infinityStates.getNumberOfSetBits() << " 'infinity' states.");
                STORM_LOG_INFO("Found " << targetStates.getNumberOfSetBits() << " 'target' states.");
                STORM_LOG_INFO("Found " << maybeStates.getNumberOfSetBits() << " 'maybe' states.");
                
                // Create resulting vector.
                std::vector<ValueType> result(transitionMatrix.getRowCount(), storm::utility::zero<ValueType>());
                
                // Check whether we need to compute exact rewards for some states.
                if (qualitative) {
                    // Set the values for all maybe-states to 1 to indicate that their reward values
                    // are neither 0 nor infinity.
                    storm::utility::vector::setVectorValues<ValueType>(result, maybeStates, storm::utility::one<ValueType>());
                } else {
                    if (!maybeStates.empty()) {
                        // In this case we have to compute the reward values for the remaining states.
                        // We can eliminate the rows and columns from the original transition probability matrix.
                        storm::storage::SparseMatrix<ValueType> submatrix = transitionMatrix.getSubmatrix(true, maybeStates, maybeStates, true);
                        
                        // Converting the matrix from the fixpoint notation to the form needed for the equation
                        // system. That is, we go from x = A*x + b to (I-A)x = b.
                        submatrix.convertToEquationSystem();
                        
                        // Initialize the x vector with 1 for each element. This is the initial guess for
                        // the iterative solvers.
                        std::vector<ValueType> x(submatrix.getColumnCount(), storm::utility::one<ValueType>());
                        
                        // Prepare the right-hand side of the equation system.
                        std::vector<ValueType> b = totalStateRewardVectorGetter(submatrix.getRowCount(), transitionMatrix, maybeStates);
                        
                        // Now solve the resulting equation system.
                        std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> solver = linearEquationSolverFactory.create(submatrix);
                        solver->solveEquationSystem(x, b);
                        
                        // Set values of resulting vector according to result.
                        storm::utility::vector::setVectorValues<ValueType>(result, maybeStates, x);
                    }
                }
                
                // Set values of resulting vector that are known exactly.
                storm::utility::vector::setVectorValues(result, infinityStates, storm::utility::infinity<ValueType>());
                
                return result;
            }

            template<typename ValueType, typename RewardModelType>
            std::vector<ValueType> SparseDtmcPrctlHelper<ValueType, RewardModelType>::computeLongRunAverage(storm::storage::SparseMatrix<ValueType> const& transitionMatrix, storm::storage::BitVector const& psiStates, bool qualitative, storm::utility::solver::LinearEquationSolverFactory<ValueType> const& linearEquationSolverFactory) {
                return SparseCtmcCslHelper<ValueType>::computeLongRunAverage(transitionMatrix, psiStates, nullptr, qualitative, linearEquationSolverFactory);
            }

            template class SparseDtmcPrctlHelper<double>;
        }
    }
}