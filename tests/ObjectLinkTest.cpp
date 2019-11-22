#include <string>
using std::string ;

#include <SofaTest/Sofa_test.h>
#include <sofa/core/objectmodel/BaseObject.h>
using sofa::core::objectmodel::BaseObject ;

#include <NodePhysics/ObjectLink.h>

namespace nodephysics::test
{
  class A : public BaseObject
  {
  public:
    SOFA_CLASS(A, BaseObject)
    
    A() : Inherit1(),
	  input(initData(&input, false, "input", "input"))
	  output(initData(&output, (int)UNDEFINED, "output", "output"))
    {
      addEngineCallback("", {&input}, [](this){
	  output.setValue(input.getValue());
	  d_componentstate.setValue(sofa::core::objectmodel::ComponentState::Valid);
	}, {&output, &d_componentstate})
    }

    ~A() override {}

    Data<bool> input;
    Data<bool> output;
  };

  class B : public BaseObject
  {
  public:
    SOFA_CLASS(B, BaseObject)

    B() : Inherit1(),
	  inputLink(initData(&inputLink, "inputLink", "inputLink"))
	  output(initData(&output, (int)UNDEFINED, "output", "output"))
    {
      addEngineCallback("", {&inputLink}, [](this){
	  msg_info("triggering update callback dependent on inputLink")
	  output.setValue(inputLink.getValue()->input.getValue());
	  d_componentstate.setValue(sofa::core::objectmodel::ComponentState::Valid);
	}, {&output, &d_componentstate})
    }

    ~A() override {}

    ObjectLink<A> inputLink;
    Data<bool> output;
  };

  


TYPED_TEST(UnilateralPlaneConstraintTest, NormalBehavior) {
    ASSERT_NO_THROW(this->normalTests()) ;
}


  
}  // nodephysics::test

namespace sofa
{

  struct ObjectLink_test: public BaseTest
  {
    A::SPtr a;
    B::SPtr b;

    void SetUp() override
    {
      a = sofa::core::objectmodel::New<A>();
      b = sofa::core::objectmodel::New<B>();
      b.inputLink.setParent(a.getLinkPath());
    }

    void testObjectLink()
    {
      ASSERT_TRUE(b.input.getValue()==&a);

      // modifying input sets it as dirty
      a.input.setValue(true);
      ASSERT_TRUE(b.input.isDirty()==true);

      // retrieving B's output triggers the update chain, and stash A's input value into B's output. 
      b.output.getValue();
      ASSERT_TRUE(b.output.getValue() == "true");
    }

  };

  // Test
  TEST_F(ObjectLink_test, testObjectLink )
  {
    this->testObjectLink();
  }

}  // namespace sofa
