#pragma once

#include <sofa/core/objectmodel/Data.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/simulation/Node.h>
#include <sofa/core/objectmodel/ClassInfo.h>

namespace nodephysics
{
using namespace sofa::core::objectmodel;


template <class T>
class ObjectLink : public Data<T*>
{
public:

    /** \copydoc BaseData(const BaseData::BaseInitData& init) */
    explicit ObjectLink(const BaseData::BaseInitData& init)
        : Data<T*>(init)
    {
    }

    /** \copydoc Data(const BaseData::BaseInitData&) */
    explicit ObjectLink(const typename Data<T*>::InitData& init)
        : Data<T*>(init)
    {
    }

    /** \copydoc BaseData(const char*, bool, bool) */
    //TODO(dmarchal:08/10/2019)Uncomment the deprecated when VS2015 support will be dropped.
    //[[deprecated("Replaced with one with std::string instead of char* version")]]
    ObjectLink( const char* helpMsg=nullptr, bool isDisplayed=true, bool isReadOnly=false)
        : Data<T*>(helpMsg, isDisplayed, isReadOnly) {}

    /** \copydoc BaseData(const char*, bool, bool) */
    ObjectLink( const std::string& helpMsg, bool isDisplayed=true, bool isReadOnly=false)
        : Data<T*>(helpMsg, isDisplayed, isReadOnly)
    {
    }


    /** \copydoc BaseData(const char*, bool, bool)
     *  \param value The default value.
     */
    ObjectLink( const T& value, const char* helpMsg=nullptr, bool isDisplayed=true, bool isReadOnly=false) :
        Data<T*>(&value, helpMsg, isDisplayed, isReadOnly)
    {
        value->d_componentstate.addOutput(this);
        Data<T*>::setValue(value);
    }

    /** \copydoc BaseData(const char*, bool, bool)
     *  \param value The default value.
     */
    ObjectLink( const T& value, const std::string& helpMsg, bool isDisplayed=true, bool isReadOnly=false)
        : Data<T*>(&value, helpMsg, isDisplayed, isReadOnly)
    {
        value->d_componentstate.addOutput(this);
        Data<T*>::setValue(value);
    }

    /// Destructor.
    virtual ~ObjectLink()
    {}


    T* resolvePath(const std::string& path)
    {
        if (this->getOwner() == nullptr)
        {
            msg_error("ObjectLink") << "Cannot resolve path, as " << this->getName() << " has no owner";
            return nullptr;
        }
        BaseNode* ctx = this->getOwner()->toBaseNode();
        if (!ctx)
            ctx = this->getOwner()->toBaseObject()->getContext()->toBaseNode();
        if (!ctx)
        {
            msg_error("ObjectLink") << "Cannot resolve path, as " << this->getOwner()->getName() << " has no context";
            return nullptr;
        }
        BaseData* data;
        ctx->findDataLinkDest(data, path + ".name", nullptr);
        return dynamic_cast<T*>(data->getOwner());
    }

    virtual bool validParent(BaseData* /*parent*/)
    {
        return false;
    }

    BaseData* getParent() const { return nullptr; }

    bool setParent(const std::string& path)
    {
        T* parent = this->resolvePath(path);
        if (parent)
        {
            setValue(parent);
            return true;
        }
        return false;
    }

    void setValue(T* value)
    {
        value->d_componentstate.addOutput(this);
        Data<T*>::setValue(value);
    }


};

}  // namespace sofa::core::objectmodel

