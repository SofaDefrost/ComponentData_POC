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
      m_handler(init.handler),
      m_counter(0)
{
    if (m_handler)
    {
        m_handler->addLink(this, m_name);
    }
}

BaseLink::~BaseLink()
{

}

void BaseLink::setHandler(LinkHandler* handler)
{
    if (m_linkedDest)
    {
        m_linkedDest->removeLinkHandler(m_handler);
        m_linkedDest->addLinkHandler(handler);
    }
    m_handler = handler;
}

void BaseLink::setLinkedDest(LinkHandler* linkedDest)
{
    if (m_linkedDest)
        m_linkedDest->removeLinkHandler(m_handler);
    m_linkedDest = linkedDest;
    addInput(&(m_linkedDest->getBase())->d_componentstate);
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

LinkHandler* BaseLink::getHandler()
{
    return m_handler;
}

std::string BaseLink::getPathName() const
{
    if (!m_handler)
        return getName();

    auto bn = m_handler->getBase()->toBaseNode();
    auto bo = m_handler->getBase()->toBaseObject();
    std::string pathname = bn ? bn->getPathName() : bo->getPathName();
    return pathname + "." + getName();
}


/****************************************************************************
 *******************************LINKHANDLER**********************************
 ****************************************************************************/

LinkHandler::LinkHandler(sofa::core::objectmodel::Base* _this)
    : m_base(_this)
{

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
    if (name.size() > 0 && (m_base->findData(name) || m_base->findLink(name)))
    {
        msg_warning(m_base) << "Data field name " << name
                << " already used in this class or in a parent class !";
    }
    m_linkList.push_back(l);
    m_linkAliases.insert(std::make_pair(name, l));
    l->setHandler(this);
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
    m_linkHandlers.erase(std::find(m_linkHandlers.begin(), m_linkHandlers.end(), h));
}

void LinkHandler::addUpdateCallback(const std::string &name, std::initializer_list<DDGNode *> inputs, std::function<ComponentState ()> function, std::initializer_list<DDGNode *> outputs)
{
    m_internalEngine[name].setName(name);
    m_internalEngine[name].setOwner(this->getBase());
    m_internalEngine[name].addInputs(inputs);
    m_internalEngine[name].addCallback(function);
    m_internalEngine[name].addOutputs(outputs);
    m_internalEngine[name].addOutput(&getBase()->d_componentstate);
}

} // namespace nodephysics
