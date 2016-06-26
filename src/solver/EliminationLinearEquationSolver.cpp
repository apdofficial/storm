#include "src/solver/EliminationLinearEquationSolver.h"

#include <numeric>

#include "src/settings/SettingsManager.h"

#include "src/solver/stateelimination/StatePriorityQueue.h"
#include "src/solver/stateelimination/PrioritizedStateEliminator.h"

#include "src/utility/graph.h"
#include "src/utility/macros.h"
#include "src/utility/vector.h"
#include "src/utility/stateelimination.h"

namespace storm {
    namespace solver {
        
        using namespace stateelimination;
        using namespace storm::utility::stateelimination;
        
        template<typename ValueType>
        EliminationLinearEquationSolverSettings<ValueType>::EliminationLinearEquationSolverSettings() {
            order = storm::settings::getModule<storm::settings::modules::EliminationSettings>().getEliminationOrder();
        }
        
        template<typename ValueType>
        void EliminationLinearEquationSolverSettings<ValueType>::setEliminationOrder(storm::settings::modules::EliminationSettings::EliminationOrder const& order) {
            this->order = order;
        }
        
        template<typename ValueType>
        storm::settings::modules::EliminationSettings::EliminationOrder EliminationLinearEquationSolverSettings<ValueType>::getEliminationOrder() const {
            return order;
        }
        
        template<typename ValueType>
        EliminationLinearEquationSolver<ValueType>::EliminationLinearEquationSolver(storm::storage::SparseMatrix<ValueType> const& A, EliminationLinearEquationSolverSettings<ValueType> const& settings) : localA(nullptr), A(A), settings(settings) {
            // Intentionally left empty.
        }
        
        template<typename ValueType>
        EliminationLinearEquationSolver<ValueType>::EliminationLinearEquationSolver(storm::storage::SparseMatrix<ValueType>&& A, EliminationLinearEquationSolverSettings<ValueType> const& settings) : localA(std::make_unique<storm::storage::SparseMatrix<ValueType>>(std::move(A))), A(*localA), settings(settings) {
            // Intentionally left empty.
        }
        
        template<typename ValueType>
        void EliminationLinearEquationSolver<ValueType>::solveEquationSystem(std::vector<ValueType>& x, std::vector<ValueType> const& b, std::vector<ValueType>* multiplyResult) const {
            STORM_LOG_WARN_COND(multiplyResult == nullptr, "Providing scratch memory will not improve the performance of this solver.");
            
            // FIXME: This solver will not work for all input systems. More concretely, the current implementation will
            // not work for systems that have a 0 on the diagonal. This is not a restriction of this technique in general
            // but arbitrary matrices require pivoting, which is not currently implemented.
            
            STORM_LOG_DEBUG("Solving equation system using elimination.");
            
            // We need to revert the transformation into an equation system matrix, because the elimination procedure
            // and the distance computation is based on the probability matrix instead.
            storm::storage::SparseMatrix<ValueType> locallyConvertedMatrix;
            if (localA) {
                localA->convertToEquationSystem();
            } else {
                locallyConvertedMatrix = A;
                locallyConvertedMatrix.convertToEquationSystem();
            }
            
            storm::storage::SparseMatrix<ValueType> const& transitionMatrix = localA ? *localA : locallyConvertedMatrix;
            storm::storage::SparseMatrix<ValueType> backwardTransitions = transitionMatrix.transpose();
            
            // Initialize the solution to the right-hand side of the equation system.
            x = b;
            
            // Translate the matrix and its transpose into the flexible format.
            storm::storage::FlexibleSparseMatrix<ValueType> flexibleMatrix(transitionMatrix, false);
            storm::storage::FlexibleSparseMatrix<ValueType> flexibleBackwardTransitions(backwardTransitions, true);
            
            boost::optional<std::vector<uint_fast64_t>> distanceBasedPriorities;
            auto order = this->getSettings().getEliminationOrder();
            if (eliminationOrderNeedsDistances(order)) {
                // Since we have no initial states at this point, we determine a representative of every BSCC regarding
                // the backward transitions, because this means that every row is reachable from this set of rows, which
                // we require to make sure we cover every row.
                storm::storage::BitVector initialRows = storm::utility::graph::getBsccCover(backwardTransitions);
                distanceBasedPriorities = getDistanceBasedPriorities(transitionMatrix, backwardTransitions, initialRows, b, eliminationOrderNeedsForwardDistances(order), eliminationOrderNeedsReversedDistances(order));
            }
            
            std::shared_ptr<StatePriorityQueue> priorityQueue = createStatePriorityQueue<ValueType>(distanceBasedPriorities, flexibleMatrix, flexibleBackwardTransitions, b, storm::storage::BitVector(x.size(), true));
            
            // Create a state eliminator to perform the actual elimination.
            PrioritizedStateEliminator<ValueType> eliminator(flexibleMatrix, flexibleBackwardTransitions, priorityQueue, x);
            
            // Eliminate all states.
            while (priorityQueue->hasNext()) {
                auto state = priorityQueue->pop();
                eliminator.eliminateState(state, false);
            }
            
            // After having solved the system, we need to revert the transition system if we kept it local.
            if (localA) {
                localA->convertToEquationSystem();
            };
        }
        
