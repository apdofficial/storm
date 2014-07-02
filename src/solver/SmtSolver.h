#ifndef STORM_SOLVER_SMTSOLVER
#define STORM_SOLVER_SMTSOLVER

#include <cstdint>

#include "exceptions/IllegalArgumentValueException.h"
#include "exceptions/NotImplementedException.h"
#include "exceptions/IllegalArgumentTypeException.h"
#include "exceptions/IllegalFunctionCallException.h"
#include "exceptions/InvalidStateException.h"
#include "storage/expressions/Expressions.h"
#include "storage/expressions/SimpleValuation.h"

#include <set>
#include <unordered_set>
#include <initializer_list>
#include <functional>

namespace storm {
	namespace solver {

		/*!
		* An interface that captures the functionality of an SMT solver.
		*/
		class SmtSolver {
		public:
			//! Option flags for smt solvers.
			enum class Options {
				ModelGeneration = 0x01,
				UnsatCoreComputation = 0x02,
				InterpolantComputation = 0x04
			};
			//! possible check results
			enum class CheckResult { SAT, UNSAT, UNKNOWN };
		public:
			/*!
			* Constructs a new smt solver with the given options.
			*
			* @param options the options for the solver
			* @throws storm::exceptions::IllegalArgumentValueException if an option is unsupported for the solver
			*/
			SmtSolver(Options options = Options::ModelGeneration) {};
			virtual ~SmtSolver() {};

			SmtSolver(const SmtSolver&) = delete;
			SmtSolver(const SmtSolver&&) {};

			//! pushes a backtrackingpoint in the solver
			virtual void push() = 0;
			//! pops a backtrackingpoint in the solver
			virtual void pop() = 0;
			//! pops multiple backtrack points
			//! @param n number of backtrackingpoint to pop
			virtual void pop(uint_fast64_t n) = 0;
			//! removes all assertions
			virtual void reset() = 0;


			//! assert an expression in the solver
			//! @param e the asserted expression, the return type has to be bool
			//! @throws IllegalArgumentTypeException if the return type of the expression is not bool
			virtual void assertExpression(storm::expressions::Expression &e) = 0;
			//! assert a set of expressions in the solver
			//! @param es the asserted expressions
			//! @see assert(storm::expressions::Expression &e)
			virtual void assertExpression(std::set<storm::expressions::Expression> &es) {
				for (storm::expressions::Expression e : es) {
					this->assertExpression(e);
				}
			}
			//! assert a set of expressions in the solver
			//! @param es the asserted expressions
			//! @see assert(storm::expressions::Expression &e)
			/* std::hash unavailable for expressions
			virtual void assertExpression(std::unordered_set<storm::expressions::Expression> &es) {
				for (storm::expressions::Expression e : es) {
					this->assertExpression(e);
				}
			}*/
			//! assert a set of expressions in the solver
			//! @param es the asserted expressions
			//! @see assert(storm::expressions::Expression &e)
			virtual void assertExpression(std::initializer_list<storm::expressions::Expression> &es) {
				for (storm::expressions::Expression e : es) {
					this->assertExpression(e);
				}
			}

			/*!
			* check satisfiability of the conjunction of the currently asserted expressions
			* 
			* @returns CheckResult::SAT if the conjunction of the asserted expressions is satisfiable,
			*          CheckResult::UNSAT if it is unsatisfiable and CheckResult::UNKNOWN if the solver
			*          could not determine satisfiability
			*/
			virtual CheckResult check() = 0;
			//! check satisfiability of the conjunction of the currently asserted expressions and the provided assumptions
			//! @param es the asserted expressions
			//! @throws IllegalArgumentTypeException if the return type of one of the expressions is not bool
			//! @see check()
			virtual CheckResult checkWithAssumptions(std::set<storm::expressions::Expression> &assumptions) = 0;
			//! check satisfiability of the conjunction of the currently asserted expressions and the provided assumptions
			//! @param es the asserted expressions
			//! @throws IllegalArgumentTypeException if the return type of one of the expressions is not bool
			//! @see check()
			/* std::hash unavailable for expressions
			virtual CheckResult checkWithAssumptions(std::unordered_set<storm::expressions::Expression> &assumptions) = 0;
			*/
			//! check satisfiability of the conjunction of the currently asserted expressions and the provided assumptions
			//! @param es the asserted expressions
			//! @throws IllegalArgumentTypeException if the return type of one of the expressions is not bool
			//! @see check()
			virtual CheckResult checkWithAssumptions(std::initializer_list<storm::expressions::Expression> assumptions) = 0;

