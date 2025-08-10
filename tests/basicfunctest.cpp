#include "../obs_ptr/IObserved.h"
#include "../obs_ptr/obs_ptr.h"
#include <gtest/gtest.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
#include <fstream>

class SimpleObsTargetTestClass : public IObserved
{
    int a = 1;

public:
    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(a);
    }
};

CEREAL_REGISTER_TYPE(SimpleObsTargetTestClass);
CEREAL_REGISTER_POLYMORPHIC_RELATION(IObserved, SimpleObsTargetTestClass)

TEST(BasicObsTest, DefaultConstruction)
{
    auto ptr1 = make_observer<SimpleObsTargetTestClass>();
    ASSERT_EQ(ptr1->get_obs(), nullptr) << "Default-constructed obs_ptr is not null after construction.";

    auto ptr2 = make_observer<SimpleObsTargetTestClass>();
    EXPECT_EQ(ptr2->get_obs(), nullptr) << "Another default-constructed obs_ptr is not null.";
    EXPECT_EQ(ptr1->get_obs(), ptr1->get_obs()) << "Two default-constructed obs_ptr do not compare equal.";
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

TEST(BasicObsTest, ComparisonOperators)
{
    auto var1 = std::make_shared<SimpleObsTargetTestClass>();
    auto var2 = std::make_shared<SimpleObsTargetTestClass>();
    auto pVar1_1 = make_observer(var1);
    auto pVar2_1 = make_observer(var2);
    auto pVar1_2 = make_observer(var1);
    auto pNull = make_observer<SimpleObsTargetTestClass>();

    // Equality operator between obs_ptrs
    EXPECT_NE(pVar1_1->get_obs(), pVar2_1->get_obs());
    EXPECT_EQ(pVar1_1->get_obs(), pVar1_1->get_obs());
    EXPECT_EQ(pVar1_1->get_obs(), pVar1_2->get_obs());

    // Nullptr comparison
    EXPECT_NE(pVar1_1->get_obs(), nullptr);
    EXPECT_EQ(pNull->get_obs(), nullptr);

    // Cross-type comparisons with nullptr
    EXPECT_TRUE(nullptr == pNull->get_obs());
    EXPECT_TRUE(pNull->get_obs() == nullptr);
    EXPECT_FALSE(nullptr != pNull->get_obs());
    EXPECT_FALSE(pNull->get_obs() != nullptr);

    EXPECT_FALSE(nullptr == pVar1_1->get_obs());
    EXPECT_FALSE(pVar1_1->get_obs() == nullptr);
    EXPECT_TRUE(nullptr != pVar1_1->get_obs());
    EXPECT_TRUE(pVar1_1->get_obs() != nullptr);

    // Cross-type comparisons with shared_ptr<T>
    EXPECT_TRUE(var1 == pVar1_1->get_obs());
    EXPECT_TRUE(pVar1_1->get_obs() == var1);
    EXPECT_FALSE(var1 != pVar1_1->get_obs());
    EXPECT_FALSE(pVar1_1->get_obs() != var1);

    EXPECT_FALSE(var1 == pVar2_1->get_obs());
    EXPECT_FALSE(pVar2_1->get_obs() == var1);
    EXPECT_TRUE(var1 != pVar2_1->get_obs());
    EXPECT_TRUE(pVar2_1->get_obs() != var1);

    // obs_ptr vs obs_ptr
    EXPECT_TRUE(pVar1_1->get_obs() == pVar1_2->get_obs());
    EXPECT_FALSE(pVar1_1->get_obs() != pVar1_2->get_obs());

    EXPECT_FALSE(pVar1_1->get_obs() == pVar2_1->get_obs());
    EXPECT_TRUE(pVar1_1->get_obs() != pVar2_1->get_obs());
}

TEST(BasicObsTest, MoveConstructorPreservesObservation)
{
    {
        // Move construction on an empty obs_ptr

        auto ptrOrigEmpty = make_observer<SimpleObsTargetTestClass>();
        auto ptrMovedEmpty = make_observer(ptrOrigEmpty);

        EXPECT_EQ(ptrOrigEmpty->get_obs(), nullptr);
        EXPECT_EQ(ptrMovedEmpty->get_obs(), nullptr);
    }
    {
        // Move construction on an obs_ptr with stuff

        auto var = std::make_shared<SimpleObsTargetTestClass>();
        ASSERT_EQ(var->Observers(), 0);

        auto ptrOrig = make_observer(var);
        EXPECT_TRUE(var->IsObserver(ptrOrig));
        EXPECT_EQ(var->Observers(), 1);

        auto ptrMoved = move_observer(ptrOrig);

        EXPECT_TRUE(var->IsObserver(ptrMoved));
        EXPECT_FALSE(var->IsObserver(ptrOrig));
        EXPECT_EQ(var->Observers(), 1);
        EXPECT_EQ(ptrMoved->get_obs(), var);
        EXPECT_EQ(ptrOrig->get_obs(), nullptr);
        EXPECT_FALSE(ptrOrig->is_set());
        EXPECT_TRUE(ptrMoved->is_set());
    }
}

TEST(CerealTest, SerializationToBinary)
{
    std::string oarchiveName = "SerializationToBinaryOut";
    std::fstream oarchiveStream(oarchiveName, std::ios::binary);
    cereal::BinaryOutputArchive archive{oarchiveStream};

    {
        auto ptr = make_observer<SimpleObsTargetTestClass>();
        auto var = std::make_shared<SimpleObsTargetTestClass>();
        ptr->set(var);

        archive(var);
        archive(ptr);
    }
}