#pragma once

#include <sofa/core/objectmodel/Data.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/simulation/Node.h>
#include <sofa/core/objectmodel/ClassInfo.h>

namespace nodephysics
{

using sofa::core::objectmodel::Data;
using sofa::core::objectmodel::BaseData;

template <class T>
class ObjectLink : public sofa::core::objectmodel::Data<T*>
{
public:

    /** \copydoc BaseData(const BaseData::BaseInitData& init) */
    explicit ObjectLink(const BaseData::BaseInitData& init)
        : Data<T>(init)
    {
    }

    /** \copydoc Data(const BaseData::BaseInitData&) */
    explicit ObjectLink(const typename Data<T>::InitData& init)
        : Data<T>(init)
    {
    }

    /** \copydoc BaseData(const char*, bool, bool) */
    //TODO(dmarchal:08/10/2019)Uncomment the deprecated when VS2015 support will be dropped.
    //[[deprecated("Replaced with one with std::string instead of char* version")]]
    ObjectLink( const char* helpMsg=nullptr, bool isDisplayed=true, bool isReadOnly=false)
        : Data<T>(helpMsg, isDisplayed, isReadOnly) {}

    /** \copydoc BaseData(const char*, bool, bool) */
    ObjectLink( const std::string& helpMsg, bool isDisplayed=true, bool isReadOnly=false)
        : Data<T>(helpMsg, isDisplayed, isReadOnly)
    {
    }


    /** \copydoc BaseData(const char*, bool, bool)
     *  \param value The default value.
     */
    ObjectLink( const T& value, const char* helpMsg=nullptr, bool isDisplayed=true, bool isReadOnly=false) :
        Data<T>(value, helpMsg, isDisplayed, isReadOnly)
    {}

    /** \copydoc BaseData(const char*, bool, bool)
     *  \param value The default value.
     */
    ObjectLink( const T& value, const std::string& helpMsg, bool isDisplayed=true, bool isReadOnly=false)
        : Data<T>(value, helpMsg, isDisplayed, isReadOnly)
    {
    }

    /// Destructor.
    virtual ~ObjectLink()
    {}














    /// A Link cannot *actually* have a parent, because:
    /// a data's parent has to be:
    /// - A Data
    /// - of the same type
    ///
    /// Instead, setParent uses the path to retrieve the linked object, sets its component state as input to the data,
    /// and stores the parent's reference as this object's value
    bool setParent(const std::string& path)
    {
        sofa::simulation::Node* node = static_cast<sofa::simulation::Node*>(this->getOwner()->toBaseNode());

        T* parent = node->getObject(classid(T), path);
        this->addInput(parent->d_componentstate);
        this->setValue(parent);
    }

    BaseData* getParent() const { return nullptr; }

    bool setParent(BaseData* /*parent*/, const std::string& /*path*/ = std::string())
    {
        return false;
    }

    bool validParent(BaseData* parentComponentState) override
    {
        if (parentComponentState->getName() == "componentState" &&
                parentComponentState->getOwner() &&
                dynamic_cast<T*>(parentComponentState->getOwner()))
            return true;
        return false;
    }

    void doSetParent(BaseData* parentComponentState) override
    {
        if (validParent(parentComponentState))
        {
            T* parent = dynamic_cast<T*>(parentComponentState->getOwner());
            this->addInput(parentComponentState);
            if (parent)
                this->setValue(parent);
        }
    }
};

}  // namespace sofa::core::objectmodel

