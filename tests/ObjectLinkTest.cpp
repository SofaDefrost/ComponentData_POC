#include <string>
using std::string ;

#include <SofaTest/Sofa_test.h>
#include <sofa/core/objectmodel/BaseObject.h>
using sofa::core::objectmodel::BaseObject ;

#include <sofa/simulation/Simulation.h>
#include <SofaSimulationGraph/DAGSimulation.h>

#include <NodePhysics/ObjectLink.h>

namespace nodephysics
{
class ClassA : public BaseObject
{
public:
    SOFA_CLASS(ClassA, BaseObject);
    
    sofa::Data<bool> input;
    sofa::Data<bool> output;
    sofa::core::DataTrackerEngine engine;

    ClassA()
        : Inherit1(),
          input(initData(&input, false, "input", "input")),
          output(initData(&output, "output", "output"))
    {
        engine.addInput(&input);
        engine.addOutputs({&d_componentstate, &output});
        engine.addCallback([=](sofa::core::DataTrackerEngine* e){
            e->updateAllInputsIfDirty();
            std::cout << "triggering update callback dependent on input" << std::endl;;
            output.setValue(input.getValue());
            d_componentstate.setValue(sofa::core::objectmodel::ComponentState::Valid);
            e->cleanDirty();
        });
        input.setDirtyOutputs();
    }

    ~ClassA() override {}


    inline friend std::istream& operator>>(std::istream& in, ClassA*& /*a*/)
    {
        std::string s;
        in >> s;
        //      a.setParent(s);
        return in;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const ClassA*& a)
    {
        out << "@" << a->getPathName();
        return out;
    }

};

class ClassB : public BaseObject
{
public:
    SOFA_CLASS(ClassB, BaseObject);

    ClassB()
        : Inherit1(),
          inputLink(initData(&inputLink, "inputLink", "inputLink")),
          output(initData(&output, "output", "output"))
    {
        engine.addInput(&inputLink);
        engine.addOutputs({&d_componentstate, &output});
        engine.addCallback([=](sofa::core::DataTrackerEngine* e){
            e->updateAllInputsIfDirty();
            std::cout << "triggering update callback dependent on inputLink" << std::endl;;
            output.setValue(inputLink.getValue()->output.getValue());
            d_componentstate.setValue(sofa::core::objectmodel::ComponentState::Valid);
            e->cleanDirty();
        });
        inputLink.setDirtyOutputs();
    }

    ~ClassB() override {}

    inline friend std::istream& operator>>(std::istream& in, ClassB*& /*a*/)
    {
        std::string s;
        in >> s;
        //      a.setParent(s);
        return in;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const ClassB*& a)
    {
        out << "@" << a->getPathName();
        return out;
    }

    nodephysics::ObjectLink<ClassA> inputLink;
    sofa::core::DataTrackerEngine engine;
    sofa::Data<bool> output;
};

}  // nodephysics

namespace sofa
{

struct ObjectLink_test: public BaseTest
{
    nodephysics::ClassA::SPtr a;
    nodephysics::ClassB::SPtr b;

    void SetUp() override
    {
        sofa::simulation::Simulation* simu;
        setSimulation(simu = new sofa::simulation::graph::DAGSimulation());

        Node::SPtr node = simu->createNewGraph("root");

        a = sofa::core::objectmodel::New<nodephysics::ClassA>();
        a->setName("A");
        node->addObject(a);
        sofa::core::objectmodel::BaseObjectDescription bodA("A");
        bodA.setAttribute("input", "false");
        a->parse(&bodA);

        b = sofa::core::objectmodel::New<nodephysics::ClassB>();
        b->setName("B");
        node->addObject(b);
        sofa::core::objectmodel::BaseObjectDescription bodB("B");
        bodB.setAttribute("inputLink", "@/A");
        bodB.setAttribute("output", "false");
        b->parse(&bodB);

//        b->inputLink.setValue(a.get());
    }


    void testObjectLink()
    {
        ASSERT_FALSE(a->input.getValue());
        ASSERT_FALSE(b->output.getValue());

        a->input.setValue(true); // Changing input value should dirtify all descendency...

        ASSERT_FALSE(a->input.isDirty()); // Value changed, but not dirtified

        ASSERT_TRUE(a->output.isDirty()); // Value dirtified
        ASSERT_TRUE(a->d_componentstate.isDirty() == true); // Value dirtified

        ASSERT_TRUE(b->inputLink.isDirty()); // Value dirtified
        ASSERT_TRUE(b->output.isDirty()); // Value dirtified
        ASSERT_TRUE(b->d_componentstate.isDirty()); // Value dirtified

        b->output.getValue();

        ASSERT_FALSE(b->output.isDirty()); // Value accessed, thus cleaned
        ASSERT_FALSE(b->inputLink.isDirty()); // Value accessed, thus cleaned

        ASSERT_FALSE(b->d_componentstate.isDirty()); // The component state should be cleaned, as setValue was called in callback!

        ASSERT_FALSE(a->output.isDirty()); // Value accessed, thus cleaned
        ASSERT_FALSE(a->d_componentstate.isDirty()); // Value accessed, thus cleaned
    }

};

// Test
TEST_F(ObjectLink_test, testObjectLink )
{
    this->testObjectLink();
}

}  // namespace sofa
