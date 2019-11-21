#include <QmlUI/initComponentData_POC.h>
#include <sofa/core/ObjectFactory.h>

#include <sofa/helper/system/PluginManager.h>
using sofa::helper::system::PluginManager;

#include <fstream>

namespace sofa
{
namespace component
{

extern "C" {
    SOFA_QMLUI_API void initExternalModule();
    SOFA_QMLUI_API const char* getModuleName();
    SOFA_QMLUI_API const char* getModuleVersion();
    SOFA_QMLUI_API const char* getModuleLicense();
    SOFA_QMLUI_API const char* getModuleDescription();
    SOFA_QMLUI_API const char* getModuleComponentList();
}

void initExternalModule()
{    
    static bool first = true;
    if (!first)
    {
        return;
    }
    first = false;
}

const char* getModuleName()
{
    return "ComponentData_POC";
}

const char* getModuleVersion()
{
    return "1.0";
}

const char* getModuleLicense()
{
    return "MIT";
}

const char* getModuleDescription()
{
    return "A POC to get rid of Links between components by replacing them by datafields templated over Component::SPtrs";
}

const char* getModuleComponentList()
{
    /// string containing the names of the classes provided by the plugin
    static std::string classes = sofa::core::ObjectFactory::getInstance()->listClassesFromTarget(sofa_tostring(SOFA_TARGET));
    return classes.c_str();
}

} // namespace component
} // namespace sofa
