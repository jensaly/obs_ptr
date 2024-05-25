#include "testclasses.h"

TEST(BasicObsTest, DefaultConstruction) {
    obs_ptr<SimpleObsTargetTestClass> test;
    ASSERT_EQ(test.get(), nullptr) << "Default-constructed obs_ptr is not null after construction.";
}

TEST(BasicObsTest, ConstructionWithTarget) {
    auto var = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test{var};
    ASSERT_EQ(test.get(), var) << "obs_ptr.get() does not return pointer it was constructed with.";

    delete var;
}

// Tests the isObserver and totalObservers functions as well.
TEST(BasicObsTest, AssignmentToTarget) {
    auto var = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test;
    
    ASSERT_EQ(var->totalObservers(), 0);
    ASSERT_FALSE(var->isObserver(test));

    test = var;
    ASSERT_EQ(test.get(), var) << "obs_ptr.get() does not return pointer it was constructed with.";
    ASSERT_TRUE(var->isObserver(test));
    ASSERT_EQ(var->totalObservers(), 1);
    delete var;
}

TEST(BasicObsTest, DestructionOfTarget) {
    auto var = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test;
    test = var;
    delete var;
    ASSERT_EQ(test.get(), nullptr);
}

// Testing assignment between two IMemObservees of the same type.
// Also testing reset to nullptr.
TEST(BasicObsTest, ReassignmentBetweenTargets) {
    auto var1 = new SimpleObsTargetTestClass();
    auto var2 = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test;
    test = var1;

    ASSERT_EQ(test.get(), var1);
    ASSERT_TRUE(var1->isObserver(test));
    ASSERT_EQ(var1->totalObservers(), 1);
    ASSERT_FALSE(var2->isObserver(test));
    ASSERT_EQ(var2->totalObservers(), 0);

    test = var2;

    ASSERT_EQ(test.get(), var2);
    ASSERT_TRUE(var2->isObserver(test));
    ASSERT_EQ(var2->totalObservers(), 1);
    ASSERT_FALSE(var1->isObserver(test));
    ASSERT_EQ(var1->totalObservers(), 0);

    test = nullptr;

    ASSERT_EQ(test.get(), nullptr);
    ASSERT_FALSE(var1->isObserver(test));
    ASSERT_EQ(var1->totalObservers(), 0);
    ASSERT_FALSE(var2->isObserver(test));
    ASSERT_EQ(var2->totalObservers(), 0);

    delete var1;
    delete var2;
}

TEST(BasicObsTest, CopyConstructor) {
    auto var = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test1;
    test1 = var;
    auto test2{test1};
    auto test3{test2};

    ASSERT_EQ(test1.get(), var);
    ASSERT_EQ(test2.get(), var);
    ASSERT_EQ(test3.get(), var);
    ASSERT_TRUE(var->isObserver(test1));
    ASSERT_TRUE(var->isObserver(test2));
    ASSERT_TRUE(var->isObserver(test3));
    ASSERT_EQ(var->totalObservers(), 3);

    test1 = nullptr;

    ASSERT_EQ(test1.get(), nullptr);
    ASSERT_EQ(test2.get(), var);
    ASSERT_EQ(test3.get(), var);
    ASSERT_FALSE(var->isObserver(test1));
    ASSERT_TRUE(var->isObserver(test2));
    ASSERT_TRUE(var->isObserver(test3));
    ASSERT_EQ(var->totalObservers(), 2);

    delete var;

    ASSERT_EQ(test2.get(), nullptr);
    ASSERT_EQ(test3.get(), nullptr);
}

// Assignment to another obs_ptr and to nullptr.
TEST(BasicObsTest, AssignmentToOtherObs) {
    auto var1 = new SimpleObsTargetTestClass();
    auto var2 = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test1{var1};
    obs_ptr<SimpleObsTargetTestClass> test2{var2};
    obs_ptr<SimpleObsTargetTestClass> test3;
    
    // Assignment from an active obs_ptr to another active obs_ptr.
    test1 = test2;
    ASSERT_FALSE(var1->isObserver(test1));
    ASSERT_TRUE(var2->isObserver(test1));
    ASSERT_TRUE(var2->isObserver(test2));
    ASSERT_EQ(var1->totalObservers(), 0);
    ASSERT_EQ(var2->totalObservers(), 2);

    // Assignment from an active obs_ptr to an inactive obs_ptr
    test2 = test3;
    ASSERT_FALSE(var2->isObserver(test2));
    ASSERT_EQ(var2->totalObservers(), 1);
    ASSERT_EQ(test2.get(), nullptr);

    // Assignment from an inactive obs_ptr to an active obs_ptr
    test3 = test1;
    ASSERT_TRUE(var2->isObserver(test1));
    ASSERT_TRUE(var2->isObserver(test3));
    ASSERT_EQ(var2->totalObservers(), 2);
    ASSERT_EQ(test3.get(), var2);

    delete var1;
    delete var2;
}

TEST(BasicObsTest, ComparisonOperators) {
    auto var1 = new SimpleObsTargetTestClass();
    auto var2 = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test1{var1};
    obs_ptr<SimpleObsTargetTestClass> test2{var2};
    obs_ptr<SimpleObsTargetTestClass> test3{var1};
    obs_ptr<SimpleObsTargetTestClass> test4;

    // Equality operator between obs_ptrs
    ASSERT_FALSE(test1 == test2);
    ASSERT_TRUE(test1 == test1);
    ASSERT_TRUE(test1 == test3);

    // Equality operator between obs_ptrs and raw pointers
    ASSERT_TRUE(test1 == var1);
    ASSERT_FALSE(test1 == var2);
    ASSERT_FALSE(test1 == test4);

    // Nullptr comparison
    ASSERT_FALSE(test1 == nullptr);
    ASSERT_TRUE(test4 == nullptr);

    delete var1;
    delete var2;
}



TEST(BasicObsTest, MoveConstructor) {
    auto var1 = new SimpleObsTargetTestClass();
    auto var2 = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test1{var1};
    obs_ptr<SimpleObsTargetTestClass> test2{var2};

    delete var1;
    delete var2;
}

TEST(BasicObsTest, DestructionOfPointer) { 
    auto var = new SimpleObsTargetTestClass();
    obs_ptr<SimpleObsTargetTestClass> test1{var};
    {
        obs_ptr<SimpleObsTargetTestClass> test2{var};
        ASSERT_TRUE(var->isObserver(test2));
        ASSERT_EQ(var->totalObservers(), 2);
    }
    ASSERT_EQ(var->totalObservers(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}