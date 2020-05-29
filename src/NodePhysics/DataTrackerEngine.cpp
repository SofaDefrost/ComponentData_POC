#include "DataTrackerEngine.h"
#include <sofa/core/objectmodel/Base.h>

namespace nodephysics
{
void DataTrackerDDGNode::addInputs(std::initializer_list<sofa::core::objectmodel::DDGNode*> inputs)
{
    for(sofa::core::objectmodel::DDGNode* d : inputs)
        addInput(d);
}

void DataTrackerDDGNode::addOutputs(std::initializer_list<sofa::core::objectmodel::DDGNode*> outputs)
{
    for(sofa::core::objectmodel::DDGNode* d : outputs)
        addOutput(d);
}

void DataTrackerDDGNode::cleanDirty(const sofa::core::ExecParams*)
{
    sofa::core::objectmodel::DDGNode::cleanDirty();

    /// it is also time to clean the tracked Data
    m_dataTracker.clean();
}

void DataTrackerDDGNode::updateAllInputsIfDirty()
{
    const DDGLinkContainer& inputs = DDGNode::getInputs();
    for(auto input : inputs)
    {
        static_cast<sofa::core::objectmodel::BaseData*>(input)->updateIfDirty();
    }
}


////////////////////////////////


void DataTrackerEngine::addCallback(std::function<sofa::core::objectmodel::ComponentState(void)> f)
{
    m_callbacks.push_back(f);
}

void DataTrackerEngine::update()
{
    updateAllInputsIfDirty();

    sofa::core::objectmodel::ComponentState cs = sofa::core::objectmodel::ComponentState::Valid;
    for(auto& callback : m_callbacks)
    {
        sofa::core::objectmodel::ComponentState state = callback();
        if (state != sofa::core::objectmodel::ComponentState::Valid)
            cs = state;
    }
    if (m_owner)
        m_owner->d_componentstate.setValue(cs);
    cleanDirty();
}

}  // namespace nodephysics
