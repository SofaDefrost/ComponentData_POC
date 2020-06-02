#pragma once

#include <sofa/core/DataTracker.h>
#include <sofa/core/objectmodel/DDGNode.h>
#include <sofa/core/objectmodel/ComponentState.h>
#include <sofa/core/objectmodel/BaseData.h>

namespace nodephysics
{






/// A DDGNode with trackable input Data (containing a DataTracker)
class SOFA_CORE_API DataTrackerDDGNode : public sofa::core::objectmodel::DDGNode
{
public:

    DataTrackerDDGNode() : sofa::core::objectmodel::DDGNode() {}

private:
    DataTrackerDDGNode(const DataTrackerDDGNode&);
    void operator=(const DataTrackerDDGNode&);

public:
    /// Create a DataCallback object associated with multiple Nodes.
    void addInputs(std::initializer_list<sofa::core::objectmodel::DDGNode*> inputs);
    void addOutputs(std::initializer_list<sofa::core::objectmodel::DDGNode*> outputs);

    /// Set dirty flag to false
    /// for the DDGNode and for all the tracked Data
    virtual void cleanDirty(const sofa::core::ExecParams* params = nullptr);


    /// utility function to ensure all inputs are up-to-date
    /// can be useful for particulary complex DDGNode
    /// with a lot input/output imbricated access
    void updateAllInputsIfDirty();

protected:

    /// @name Tracking Data mechanism
    /// each Data added to the DataTracker
    /// is tracked to be able to check if its value changed
    /// since their last clean, called by default
    /// in DataEngine::cleanDirty().
    /// @{

    sofa::core::DataTracker m_dataTracker;

    ///@}

};


///////////////////



    class SOFA_CORE_API DataTrackerEngine : public nodephysics::DataTrackerDDGNode
    {
    public:
        /// set the update function to call
        /// when asking for an output and any input changed.
        void addCallback(std::function<sofa::core::objectmodel::ComponentState(void)> f);

        /// Calls the callback when one of the data has changed.
        void update() override;

        void setName(const std::string& n)
        {
            m_name = n;
        }

        void setOwner(sofa::core::objectmodel::Base* owner)
        {
            m_owner = owner;
        }

    protected:
        std::vector<std::function<sofa::core::objectmodel::ComponentState(void)>> m_callbacks;
        std::string m_name {""};
        sofa::core::objectmodel::Base* m_owner {nullptr};
    };

} // namespace nodephysics
