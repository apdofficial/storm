#ifndef STORM_SETTINGS_ARGUMENT_H_
#define STORM_SETTINGS_ARGUMENT_H_

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <utility>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "src/settings/ArgumentBase.h"
#include "src/settings/ArgumentType.h"
#include "src/settings/ArgumentTypeInferationHelper.h"
#include "src/utility/StringHelper.h"
#include "src/exceptions/ArgumentUnificationException.h"
#include "src/exceptions/IllegalArgumentException.h"
#include "src/exceptions/IllegalArgumentValueException.h"
#include "src/exceptions/IllegalFunctionCallException.h"

#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
extern log4cplus::Logger logger;

namespace storm {
	namespace settings {

		template<typename T>
		class Argument : public ArgumentBase {
		public:
			typedef std::function<bool (const T, std::string&)> userValidationFunction_t;

			typedef T argumentType_t;

			/*
			T argumentValue;
			ArgumentType argumentType;

			std::vector<userValidationFunction_t> userValidationFunction;

			T defaultValue;
			bool hasDefaultValue;
			*/

			Argument(std::string const& argumentName, std::string const& argumentDescription, std::vector<userValidationFunction_t> const& validationFunctions, bool isOptional): ArgumentBase(argumentName, argumentDescription, isOptional), argumentType(ArgumentTypeInferation::inferToEnumType<T>()), userValidationFunction(validationFunctions), hasDefaultValue(false) {
				if (isOptional) {
					LOG4CPLUS_ERROR(logger, "Argument::Argument: The Argument \"" << argumentName << "\" is flaged as optional but no default value was given!");
					throw storm::exceptions::IllegalArgumentException() << "The Argument \"" << argumentName << "\" is flaged as optional but no default value was given!";
				}
			}

			Argument(std::string const& argumentName, std::string const& argumentDescription, std::vector<userValidationFunction_t> const& validationFunctions, bool isOptional, T defaultValue): ArgumentBase(argumentName, argumentDescription, isOptional), argumentType(ArgumentTypeInferation::inferToEnumType<T>()), userValidationFunction(validationFunctions), defaultValue(defaultValue), hasDefaultValue(true) {
			
			}

			Argument(Argument const& other) : ArgumentBase(other.argumentName, other.argumentDescription, other.isOptional), argumentType(other.argumentType), defaultValue(other.defaultValue), hasDefaultValue(other.hasDefaultValue) {
				this->userValidationFunction.reserve(other.userValidationFunction.size());
				for (size_t i = 0; i < other.userValidationFunction.size(); ++i) {
					this->userValidationFunction.push_back(userValidationFunction_t(other.userValidationFunction.at(i)));
				}
			}

			virtual ~Argument() {
				//LOG4CPLUS_DEBUG(logger, "Argument::~Argument: Destructing Argument \"" << this->getArgumentName() << "\" of Type " << ArgumentTypeHelper::toString(this->getArgumentType()));

				this->userValidationFunction.clear();
				this->argumentType = ArgumentType::Invalid;
			}

			virtual ArgumentBase* clone() const override {
				return new Argument(*this);
			}

			assignmentResult_t fromStringValue(std::string const& fromStringValue) override {
				bool conversionOk = false;
				T newValue = this->convertFromString(fromStringValue, &conversionOk);
				if (!conversionOk) {
					std::string message("Could not convert the given String into ArgumentType Format (\"");
					message.append(ArgumentTypeHelper::toString(this->argumentType));
					message.append("\")!");
					return std::make_pair(false, message);
				}
				return this->fromTypeValue(newValue);
			}

			assignmentResult_t fromTypeValue(T const& newValue) {
				std::string errorText = "";
				if (!this->validateForEach(newValue, errorText)) {
					// The Conversion failed or a user defined Validation Function was given and it rejected the Input.
					return std::make_pair(false, errorText);
				}
				this->argumentValue = newValue;
				this->hasBeenSet = true;
				return std::make_pair(true, "Success");
			}

			virtual ArgumentType getArgumentType() const override {
				return this->argumentType;
			}

