#ifndef STORM_LOGIC_CLONEVISITOR_H_
#define STORM_LOGIC_CLONEVISITOR_H_

#include <memory>

#include "src/logic/FormulaVisitor.h"

namespace storm {
    namespace logic {
        
        class CloneVisitor : public FormulaVisitor {
        public:
            std::shared_ptr<Formula> clone(Formula const& f) const;
            
            virtual boost::any visit(AtomicExpressionFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(AtomicLabelFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(BinaryBooleanStateFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(BooleanLiteralFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(BoundedUntilFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(ConditionalFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(CumulativeRewardFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(EventuallyFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(TimeOperatorFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(GloballyFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(InstantaneousRewardFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(LongRunAverageOperatorFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(LongRunAverageRewardFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(NextFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(ProbabilityOperatorFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(RewardOperatorFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(UnaryBooleanStateFormula const& f, boost::any const& data) const override;
            virtual boost::any visit(UntilFormula const& f, boost::any const& data) const override;
        };
        
    }
}


#endif /* STORM_LOGIC_CLONEVISITOR_H_ */