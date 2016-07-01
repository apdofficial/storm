#include "src/logic/RewardMeasureType.h"

namespace storm {
    namespace logic {
        
        std::ostream& operator<<(std::ostream& out, RewardMeasureType const& type) {
            switch (type) {
                case RewardMeasureType::Expectation:
                    out << "exp";
                    break;
                case RewardMeasureType::Variance:
                    out << "var";
                    break;
            }
            return out;
        }
        
    }
}