			template <typename S>
			void unify(Argument<S> &rhs) {
				if (this->getArgumentType() != rhs.getArgumentType()) {
					LOG4CPLUS_ERROR(logger, "Argument::unify: While unifying argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Types do not match (\"" << ArgumentTypeHelper::toString(this->getArgumentType()) << "\" and \"" << ArgumentTypeHelper::toString(rhs.getArgumentType()) << "\").");
					throw storm::exceptions::ArgumentUnificationException() << "While unifying Argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Types do not match (\"" << ArgumentTypeHelper::toString(this->getArgumentType()) << "\" and \"" << ArgumentTypeHelper::toString(rhs.getArgumentType()) << "\").";
				}

				if (this->getIsOptional() != rhs.getIsOptional()) {
					LOG4CPLUS_ERROR(logger, "Argument::unify: While unifying argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Both must either be optional or non-optional.");
					throw storm::exceptions::ArgumentUnificationException() << "While unifying argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Both must either be optional or non-optional.";
				}

				if (this->getHasDefaultValue() != rhs.getHasDefaultValue()) {
					LOG4CPLUS_ERROR(logger, "Argument::unify: While unifying argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Mismatching default values.");
					throw storm::exceptions::ArgumentUnificationException() << "While unifying argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Mismatching default values.";
				}

				if (this->getArgumentDescription().compare(rhs.getArgumentDescription()) != 0) {
					LOG4CPLUS_WARN(logger, "Argument::unify: While unifying argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Mismatching descriptions.");
				}

				if (this->getArgumentName().compare(rhs.getArgumentName()) != 0) {
					LOG4CPLUS_WARN(logger, "Argument::unify: While unifying argument \"" << this->getArgumentName() << "\" and argument \"" << rhs.getArgumentName() << "\": Mismatching descriptions.");
				}

				// Add Validation functions
				userValidationFunction.insert(userValidationFunction.end(), rhs.userValidationFunction.begin(), rhs.userValidationFunction.end());

				// Re-Add the Default Value to check for errors
				this->setDefaultValue(this->defaultValue);
			}

			T getArgumentValue() const {
				if (!this->getHasBeenSet()) {
					LOG4CPLUS_ERROR(logger, "Argument::getArgumentValue: Unable to retrieve argument of option \"" << this->getArgumentName() << "\", because it was never set and does not specify a default value.");
					throw storm::exceptions::IllegalFunctionCallException() << "Unable to retrieve argument of option \"" << this->getArgumentName() << "\", because it was never set and does not specify a default value.";
				}
				return this->argumentValue;
			}

			virtual bool getHasDefaultValue() const override {
				return this->hasDefaultValue;
			}

			std::string const& getDefaultValue() {
				return this->defaultValue;
			}

			void setFromDefaultValue() override {
				if (!this->hasDefaultValue) {
					LOG4CPLUS_ERROR(logger, "Argument::setFromDefaultValue: Unable to retrieve default value for argument \"" << this->getArgumentName() << "\" (" << this->getArgumentDescription() << ").");
					throw storm::exceptions::IllegalFunctionCallException() << "Unable to retrieve default value for argument \"" << this->getArgumentName() << "\" (" << this->getArgumentDescription() << ").";
				}
				// this call also sets the hasBeenSet flag
				assignmentResult_t result = this->fromTypeValue(this->defaultValue);
				if (!result.first) {
					LOG4CPLUS_ERROR(logger, "Argument::setFromDefaultValue: Unable to assign default value to argument \"" << this->getArgumentName() << "\" (" << this->getArgumentDescription() << "), because default value was rejected (" << result.second << ").");
					throw storm::exceptions::IllegalArgumentValueException() << "Unable to assign default value to argument \"" << this->getArgumentName() << "\" (" << this->getArgumentDescription() << "), because default value was rejected (" << result.second << ").";
				}
			}

