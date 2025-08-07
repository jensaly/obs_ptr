#include "testclasses.h"
/*
TEST(BasicObsTest, DefaultConstruction)
{
    auto test = make_empty_observer<SimpleObsTargetTestClass>();
    ASSERT_EQ(test->get_raw(), nullptr) << "Default-constructed obs_ptr is not null after construction.";
}

TEST(BasicObsTest, ConstructionWithTarget)
{
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    auto test = make_observer(var);
    ASSERT_EQ(test->get_raw(), var.get()) << "obs_ptr.get() does not return pointer it was constructed with.";
}

// Tests the IsObserver and Observers functions as well.
TEST(BasicObsTest, AssignmentToTarget)
{
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    obs_ptr<SimpleObsTargetTestClass> test = make_empty_observer<SimpleObsTargetTestClass>();

    ASSERT_EQ(var->Observers(), 0);
    ASSERT_FALSE(var->IsObserver(test));

    test->set(var);

    EXPECT_EQ(test->get_raw(), var.get()) << "obs_ptr.get() does not return pointer it was constructed with.";
    EXPECT_TRUE(var->IsObserver(test));
    EXPECT_EQ(var->Observers(), 1);
}

TEST(BasicObsTest, ScopedDestructionOfTarget)
{
    obs_ptr<SimpleObsTargetTestClass> test;
    {
        auto var = std::make_shared<SimpleObsTargetTestClass>();
        test = make_observer(var);
        ASSERT_NE(test->get_raw(), nullptr);
    }
    ASSERT_EQ(test->get_raw(), nullptr);
}

// Testing assignment between two IObserveds of the same type.
// Also testing reset to nullptr.
TEST(BasicObsTest, ReassignmentBetweenTargets)
{
    auto var1 = std::make_shared<SimpleObsTargetTestClass>();
    auto var2 = std::make_shared<SimpleObsTargetTestClass>();
    obs_ptr<SimpleObsTargetTestClass> test;
    test->set(var1);

    ASSERT_EQ(test->get_raw(), var1.get());
    ASSERT_TRUE(var1->IsObserver(test));
    ASSERT_EQ(var1->Observers(), 1);
    ASSERT_FALSE(var2->IsObserver(test));
    ASSERT_EQ(var2->Observers(), 0);

    test->set(var2);

    ASSERT_EQ(test->get_raw(), var2.get());
    ASSERT_TRUE(var2->IsObserver(test));
    ASSERT_EQ(var2->Observers(), 1);
    ASSERT_FALSE(var1->IsObserver(test));
    ASSERT_EQ(var1->Observers(), 0);

    test->set(nullptr);

    ASSERT_EQ(test->get_raw(), nullptr);
    ASSERT_FALSE(var1->IsObserver(test));
    ASSERT_EQ(var1->Observers(), 0);
    ASSERT_FALSE(var2->IsObserver(test));
    ASSERT_EQ(var2->Observers(), 0);
}

TEST(BasicObsTest, CopyConstructor)
{
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    obs_ptr<SimpleObsTargetTestClass> test1;
    test1->set(var);
    auto test2{test1};
    auto test3{test2};

    ASSERT_EQ(test1->get_raw(), var.get());
    ASSERT_EQ(test2->get_raw(), var.get());
    ASSERT_EQ(test3->get_raw(), var.get());
    ASSERT_TRUE(var->IsObserver(test1));
    ASSERT_TRUE(var->IsObserver(test2));
    ASSERT_TRUE(var->IsObserver(test3));
    ASSERT_EQ(var->Observers(), 3);

    test1 = nullptr;

    ASSERT_EQ(test1->get_raw(), nullptr);
    ASSERT_EQ(test2->get_raw(), var.get());
    ASSERT_EQ(test3->lock(), var);
    ASSERT_FALSE(var->IsObserver(test1));
    ASSERT_TRUE(var->IsObserver(test2));
    ASSERT_TRUE(var->IsObserver(test3));
    ASSERT_EQ(var->Observers(), 2);

    var.reset();

    ASSERT_EQ(test2->get_raw(), nullptr);
    ASSERT_EQ(test3->get_raw(), nullptr);
}

// Assignment to another obs_ptr and to nullptr.
TEST(BasicObsTest, AssignmentToOtherObs)
{
    auto var1 = std::make_shared<SimpleObsTargetTestClass>();
    auto var2 = std::make_shared<SimpleObsTargetTestClass>();
    obs_ptr<SimpleObsTargetTestClass> test1 = make_observer(var1);
    obs_ptr<SimpleObsTargetTestClass> test2 = make_observer(var2);
    obs_ptr<SimpleObsTargetTestClass> test3;

    // Assignment from an active obs_ptr to another active obs_ptr.
    test1 = test2;
    ASSERT_FALSE(var1->IsObserver(test1));
    ASSERT_TRUE(var2->IsObserver(test1));
    ASSERT_TRUE(var2->IsObserver(test2));
    ASSERT_EQ(var1->Observers(), 0);
    ASSERT_EQ(var2->Observers(), 2);

    // Assignment from an active obs_ptr to an inactive obs_ptr
    test2 = test3;
    ASSERT_FALSE(var2->IsObserver(test2));
    ASSERT_EQ(var2->Observers(), 1);
    ASSERT_EQ(test2->get_raw(), nullptr);

    // Assignment from an inactive obs_ptr to an active obs_ptr
    test3 = test1;
    ASSERT_TRUE(var2->IsObserver(test1));
    ASSERT_TRUE(var2->IsObserver(test3));
    ASSERT_EQ(var2->Observers(), 2);
    ASSERT_EQ(test3->lock(), var2);
}

TEST(BasicObsTest, ComparisonOperators)
{
    auto var1 = std::make_shared<SimpleObsTargetTestClass>();
    auto var2 = std::make_shared<SimpleObsTargetTestClass>();
    obs_ptr<SimpleObsTargetTestClass> test1 = make_observer(var1);
    obs_ptr<SimpleObsTargetTestClass> test2 = make_observer(var2);
    obs_ptr<SimpleObsTargetTestClass> test3 = make_observer(var1);
    obs_ptr<SimpleObsTargetTestClass> test4;

    // Equality operator between obs_ptrs
    ASSERT_FALSE(test1 == test2);
    ASSERT_TRUE(test1 == test1);
    ASSERT_TRUE(test1 == test3);

    // Nullptr comparison
    ASSERT_FALSE(test1 == nullptr);
    ASSERT_TRUE(test4 == nullptr);
}

TEST(BasicObsTest, MoveConstructor)
{
    auto var1 = std::make_shared<SimpleObsTargetTestClass>();
    auto var2 = std::make_shared<SimpleObsTargetTestClass>();
    obs_ptr<SimpleObsTargetTestClass> test1 = make_observer(var1);
    obs_ptr<SimpleObsTargetTestClass> test2 = make_observer(var2);
}

TEST(BasicObsTest, DestructionOfPointer)
{
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    obs_ptr<SimpleObsTargetTestClass> test1 = make_observer(var);
    {
        obs_ptr<SimpleObsTargetTestClass> test2 = make_observer(var);
        ASSERT_TRUE(var->IsObserver(test2));
        ASSERT_EQ(var->Observers(), 2);
    }
    ASSERT_EQ(var->Observers(), 1);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
    */