        template<typename ValueType>
        void EliminationLinearEquationSolver<ValueType>::performMatrixVectorMultiplication(std::vector<ValueType>& x, std::vector<ValueType> const* b, uint_fast64_t n, std::vector<ValueType>* multiplyResult) const {
            // Set up some temporary variables so that we can just swap pointers instead of copying the result after
            // each iteration.
            std::vector<ValueType>* currentX = &x;
            
            bool multiplyResultProvided = true;
            std::vector<ValueType>* nextX = multiplyResult;
            if (nextX == nullptr) {
                nextX = new std::vector<ValueType>(x.size());
                multiplyResultProvided = false;
            }
            std::vector<ValueType> const* copyX = nextX;
            
            // Now perform matrix-vector multiplication as long as we meet the bound.
            for (uint_fast64_t i = 0; i < n; ++i) {
                A.multiplyWithVector(*currentX, *nextX);
                std::swap(nextX, currentX);
                
                // If requested, add an offset to the current result vector.
                if (b != nullptr) {
                    storm::utility::vector::addVectors(*currentX, *b, *currentX);
                }
            }
            
            // If we performed an odd number of repetitions, we need to swap the contents of currentVector and x,
            // because the output is supposed to be stored in the input vector x.
            if (currentX == copyX) {
                std::swap(x, *currentX);
            }
            
            // If the vector for the temporary multiplication result was not provided, we need to delete it.
            if (!multiplyResultProvided) {
                delete copyX;
            }
        }
        
        template<typename ValueType>
        void EliminationLinearEquationSolver<ValueType>::performMatrixVectorMultiplication(std::vector<ValueType>& x, std::vector<ValueType>& result, std::vector<ValueType> const* b) const {
            if (&x != &result) {
                A.multiplyWithVector(x, result);
                if (b != nullptr) {
                    storm::utility::vector::addVectors(result, *b, result);
                }
            } else {
                // If the two vectors are aliases, we need to create a temporary.
                std::vector<ValueType> tmp(result.size());
                A.multiplyWithVector(x, tmp);
                if (b != nullptr) {
                    storm::utility::vector::addVectors(tmp, *b, result);
                }
            }
        }
        
        template<typename ValueType>
        EliminationLinearEquationSolverSettings<ValueType>& EliminationLinearEquationSolver<ValueType>::getSettings() {
            return settings;
        }
        
        template<typename ValueType>
        EliminationLinearEquationSolverSettings<ValueType> const& EliminationLinearEquationSolver<ValueType>::getSettings() const {
            return settings;
        }
        
        template<typename ValueType>
        std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> EliminationLinearEquationSolverFactory<ValueType>::create(storm::storage::SparseMatrix<ValueType> const& matrix) const {
            return std::make_unique<storm::solver::EliminationLinearEquationSolver<ValueType>>(matrix, settings);
        }
        
        template<typename ValueType>
        std::unique_ptr<storm::solver::LinearEquationSolver<ValueType>> EliminationLinearEquationSolverFactory<ValueType>::create(storm::storage::SparseMatrix<ValueType>&& matrix) const {
            return std::make_unique<storm::solver::EliminationLinearEquationSolver<ValueType>>(std::move(matrix), settings);
        }
        
        template<typename ValueType>
        EliminationLinearEquationSolverSettings<ValueType>& EliminationLinearEquationSolverFactory<ValueType>::getSettings() {
            return settings;
        }
        
        template<typename ValueType>
        EliminationLinearEquationSolverSettings<ValueType> const& EliminationLinearEquationSolverFactory<ValueType>::getSettings() const {
            return settings;
        }
        
        template<typename ValueType>
        std::unique_ptr<LinearEquationSolverFactory<ValueType>> EliminationLinearEquationSolverFactory<ValueType>::clone() const {
            return std::make_unique<EliminationLinearEquationSolverFactory<ValueType>>(*this);
        }
        
        template class EliminationLinearEquationSolverSettings<double>;
        template class EliminationLinearEquationSolverSettings<storm::RationalNumber>;
        template class EliminationLinearEquationSolverSettings<storm::RationalFunction>;
        
        template class EliminationLinearEquationSolver<double>;
        template class EliminationLinearEquationSolver<storm::RationalNumber>;
        template class EliminationLinearEquationSolver<storm::RationalFunction>;
        
        template class EliminationLinearEquationSolverFactory<double>;
        template class EliminationLinearEquationSolverFactory<storm::RationalNumber>;
        template class EliminationLinearEquationSolverFactory<storm::RationalFunction>;
    }
}

