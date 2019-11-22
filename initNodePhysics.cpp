/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2018 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <SofaImplicitField3/config.h>

#include<sofa/core/ObjectFactory.h>
using sofa::core::ObjectFactory;

#include <SofaImplicitField/components/geometry/ScalarField.h>
#include <SofaImplicitField/components/geometry/SphericalField.h>
#include <SofaImplicitField/components/geometry/DiscreteGridField.h>
#include <SofaImplicitField/components/visual/PointCloudImplicitFieldVisualization.h>

#include <sofa/helper/system/PluginManager.h>
using sofa::helper::system::PluginManager ;
#include <SofaPython3/PythonEnvironment.h>



namespace sofaimplicitfield3
{

extern "C" {
SOFA_SOFAIMPLICITFIELD3_API void initExternalModule();
SOFA_SOFAIMPLICITFIELD3_API const char* getModuleName();
SOFA_SOFAIMPLICITFIELD3_API const char* getModuleVersion();
SOFA_SOFAIMPLICITFIELD3_API const char* getModuleLicense();
SOFA_SOFAIMPLICITFIELD3_API const char* getModuleDescription();
SOFA_SOFAIMPLICITFIELD3_API const char* getModuleComponentList();
}

void initExternalModule()
{
    static bool first = true;
    if (first)
    {
        first = false;
    }

    PluginManager::getInstance().loadPlugin("SofaPython3") ;
    sofapython3::PythonEnvironment::runString("import Sofa");
}

const char* getModuleName()
{
    return "SofaImplicitField3";
}

const char* getModuleVersion()
{
    return "1.0";
}

const char* getModuleLicense()
{
    return "LGPL";
}


const char* getModuleDescription()
{
    return "Additional feature for modeling with implicit surfaces.";
}

const char* getModuleComponentList()
{
    /// string containing the names of the classes provided by the plugin
    static std::string classes = ObjectFactory::getInstance()->listClassesFromTarget(sofa_tostring(SOFA_TARGET));
    return classes.c_str();
}

} /// sofaimplicitefield3

