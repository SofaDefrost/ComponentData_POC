#pragma once

#include <sofa/core/objectmodel/Data.h>
#include <sofa/core/objectmodel/BaseObject.h>

namespace sofa::core::objectmodel
{

template <class T>
class Link : public Data<T>
{
    void setDirtyValue(const core::ExecParams* params)
    {
        // A Dirty component has dirty inputs & outputs
        for (auto data : this->getValue().getDataFields())
        {
            data->setDirtyValue();
        }
        Data<T>::setDirtyValue(params);
    }
};

}  // namespace sofa::core::objectmodel

