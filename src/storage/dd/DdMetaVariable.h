#ifndef STORM_STORAGE_DD_DDMETAVARIBLE_H_
#define STORM_STORAGE_DD_DDMETAVARIBLE_H_

#include "src/storage/dd/DdType.h"

namespace storm {
    namespace dd {
        // Declare DdMetaVariable class so we can then specialize it for the different DD types.
        template<DdType Type> class DdMetaVariable;
    }
}

#endif /* STORM_STORAGE_DD_DDMETAVARIBLE_H_ */