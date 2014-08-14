/*
 * TimeBoundedUntil.h
 *
 *  Created on: 10.04.2013
 *      Author: thomas
 */

#ifndef STORM_FORMULA_CSL_TIMEBOUNDEDUNTIL_H_
#define STORM_FORMULA_CSL_TIMEBOUNDEDUNTIL_H_

#include "src/formula/csl/AbstractPathFormula.h"
#include "src/formula/csl/AbstractStateFormula.h"

namespace storm {
namespace property {
namespace csl {

template <class T> class TimeBoundedUntil;

/*!
 *  @brief Interface class for model checkers that support TimeBoundedUntil.
 *
 *  All model checkers that support the formula class BoundedEventually must inherit
 *  this pure virtual class.
 */
template <class T>
class ITimeBoundedUntilModelChecker {
    public:
		/*!
         *  @brief Evaluates TimeBoundedUntil formula within a model checker.
         *
         *  @param obj Formula object with subformulas.
         *  @return Result of the formula for every node.
         */
        virtual std::vector<T> checkTimeBoundedUntil(const TimeBoundedUntil<T>& obj, bool qualitative) const = 0;
};

template <class T>
class TimeBoundedUntil: public AbstractPathFormula<T> {
public:

	/**
	 * Constructor providing bounds only;
	 * Sub formulas are set to null.
	 *
	 * @param lowerBound
	 * @param upperBound
	 */
	TimeBoundedUntil(T lowerBound, T upperBound) : left(nullptr), right(nullptr) {
		setInterval(lowerBound, upperBound);
	}


	/**
	 * Full constructor
	 * @param lowerBound
	 * @param upperBound
	 * @param left
	 * @param right
	 */
	TimeBoundedUntil(T lowerBound, T upperBound, std::shared_ptr<AbstractStateFormula<T>> const & left, std::shared_ptr<AbstractStateFormula<T>> const & right) : left(left), right(right) {
		setInterval(lowerBound, upperBound);
	}

	/*!
	 * Destructor
	 */
	virtual ~TimeBoundedUntil() {
		// Intentionally left empty.
	}

	/*!
	 * Clones the called object.
	 *
	 * Performs a "deep copy", i.e. the subtrees of the new object are clones of the original ones
	 *
	 * @returns a new BoundedUntil-object that is identical the called object.
	 */
	virtual std::shared_ptr<AbstractPathFormula<T>> clone() const override {
		std::shared_ptr<TimeBoundedUntil<T>> result(new TimeBoundedUntil<T>(lowerBound, upperBound));
		if (this->leftIsSet()) {
			result->setLeft(left->clone());
		}
		if (this->rightIsSet()) {
			result->setRight(right->clone());
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
		return modelChecker.template as<ITimeBoundedUntilModelChecker>()->checkTimeBoundedUntil(*this, qualitative);
	}

	/*!
	 * @returns a string representation of the formula
	 */
	virtual std::string toString() const override {
		std::string result = left->toString();
		result += " U";
		if (upperBound == std::numeric_limits<double>::infinity()) {
			result += ">=" + std::to_string(lowerBound);
		} else {
			result += "[";
			result += std::to_string(lowerBound);
			result += ",";
			result += std::to_string(upperBound);
			result += "]";
		}
		result += " ";
		result += right->toString();
		return result;
	}

	/*!
	 * Sets the left child node.
	 *
	 * @param newLeft the new left child.
	 */
	void setLeft(std::shared_ptr<AbstractStateFormula<T>> const & newLeft) {
		left = newLeft;
	}

	/*!
	 * Sets the right child node.
	 *
	 * @param newRight the new right child.
	 */
	void setRight(std::shared_ptr<AbstractStateFormula<T>> const & newRight) {
		right = newRight;
	}

	/*!
	 * @returns a pointer to the left child node
	 */
	std::shared_ptr<AbstractStateFormula<T>> const & getLeft() const {
		return left;
	}

	/*!
	 * @returns a pointer to the right child node
	 */
	std::shared_ptr<AbstractStateFormula<T>> const & getRight() const {
		return right;
	}

	/*!
	 *
	 * @return True if the left child is set, i.e. it does not point to nullptr; false otherwise
	 */
	bool leftIsSet() const {
		return left.get() != nullptr;
	}

	/*!
	 *
	 * @return True if the right child is set, i.e. it does not point to nullptr; false otherwise
	 */
	bool rightIsSet() const {
		return right.get() != nullptr;
	}

	/**
	 * Getter for lowerBound attribute
	 *
	 * @return lower bound of the operator.
	 */
	T const & getLowerBound() const {
		return lowerBound;
	}

	/**
	 * Getter for upperBound attribute
	 * @return upper bound of the operator.
	 */
	T const & getUpperBound() const {
		return upperBound;
	}

	/**
	 * Set the time interval for the time bounded operator
	 *
	 * @param lowerBound
	 * @param upperBound
	 * @throw InvalidArgumentException if the lower bound is larger than the upper bound.
	 */
	void setInterval(T lowerBound, T upperBound) {
		if (lowerBound > upperBound) {
			throw new storm::exceptions::InvalidArgumentException("Lower bound is larger than upper bound");
		}
		this->lowerBound = lowerBound;
		this->upperBound = upperBound;
	}

private:
	std::shared_ptr<AbstractStateFormula<T>> left;
	std::shared_ptr<AbstractStateFormula<T>> right;
	T lowerBound, upperBound;
};

} /* namespace csl */
} /* namespace property */
} /* namespace storm */

#endif /* STORM_FORMULA_CSL_TIMEBOUNDEDUNTIL_H_ */