			virtual std::string getValueAsString() const override {
				switch (this->argumentType) {
					case ArgumentType::String:
						return ArgumentTypeInferation::inferToString(ArgumentType::String, this->getArgumentValue());
					case ArgumentType::Boolean: {
						bool iValue = ArgumentTypeInferation::inferToBoolean(ArgumentType::Boolean, this->getArgumentValue());
						if (iValue) {
							return "true";
						} else {
							return "false";
						}

					}
					default:
						return this->convertToString(this->argumentValue);

				}
			}
			
			virtual int_fast64_t getValueAsInteger() const override {
				switch (this->argumentType) {
					case ArgumentType::Integer:
						return ArgumentTypeInferation::inferToInteger(ArgumentType::Integer, this->getArgumentValue());
					default: {
						LOG4CPLUS_ERROR(logger, "Argument::getValueAsInteger(): Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as integer.");
						throw storm::exceptions::IllegalFunctionCallException() << "Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as integer.";
					}
				}
			}
			
			virtual uint_fast64_t getValueAsUnsignedInteger() const override {
				switch (this->argumentType) {
					case ArgumentType::UnsignedInteger:
						return ArgumentTypeInferation::inferToUnsignedInteger(ArgumentType::UnsignedInteger, this->getArgumentValue());
					default: {
						LOG4CPLUS_ERROR(logger, "Argument::getValueAsUnsignedInteger(): Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as unsigned integer.");
						throw storm::exceptions::IllegalFunctionCallException() << "Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as unsigned integer.";
					}
				}
			}
			
			virtual double getValueAsDouble() const override {
				switch (this->argumentType) {
					case ArgumentType::Double:
						return ArgumentTypeInferation::inferToDouble(ArgumentType::Double, this->getArgumentValue());
					default: {
						LOG4CPLUS_ERROR(logger, "Argument::getValueAsDouble(): Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as double.");
						throw storm::exceptions::IllegalFunctionCallException() << "Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as double.";
					}
				}
			}
			
			virtual bool getValueAsBoolean() const override {
				switch (this->argumentType) {
					case ArgumentType::Boolean:
						return ArgumentTypeInferation::inferToBoolean(ArgumentType::Boolean, this->getArgumentValue());
					default: {
						LOG4CPLUS_ERROR(logger, "Argument::getValueAsBoolean(): Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as boolean.");
						throw storm::exceptions::IllegalFunctionCallException() << "Unable to retrieve value of argument \"" << getArgumentName() << "\" of type \"" << ArgumentTypeHelper::toString(getArgumentType()) << "\" as boolean.";
					}
				}
			}
		private:
			T argumentValue;
			ArgumentType argumentType;

			std::vector<userValidationFunction_t> userValidationFunction;

			T defaultValue;
			bool hasDefaultValue;

			void setDefaultValue(T const& newDefault) {
				std::string errorText = "";
				if (!this->validateForEach(newDefault, errorText)) {
					// A user defined Validation Function was given and it rejected the Input.
					LOG4CPLUS_ERROR(logger, "Argument::setDefaultValue: Illegal default value for argument \"" << this->getArgumentName() << "\"." << std::endl << "The validation function rejected the value (" << errorText << ").");
					throw storm::exceptions::IllegalArgumentValueException() << "Illegal default value for argument \"" << this->getArgumentName() << "\". The validation function rejected the value (" << errorText << ").";
				} 
				this->defaultValue = newDefault;
				this->hasDefaultValue = true;
			}

			void unsetDefaultValue() {
				this->hasDefaultValue = false;
			}

			std::string convertToString(T const& t) const {
					std::ostringstream stream;
					stream << t;
					return stream.str();
			}
			
			T convertFromString(std::string const& s, bool* ok = nullptr) const {
				return storm::settings::ArgumentBase::ArgumentHelper::convertFromString<T>(s, ok);
			}

			bool validateForEach(T const& value, std::string& errorMessageTarget) const {
				bool result = true;

				for (auto lambda: this->userValidationFunction) {
					result = result && lambda(value, errorMessageTarget);
				}
				return result;
			}

			
		};
	}
}

#endif // STORM_SETTINGS_ARGUMENT_H_