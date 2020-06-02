#include "Link.inl"
#include <sofa/core/objectmodel/Base.h>
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/core/objectmodel/BaseNode.h>
#include <sofa/helper/logging/Message.h>


namespace nodephysics
{

using namespace sofa::core::objectmodel;

/****************************************************************************
 *********************************BASELINK***********************************
 ****************************************************************************/

BaseLink::BaseLink(const BaseLink::InitLink &init)
    : m_name(init.name),
      m_help(init.help),
      m_group(init.group),
      m_linkedDest(init.linkedDest),
      m_owner(init.owner),
      m_counter(0)
{
    if (m_owner)
    {
        m_owner->addLink(this, m_name);
    }
    setDirtyValue();
}

BaseLink::~BaseLink()
{

}

void BaseLink::setOwner(LinkHandler* owner)
{
    if (m_linkedDest)
    {
        m_linkedDest->removeLinkHandler(m_owner);
        m_linkedDest->addLinkHandler(owner);
    }
    m_owner = owner;
}

void BaseLink::setLinkedDest(LinkHandler* linkedDest)
{
    if (m_linkedDest)
        m_linkedDest->removeLinkHandler(m_owner);
    m_linkedDest = linkedDest;
    if (m_linkedDest) {
        m_linkedDest->addLinkHandler(m_owner);
        addInput(&m_linkedDest->d_componentstate);
    }
    ++m_counter;
    setDirtyOutputs();
}

LinkHandler* BaseLink::getLinkedDest()
{
    update();
    return m_linkedDest;
}


void BaseLink::update()
{
    for(auto it : inputs)
    {
        if (it->isDirty())
        {
            it->update();
        }
    }
    ++m_counter;
    cleanDirty();
}

const std::string& BaseLink::getName() const
{
    return m_name;
}

LinkHandler* BaseLink::getOwner()
{
    return m_owner;
}

std::string BaseLink::getPathName() const
{
    if (!m_owner)
        return getName();

    auto bn = m_owner->toBaseNode();
    auto bo = m_owner->toBaseObject();
    std::string pathname = bn ? bn->getPathName() : bo->getPathName();
    return pathname + "." + getName();
}


/****************************************************************************
 *******************************LINKHANDLER**********************************
 ****************************************************************************/

LinkHandler::LinkHandler()
{

}

bool LinkHandler::hasField(const std::string &attribute) const
{
    return m_aliasData.find(attribute) != m_aliasData.end()
            || m_aliasLink.find(attribute) != m_aliasLink.end()
            || m_linkAliases.find(attribute) != m_linkAliases.end();
}

/// Assign one field value (Data or Link)
bool LinkHandler::parseField(const std::string &attribute, const std::string &value)
{
    BaseLink* link = findLink(attribute);
    std::vector< BaseData* > dataVec = findGlobalField(attribute);
    std::vector< sofa::core::objectmodel::BaseLink* > linkVec = findLinks(attribute);
    if (dataVec.empty() && linkVec.empty() && !link)
    {
        msg_warning() << "Unknown Data field or Link: " << attribute ;
        return false; // no field found
    }

    bool ok = true;
    // Try to find "nodephysics" link
    if (value.length() > 0 && value[0] == '@')
    {
        BaseData* data = nullptr;
        sofa::core::objectmodel::BaseLink* bl = nullptr;
        findDataLinkDest(data, value + ".name", bl);
        if (data != nullptr || data->getOwner() == nullptr)
        {
            link->setLinkedDest(dynamic_cast<LinkHandler*>(data->getOwner()));
            ok = true;
        }
        else {
            msg_error(this) << "Could not find object with path " << value << " from " << this->getName();
            ok = false;
        }
    }

    for (unsigned int d=0; d<dataVec.size(); ++d)
    {
        // test if data is a link and can be linked
        if (value.length() > 0 && value[0] == '@' && dataVec[d]->canBeLinked())
        {
            if (!dataVec[d]->setParent(value))
            {
                BaseData* data = nullptr;
                sofa::core::objectmodel::BaseLink* bl = nullptr;
                dataVec[d]->findDataLinkDest(data, value, bl);
                if (data != nullptr && dynamic_cast<EmptyData*>(data) != nullptr)
                {
                    Base* owner = data->getOwner();
                    DDGNode* o = dynamic_cast<DDGNode*>(owner);
                    o->delOutput(data);
                    owner->removeData(data);
                    BaseData* newBD = dataVec[d]->getNewInstance();
                    newBD->setName(data->getName());
                    owner->addData(newBD);
                    newBD->setGroup("Outputs");
                    o->addOutput(newBD);
                    dataVec[d]->setParent(newBD);
                    ok = true;
                    continue;
                }
                msg_warning()<<"Could not setup Data link between "<< value << " and " << attribute << "." ;
                ok = false;
                continue;
            }
            else
            {
                BaseData* parentData = dataVec[d]->getParent();
                msg_info() << "Link from parent Data " << value << " (" << parentData->getValueTypeInfo()->name() << ") to Data " << attribute << "(" << dataVec[d]->getValueTypeInfo()->name() << ") OK";
            }
            /* children Data cannot be modified changing the parent Data value */
            dataVec[d]->setReadOnly(true);
            continue;
        }
        if( !(dataVec[d]->read( value )) && !value.empty())
        {
            msg_warning()<<"Could not read value for data field "<< attribute <<": " << value ;
            ok = false;
        }
    }
    for (unsigned int l=0; l<linkVec.size(); ++l)
    {
        if( !(linkVec[l]->read( value )) && !value.empty())
        {
            msg_warning()<<"Could not read value for link "<< attribute <<": " << value;
            ok = false;
        }
        msg_info() << "Link " << linkVec[l]->getName() << " = " << linkVec[l]->getValueString();
        unsigned int s = unsigned(linkVec[l]->getSize());
        for (unsigned int i=0; i<s; ++i)
        {
            std::stringstream tmp;
            tmp << "  " << linkVec[l]->getLinkedPath(i) << " = ";
            Base* b = linkVec[l]->getLinkedBase(i);
            BaseData* d = linkVec[l]->getLinkedData(i);
            if (b) tmp << b->getTypeName() << " " << b->getName();
            if (d) tmp << " . " << d->getValueTypeString() << " " << d->getName();
            msg_info() << tmp.str();
        }
    }
    return ok;
}



/// Find a link given its name. Return nullptr if not found.
BaseLink* LinkHandler::findLink( const std::string &name ) const
{
    auto it = m_linkAliases.find(name);
    if (it != m_linkAliases.end())
        return it->second;
    else
        return nullptr;
}

/// Registers a DDGNode Link.
void LinkHandler::addLink(BaseLink* l, const std::string& name)
{
    if (name.size() > 0 && (findData(name) || findLink(name)))
    {
        msg_warning(this) << "Data field name " << name
                << " already used in this class or in a parent class !";
    }
    m_linkList.push_back(l);
    m_linkAliases.insert(std::make_pair(name, l));
    l->setOwner(this);
}
/// Remove a DDGNode Link.
void LinkHandler::removeLink(BaseLink* l)
{
    m_linkList.erase(std::find(m_linkList.begin(), m_linkList.end(), l));
    m_linkAliases.erase(m_linkAliases.find(l->getName()));
}


/// Registers a link owner. should only be called from DDGLinks
void LinkHandler::addLinkHandler(LinkHandler* h)
{
    if (std::find(m_linkHandlers.begin(), m_linkHandlers.end(), h) == m_linkHandlers.end())
            m_linkHandlers.push_back(h);
}

/// Removes a link owner. should only be called from DDGLinks
void LinkHandler::removeLinkHandler(LinkHandler* h)
{
    if (!m_linkHandlers.size())
        return;
    m_linkHandlers.erase(std::find(m_linkHandlers.begin(), m_linkHandlers.end(), h));
}

void LinkHandler::addUpdateCallback(const std::string &name, std::initializer_list<DDGNode *> inputs, std::function<ComponentState ()> function, std::initializer_list<DDGNode *> outputs)
{
    m_internalEngine[name].setName(name);
    m_internalEngine[name].setOwner(this);
    m_internalEngine[name].addInputs(inputs);
    m_internalEngine[name].addCallback(function);
    m_internalEngine[name].addOutputs(outputs);
    m_internalEngine[name].addOutput(&d_componentstate);
}

} // namespace nodephysics
