#pragma once

#include <sofa/helper/system/config.h>

#ifdef SOFA_BUILD_COMPONENTDATA_POC
#define SOFA_TARGET ComponentData_POC
#define SOFA_COMPONENTDATA_POC_API SOFA_EXPORT_DYNAMIC_LIBRARY
#else
#define SOFA_COMPONENTDATA_POC_API SOFA_IMPORT_DYNAMIC_LIBRARY
#endif
