#include <string>
using std::string ;

#include <NodePhysics/Link.inl>

#include <SofaTest/Sofa_test.h>
#include <sofa/core/objectmodel/BaseObject.h>
using sofa::core::objectmodel::BaseObject;
using sofa::core::objectmodel::ComponentState;

#include <sofa/simulation/Simulation.h>
#include <SofaSimulationGraph/DAGSimulation.h>


class ClassA : public nodephysics::LinkHandler, public BaseObject
{
public:
    SOFA_CLASS(ClassA, BaseObject);

    sofa::Data<bool> input;
    sofa::Data<bool> output;

    ClassA()
        : nodephysics::LinkHandler(),
          Inherit1(),
          input(initData(&input, false, "in", "in")),
          output(initData(&output, "out", "out"))
    {
        addUpdateCallback("engineA", {&input}, [&]() -> ComponentState {
            std::cout << "in engineA" << std::endl;
            output.setValue(input.getValue());
            return sofa::core::objectmodel::ComponentState::Valid;
        }, {&output});
    }

    ~ClassA() override {}
};

class ClassB : public BaseObject, public nodephysics::LinkHandler
{
public:
    SOFA_CLASS(ClassB, BaseObject);

    ClassB()
        : Inherit1(),
          nodephysics::LinkHandler(),
          inputLink(nodephysics::initLink(this, "in", "help string")),
          output(initData(&output, "out", "out"))
    {
        addUpdateCallback("engineB", {&inputLink}, [&]() -> ComponentState {
            std::cout << "in engineB" << std::endl;
            output.setValue(inputLink.getLinkedDest()->output.getValue());
            return sofa::core::objectmodel::ComponentState::Valid;
        }, {&output});
    }

    ~ClassB() override {}

    nodephysics::Link<ClassA> inputLink;
    sofa::Data<bool> output;
};



namespace sofa
{

struct Link_test: public BaseTest
{
    ClassA::SPtr a;
    ClassB::SPtr b;
    Node::SPtr node;

    void SetUp() override
    {
        sofa::simulation::Simulation* simu;
        setSimulation(simu = new sofa::simulation::graph::DAGSimulation());

        node = simu->createNewGraph("root");

        a = sofa::core::objectmodel::New<ClassA>();
        a->setName("A");
        node->addObject(a);
        sofa::core::objectmodel::BaseObjectDescription bodA("A");
        bodA.setAttribute("in", "false");
        a->parse(&bodA);

        b = sofa::core::objectmodel::New<ClassB>();
        b->setName("B");
        node->addObject(b);
        sofa::core::objectmodel::BaseObjectDescription bodB("B");
        bodB.setAttribute("in", "@/A");
        bodB.setAttribute("out", "false");
        b->parse(&bodB);

        std::cout << "B inputLink dest: " <<  b->inputLink.getLinkedDest() << std::endl;
        std::cout << "B inputLink owner: " <<  b->inputLink.getLinkedDest() << std::endl;
    }

    void testGraphConsistency()
    {
        std::cout << "INITIAL STATE (everything but A::in should be dirty):" << std::endl;

        ASSERT_FALSE(a->input.isDirty());
        ASSERT_TRUE(a->output.isDirty());
        ASSERT_TRUE(a->d_componentstate.isDirty());
        ASSERT_TRUE(b->inputLink.isDirty());
        ASSERT_TRUE(b->output.isDirty());
        ASSERT_TRUE(b->d_componentstate.isDirty());

        b->output.getValue();
        std::cout << "\nAFTER accessing B::out (only B::componentState should be dirty):" << std::endl;
        ASSERT_FALSE(a->input.isDirty());
        ASSERT_FALSE(a->output.isDirty());
        ASSERT_FALSE(a->d_componentstate.isDirty());
        ASSERT_FALSE(b->inputLink.isDirty());
        ASSERT_FALSE(b->output.isDirty());
        ASSERT_TRUE(b->d_componentstate.isDirty());



        a->input.setValue(true); // Changing input value should dirtify all descendency...
        std::cout << "\nAFTER modifying A::in (should dirtify all but A::in):" << std::endl;
        ASSERT_FALSE(a->input.isDirty());
        ASSERT_TRUE(a->output.isDirty());
        ASSERT_TRUE(a->d_componentstate.isDirty());
        ASSERT_TRUE(b->inputLink.isDirty());
        ASSERT_TRUE(b->output.isDirty());
        ASSERT_TRUE(b->d_componentstate.isDirty());
    }


    void testLink_methods()
    {

        ASSERT_TRUE(a.get() == b->inputLink.getLinkedDest());
        ASSERT_TRUE(b.get() == b->inputLink.getOwner());

        ClassA::SPtr c = sofa::core::objectmodel::New<ClassA>();
        c->setName("C");
        node->addObject(c);

        b->inputLink.setLinkedDest(c.get());
        ASSERT_TRUE(b->inputLink.getLinkedDest() == c.get());
    }


    void testLink_ownership_methods()
    {
        ASSERT_EQ(a->getLinkHandlers().size(), 1);
        ASSERT_EQ(a->getLinkHandlers().front()->getName(), b->getName());


        ClassA::SPtr c = sofa::core::objectmodel::New<ClassA>();
        c->setName("C");
        node->addObject(c);
        b->inputLink.setLinkedDest(c.get());

        ASSERT_EQ(a->getLinkHandlers().size(), 0);
        ASSERT_EQ(c->getLinkHandlers().size(), 1);
        ASSERT_EQ(c->getLinkHandlers().front()->getName(), b->getName());

        b->inputLink.setLinkedDest(nullptr);
        ASSERT_EQ(c->getLinkHandlers().size(), 0);
    }
};

// Test
TEST_F(Link_test, testGraphConsistency )
{
    this->testGraphConsistency();
}

TEST_F(Link_test, testLink_methods )
{
    this->testLink_methods();
}

TEST_F(Link_test, testLink_ownership_methods )
{
    this->testLink_ownership_methods();
}

}  // namespace sofa
