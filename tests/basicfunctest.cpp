#include "../obs_ptr/IObserved.h"
#include "../obs_ptr/obs_ptr.h"
#include <gtest/gtest.h>

class SimpleObsTargetTestClass : public IObserved
{
    int a = 1;
};

TEST(BasicObsTest, DefaultConstruction)
{
    auto test = make_observer<SimpleObsTargetTestClass>();
    ASSERT_EQ(test->get_obs(), nullptr) << "Default-constructed obs_ptr is not null after construction.";
}

TEST(BasicObsTest, ConstructionWithTarget)
{
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    auto test = make_observer(var);
    ASSERT_EQ(test->get_obs(), var) << "obs_ptr.get() does not return pointer it was constructed with.";

    // Both destroyed
}

// Tests the IsObserver and Observers functions as well.
TEST(BasicObsTest, ManualAssignmentAndUnassignmentToTarget)
{
    auto var1 = std::make_shared<SimpleObsTargetTestClass>();
    auto var2 = std::make_shared<SimpleObsTargetTestClass>();
    auto ptr1 = make_observer<SimpleObsTargetTestClass>();
    auto ptr2 = make_observer<SimpleObsTargetTestClass>();

    ASSERT_EQ(var1->Observers(), 0);
    ASSERT_FALSE(var1->IsObserver(ptr1));
    ASSERT_FALSE(var1->IsObserver(ptr2));
    ASSERT_EQ(var2->Observers(), 0);
    ASSERT_FALSE(var2->IsObserver(ptr1));
    ASSERT_FALSE(var2->IsObserver(ptr2));

    {
        // Unset the observer, check that it is tracked by observed
        ptr1->set(var1);

        EXPECT_EQ(ptr1->get_obs(), var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr1->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 1);
    }

    {
        // Unset the observer, check that it is no longer tracked by observed
        ptr1->unset();

        EXPECT_EQ(ptr1->get_obs(), nullptr) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_FALSE(ptr1->is_set());
        EXPECT_FALSE(var1->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 0);
    }

    {
        // Reset the observer, check that it is again tracked (no residue behavior from unset)
        ptr1->set(var1);

        EXPECT_EQ(ptr1->get_obs(), var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr1->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 1);
    }

    {
        // Set second observer, check that both are now observed
        ptr2->set(var1);

        EXPECT_EQ(ptr2->get_obs(), var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr2->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_TRUE(var1->IsObserver(ptr2));
        EXPECT_EQ(var1->Observers(), 2);
    }

    {
        // Attempt to add observer again, check we are not tracking multiple copies
        ptr2->set(var1);

        EXPECT_EQ(ptr2->get_obs(), var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr2->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_TRUE(var1->IsObserver(ptr2));
        EXPECT_EQ(var1->Observers(), 2);
    }

    {
        // Unset second pointer, check that first is not affected
        ptr2->unset();

        EXPECT_EQ(ptr1->get_obs(), var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr1->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 1);

        // Setting back cause we need two pointers
        ptr2->set(var1);
        ASSERT_TRUE(var1->IsObserver(ptr2));
    }

    {
        // Use set method to change observer from var1 to var2, ensure cleanup.
        ptr1->set(var2);

        EXPECT_EQ(ptr1->get_obs(), var2) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr1->is_set());
        EXPECT_FALSE(var1->IsObserver(ptr1));
        EXPECT_TRUE(var2->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 1);
        EXPECT_EQ(var2->Observers(), 1);
    }
}

TEST(BasicObsTest, Destruction)
{
    auto ptrRoot = make_observer<SimpleObsTargetTestClass>();
    auto varRoot = std::make_shared<SimpleObsTargetTestClass>();

    {
        // Scoped destruction of target
        auto varScoped = std::make_shared<SimpleObsTargetTestClass>();
        ptrRoot->set(varScoped);
        ASSERT_NE(ptrRoot->get_obs(), nullptr);
    }
    EXPECT_EQ(ptrRoot->get_obs(), nullptr);

    {
        // Scoped destruction of pointer
        auto ptrScoped = make_observer(varRoot);

        EXPECT_NE(ptrScoped->get_obs(), nullptr);
        EXPECT_TRUE(varRoot->IsObserver(ptrScoped));
        EXPECT_EQ(varRoot->Observers(), 1);
    }
    EXPECT_EQ(varRoot->Observers(), 0);

    {
        // Manual destruction of target
        auto varScoped = std::make_shared<SimpleObsTargetTestClass>();
        ptrRoot->set(varScoped);
        ASSERT_NE(ptrRoot->get_obs(), nullptr);

        varScoped.reset();
    }
    EXPECT_EQ(ptrRoot->get_obs(), nullptr);

    {
        // Manual destruction of pointer
        auto ptrScoped = make_observer(varRoot);

        EXPECT_NE(ptrScoped->get_obs(), nullptr);
        EXPECT_TRUE(varRoot->IsObserver(ptrScoped));
        EXPECT_EQ(varRoot->Observers(), 1);

        ptrScoped.reset();
    }
    EXPECT_EQ(varRoot->Observers(), 0);
}

TEST(BasicObsTest, Copying)
{
    auto ptr1 = make_observer<SimpleObsTargetTestClass>();
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    ptr1->set(var);

    auto ptr2 = make_observer(ptr1); // Copy
    auto ptr3 = make_observer(ptr2); // Copy from a copy

    EXPECT_EQ(ptr1->get_obs(), var);
    EXPECT_EQ(ptr2->get_obs(), var);
    EXPECT_EQ(ptr3->get_obs(), var);

    EXPECT_TRUE(var->IsObserver(ptr1));
    EXPECT_TRUE(var->IsObserver(ptr2));
    EXPECT_TRUE(var->IsObserver(ptr3));
    EXPECT_EQ(var->Observers(), 3);

    ptr1 = nullptr;

    EXPECT_EQ(ptr2->get_obs(), var);
    EXPECT_EQ(ptr3->get_obs(), var);
    EXPECT_FALSE(var->IsObserver(ptr1));
    EXPECT_TRUE(var->IsObserver(ptr2));
    EXPECT_TRUE(var->IsObserver(ptr3));
    ASSERT_EQ(var->Observers(), 2);

    var.reset();

    EXPECT_EQ(ptr2->get_obs(), nullptr);
    EXPECT_EQ(ptr3->get_obs(), nullptr);
}

/*
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