			/*!
			* Gets a model for the assertion stack (and possibly assumtions) for the last call to ::check or ::checkWithAssumptions if that call
			* returned CheckResult::SAT. Otherwise an exception is thrown. 
			* @remark Note that this function may throw an exception if it is not called immediately after a call to::check or ::checkWithAssumptions 
			*         that returned CheckResult::SAT depending on the implementation.
			* @throws InvalidStateException if no model is available
			* @throws IllegalFunctionCallException if model generation is not configured for this solver
			* @throws NotImplementedException if model generation is not implemented with this solver class
			*/
			virtual storm::expressions::SimpleValuation getModel() {
				throw storm::exceptions::NotImplementedException("This subclass of SmtSolver does not support model generation.");
			}

			/*!
			* Performs all AllSat over the important atoms. All valuations of the important atoms such that the currently asserted formulas are satisfiable
			* are returned from the function.
			*
			* @warning If infinitely many valuations exist, such that the currently asserted formulas are satisfiable, this function will never return!
			*
			* @param important A set of expressions over which to perform all sat.
			*
			* @returns the set of all valuations of the important atoms, such that the currently asserted formulas are satisfiable
			*
			* @throws IllegalFunctionCallException if model generation is not configured for this solver
			* @throws NotImplementedException if model generation is not implemented with this solver class
			*/
			virtual std::vector<storm::expressions::SimpleValuation> allSat(std::vector<storm::expressions::Expression> important) {
				throw storm::exceptions::NotImplementedException("This subclass of SmtSolver does not support model generation.");
			}

			/*!
			* Performs all AllSat over the important atoms. Once a valuation of the important atoms such that the currently asserted formulas are satisfiable
			* is found the callback is called with that valuation.
			*
			* @param important A set of expressions over which to perform all sat.
			* @param callback A function to call for each found valuation.
			*
			* @returns the number of valuations of the important atoms, such that the currently asserted formulas are satisfiable that where found
			*
			* @throws IllegalFunctionCallException if model generation is not configured for this solver
			* @throws NotImplementedException if model generation is not implemented with this solver class
			*/
			virtual uint_fast64_t allSat(std::vector<storm::expressions::Expression> important, std::function<bool(storm::expressions::SimpleValuation&)> callback) {
				throw storm::exceptions::NotImplementedException("This subclass of SmtSolver does not support model generation.");
			}

			/*!
			* Retrieves the unsat core of the last call to check()
			*
			* @returns a subset of the asserted formulas s.t. this subset is unsat
			*
			* @throws InvalidStateException if no unsat core is available, i.e. the asserted formulas are consistent
			* @throws IllegalFunctionCallException if unsat core generation is not configured for this solver
			* @throws NotImplementedException if unsat core generation is not implemented with this solver class
			*/
			virtual std::vector<storm::expressions::Expression> getUnsatCore() {
				throw storm::exceptions::NotImplementedException("This subclass of SmtSolver does not support unsat core generation.");
			}

			/*!
			* Retrieves a subset of the assumptions from the last call to checkWithAssumptions(), s.t. the result is still unsatisfiable
			*
			* @returns a subset of the assumptions s.t. this subset of the assumptions results in unsat
			*
			* @throws InvalidStateException if no unsat assumptions is available, i.e. the asserted formulas are consistent
			* @throws IllegalFunctionCallException if unsat assumptions generation is not configured for this solver
			* @throws NotImplementedException if unsat assumptions generation is not implemented with this solver class
			*/
			virtual std::vector<storm::expressions::Expression> getUnsatAssumptions() {
				throw storm::exceptions::NotImplementedException("This subclass of SmtSolver does not support unsat core generation.");
			}
		};
	}
}

#endif // STORM_SOLVER_SMTSOLVER