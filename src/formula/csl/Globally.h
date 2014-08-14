/*
 * Next.h
 *
 *  Created on: 26.12.2012
 *      Author: Christian Dehnert
 */

#ifndef STORM_FORMULA_CSL_GLOBALLY_H_
#define STORM_FORMULA_CSL_GLOBALLY_H_

#include "src/formula/csl/AbstractPathFormula.h"
#include "src/formula/csl/AbstractStateFormula.h"
#include "src/modelchecker/csl/ForwardDeclarations.h"

namespace storm {
namespace property {
namespace csl {

template <class T> class Globally;

/*!
 *  @brief Interface class for model checkers that support Globally.
 *   
 *  All model checkers that support the formula class Globally must inherit
 *  this pure virtual class.
 */
template <class T>
class IGloballyModelChecker {
    public:
		/*!
         *  @brief Evaluates Globally formula within a model checker.
         *
         *  @param obj Formula object with subformulas.
         *  @return Result of the formula for every node.
         */
        virtual std::vector<T> checkGlobally(const Globally<T>& obj, bool qualitative) const = 0;
};

/*!
 * @brief
 * Class for an abstract (path) formula tree with a Globally node as root.
 *
 * Has one Abstract state formula as sub formula/tree.
 *
 * @par Semantics
 * The formula holds iff globally \e child holds.
 *
 * The subtree is seen as part of the object and deleted with the object
 * (this behavior can be prevented by setting them to nullptr before deletion)
 *
 * @see AbstractPathFormula
 * @see AbstractCslFormula
 */
template <class T>
class Globally : public AbstractPathFormula<T> {

public:

	/*!
	 * Empty constructor
	 */
	Globally() : child(nullptr){
		// Intentionally left empty.
	}

	/*!
	 * Constructor
	 *
	 * @param child The child node
	 */
	Globally(std::shared_ptr<AbstractStateFormula<T>> const & child) : child(child){
		// Intentionally left empty.
	}

	/*!
	 * Constructor.
	 *
	 * Also deletes the subtree.
	 * (this behaviour can be prevented by setting the subtrees to nullptr before deletion)
	 */
	virtual ~Globally() {
		// Intentionally left empty.
	}

	/*!
	 * Clones the called object.
	 *
	 * Performs a "deep copy", i.e. the subtrees of the new object are clones of the original ones
	 *
	 * @returns a new Globally-object that is identical the called object.
	 */
	virtual std::shared_ptr<AbstractPathFormula<T>> clone() const override {
		std::shared_ptr<Globally<T>> result(new Globally<T>());
		if (this->childIsSet()) {
			result->setChild(child->clone());
		}
		return result;
	}

	/*!
	 * Calls the model checker to check this formula.
	 * Needed to infer the correct type of formula class.
	 *
	 * @note This function should only be called in a generic check function of a model checker class. For other uses,
	 *       the methods of the model checker should be used.
	 *
	 * @returns A vector indicating the probability that the formula holds for each state.
	 */
	virtual std::vector<T> check(storm::modelchecker::csl::AbstractModelChecker<T> const & modelChecker, bool qualitative) const override {
		return modelChecker.template as<IGloballyModelChecker>()->checkGlobally(*this, qualitative);
	}

	/*!
	 * @returns a string representation of the formula
	 */
	virtual std::string toString() const override {
		std::string result = "G ";
		result += child->toString();
		return result;
	}

	/*!
	 * @returns the child node
	 */
	AbstractStateFormula<T> const & getChild() const {
		return child;
	}

	/*!
	 * Sets the subtree
	 * @param child the new child node
	 */
	void setChild(std::shared_ptr<AbstractStateFormula<T>> const & child) {
		this->child = child;
	}

	/*!
	 *
	 * @return True if the child node is set, i.e. it does not point to nullptr; false otherwise
	 */
	bool childIsSet() const {
		return child.get() != nullptr;
	}

private:
	std::shared_ptr<AbstractStateFormula<T>> child;
};

} //namespace csl
} //namespace property
} //namespace storm

#endif /* STORM_FORMULA_CSL_GLOBALLY_H_ */
