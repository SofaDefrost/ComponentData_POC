#pragma once

#include "DataTrackerEngine.h"

#include <sofa/core/objectmodel/DDGNode.h>
#include <sofa/core/objectmodel/ComponentState.h>
#include <sofa/core/objectmodel/BaseData.h>
#include <sofa/core/ExecParams.h>
#include <sofa/core/core.h>

#include <list>

namespace sofa::core::objectmodel { class Base; }

namespace nodephysics
{
class LinkHandler;


/**
 * @brief The BaseLink class
 *
 * BaseLink inherits DDGNode, thus is part of the data dependency graph, thus can have inputs and outputs.
 * When setting a link, the linked base's componentState data is added as an input to the BaseLink,
 * which creates the connection between the BaseLink and the DDG.
 * any data, engine, etc. can then be connected as output.
 */
class SOFA_CORE_API BaseLink : public sofa::core::objectmodel::DDGNode
{
public:
    /// This internal class is used by the initLink() methods to store initialization parameters of a Data
    class SOFA_CORE_API InitLink
    {
    public:
        InitLink()
            : name(""),
              help(""),
              group(""),
              linkedDest(nullptr),
              handler(nullptr)
        {}

        std::string name;
        std::string help;
        std::string group;
        LinkHandler* linkedDest;
        LinkHandler* handler;
    };

    explicit BaseLink(const InitLink& init);

    virtual ~BaseLink() override;

    void setHandler(LinkHandler* handler);
    LinkHandler* getHandler();

    void setLinkedDest(LinkHandler* linkedDest);
    LinkHandler* getLinkedDest();

    virtual void update() override;

    const std::string& getName() const;
    std::string getPathName() const;

protected:
    std::string m_name {""};
    std::string m_help {""};
    std::string m_group {""};

    LinkHandler* m_linkedDest {nullptr};
    LinkHandler* m_handler {nullptr};

private:
    /// Number of changes since creation
    int m_counter;
};


template <class T>
class Link : public BaseLink
{
  public:

    explicit Link(const BaseLink::InitLink& init);
    virtual ~Link();

    void setLinkedDest(T* linkedDest);
    T* getLinkedDest();

    T* operator->();
    T* operator*();

    void operator=(T* o);
    void operator=(typename T::SPtr o);
};


// This class should be inherited by Base, and handles a list of all links this object has registered,
// along with all link handlers pointing to this object.
class LinkHandler
{
public:
    LinkHandler(sofa::core::objectmodel::Base* _this);

    typedef std::list<BaseLink*> LinkList;
    typedef std::list<LinkHandler*> LinkHandlers;
    typedef std::map<std::string, BaseLink*> LinkMap;

    /// Find a link given its name. Return nullptr if not found.
    BaseLink* findLink( const std::string &name ) const;

    /// Registers a DDGNode Link.
    void addLink(BaseLink* l, const std::string& name);
    /// Remove a DDGNode Link.
    void removeLink(BaseLink* l);

    /// Registers a link owner. should only be called from DDGLinks
    void addLinkHandler(LinkHandler* h);
    /// Removes a link owner. should only be called from DDGLinks
    void removeLinkHandler(LinkHandler* h);

    /// Accessor to the vector containing all the links of this object
    const LinkList& getLinks() const { return m_linkList; }
    /// Accessor to the alias map containing all the links of this object
    const LinkMap& getLinkAliases() const { return m_linkAliases; }

    /// Accessor to the list of handlers holding a link to this object
    const LinkHandlers getLinkHandlers() const { return m_linkHandlers; }

    /// Returns the pointer to the Base class of this scene graph item
    sofa::core::objectmodel::Base* getBase() { return m_base; }
    const sofa::core::objectmodel::Base* getBase() const { return m_base; }


    // This callback mechanism is already part of Base in our SOFA branch for SofaQtQuick, used w/o any observed side-effects:
    void addUpdateCallback(const std::string& name,
                           std::initializer_list<sofa::core::objectmodel::DDGNode*> inputs,
                           std::function<sofa::core::objectmodel::ComponentState(void)> function,
                           std::initializer_list<sofa::core::objectmodel::DDGNode*> outputs);

private:
    LinkList m_linkList; // The list of all links handled by this object, pointing to other objects
    LinkMap m_linkAliases;
    LinkHandlers m_linkHandlers; // The list of all handlers holding a link to this object
    sofa::core::objectmodel::Base* m_base; // The pointer to the Base that uses this handler
    std::map<std::string, nodephysics::DataTrackerEngine> m_internalEngine;
};

BaseLink::InitLink initLink(LinkHandler* handler,
                               std::string name,
                               std::string help,
                               std::string group = "")
{
    BaseLink::InitLink init;
    init.handler = handler;
    init.name = name;
    init.help = help;
    init.group = group;
    return init;
}


} // namespace nodephysics
