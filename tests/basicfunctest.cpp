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
    void save(Archive &ar) const
    {
        ar(cereal::base_class<IObserved>(this), a);
    }

    template <class Archive>
    void load(Archive &ar)
    {
        ar(cereal::base_class<IObserved>(this), a);
    }
};

CEREAL_REGISTER_TYPE(SimpleObsTargetTestClass);
CEREAL_REGISTER_POLYMORPHIC_RELATION(IObserved, SimpleObsTargetTestClass)

CEREAL_REGISTER_TYPE(obs_ptr<SimpleObsTargetTestClass>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(IObserver, obs_ptr<SimpleObsTargetTestClass>)

TEST(BasicObsTest, DefaultConstruction)
{
    auto ptr1 = make_observer<SimpleObsTargetTestClass>();
    ASSERT_EQ(ptr1, nullptr) << "Default-constructed obs_ptr is not null after construction.";

    auto ptr2 = make_observer<SimpleObsTargetTestClass>();
    EXPECT_EQ(ptr2, nullptr) << "Another default-constructed obs_ptr is not null.";
    EXPECT_EQ(ptr1, ptr1) << "Two default-constructed obs_ptr do not compare equal.";
}

TEST(BasicObsTest, ConstructionWithTarget)
{
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    auto test = make_observer(var);
    ASSERT_EQ(test, var) << "obs_ptr.get() does not return pointer it was constructed with.";

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

        EXPECT_EQ(ptr1, var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr1->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 1);
    }

    {
        // Unset the observer, check that it is no longer tracked by observed
        ptr1->unset();

        EXPECT_EQ(ptr1, nullptr) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_FALSE(ptr1->is_set());
        EXPECT_FALSE(var1->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 0);
    }

    {
        // Reset the observer, check that it is again tracked (no residue behavior from unset)
        ptr1->set(var1);

        EXPECT_EQ(ptr1, var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr1->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_EQ(var1->Observers(), 1);
    }

    {
        // Set second observer, check that both are now observed
        ptr2->set(var1);

        EXPECT_EQ(ptr2, var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr2->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_TRUE(var1->IsObserver(ptr2));
        EXPECT_EQ(var1->Observers(), 2);
    }

    {
        // Attempt to add observer again, check we are not tracking multiple copies
        ptr2->set(var1);

        EXPECT_EQ(ptr2, var1) << "obs_ptr.get() does not return pointer it was constructed with.";
        EXPECT_TRUE(ptr2->is_set());
        EXPECT_TRUE(var1->IsObserver(ptr1));
        EXPECT_TRUE(var1->IsObserver(ptr2));
        EXPECT_EQ(var1->Observers(), 2);
    }

    {
        // Unset second pointer, check that first is not affected
        ptr2->unset();

        EXPECT_EQ(ptr1, var1) << "obs_ptr.get() does not return pointer it was constructed with.";
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

        EXPECT_EQ(ptr1, var2) << "obs_ptr.get() does not return pointer it was constructed with.";
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
        ASSERT_NE(ptrRoot, nullptr);
    }
    EXPECT_EQ(ptrRoot, nullptr);

    {
        // Scoped destruction of pointer
        auto ptrScoped = make_observer(varRoot);

        EXPECT_NE(ptrScoped, nullptr);
        EXPECT_TRUE(varRoot->IsObserver(ptrScoped));
        EXPECT_EQ(varRoot->Observers(), 1);
    }
    EXPECT_EQ(varRoot->Observers(), 0);

    {
        // Manual destruction of target
        auto varScoped = std::make_shared<SimpleObsTargetTestClass>();
        ptrRoot->set(varScoped);
        ASSERT_NE(ptrRoot, nullptr);

        varScoped.reset();
    }
    EXPECT_EQ(ptrRoot, nullptr);

    {
        // Manual destruction of pointer
        auto ptrScoped = make_observer(varRoot);

        EXPECT_NE(ptrScoped, nullptr);
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

    auto ptr2 = copy_observer(ptr1); // Copy
    auto ptr3 = copy_observer(ptr2); // Copy from a copy

    EXPECT_EQ(ptr1, var);
    EXPECT_EQ(ptr2, var);
    EXPECT_EQ(ptr3, var);

    EXPECT_TRUE(var->IsObserver(ptr1));
    EXPECT_TRUE(var->IsObserver(ptr2));
    EXPECT_TRUE(var->IsObserver(ptr3));
    EXPECT_EQ(var->Observers(), 3);

    ptr1 = nullptr;

    EXPECT_EQ(ptr2, var);
    EXPECT_EQ(ptr3, var);
    EXPECT_FALSE(var->IsObserver(ptr1));
    EXPECT_TRUE(var->IsObserver(ptr2));
    EXPECT_TRUE(var->IsObserver(ptr3));
    ASSERT_EQ(var->Observers(), 2);

    var.reset();

    EXPECT_EQ(ptr2, nullptr);
    EXPECT_EQ(ptr3, nullptr);
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
    EXPECT_NE(pVar1_1, pVar2_1);
    EXPECT_EQ(pVar1_1, pVar1_1);
    EXPECT_EQ(pVar1_1, pVar1_2);

    // Nullptr comparison
    EXPECT_NE(pVar1_1, nullptr);
    EXPECT_EQ(pNull, nullptr);

    // Cross-type comparisons with nullptr
    EXPECT_TRUE(nullptr == pNull);
    EXPECT_TRUE(pNull == nullptr);
    EXPECT_FALSE(nullptr != pNull);
    EXPECT_FALSE(pNull != nullptr);

    EXPECT_FALSE(nullptr == pVar1_1);
    EXPECT_FALSE(pVar1_1 == nullptr);
    EXPECT_TRUE(nullptr != pVar1_1);
    EXPECT_TRUE(pVar1_1 != nullptr);

    // Cross-type comparisons with shared_ptr<T>
    EXPECT_TRUE(var1 == pVar1_1);
    EXPECT_TRUE(pVar1_1 == var1);
    EXPECT_FALSE(var1 != pVar1_1);
    EXPECT_FALSE(pVar1_1 != var1);

    EXPECT_FALSE(var1 == pVar2_1);
    EXPECT_FALSE(pVar2_1 == var1);
    EXPECT_TRUE(var1 != pVar2_1);
    EXPECT_TRUE(pVar2_1 != var1);

    // obs_ptr vs obs_ptr
    EXPECT_TRUE(pVar1_1 == pVar1_2);
    EXPECT_FALSE(pVar1_1 != pVar1_2);

    EXPECT_FALSE(pVar1_1 == pVar2_1);
    EXPECT_TRUE(pVar1_1 != pVar2_1);
}

TEST(BasicObsTest, MoveConstructorPreservesObservation)
{
    {
        // Move construction on an empty obs_ptr

        auto ptrOrigEmpty = make_observer<SimpleObsTargetTestClass>();
        auto ptrMovedEmpty = copy_observer(ptrOrigEmpty);

        EXPECT_EQ(ptrOrigEmpty, nullptr);
        EXPECT_EQ(ptrMovedEmpty, nullptr);
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
        EXPECT_EQ(ptrMoved, var);
        EXPECT_EQ(ptrOrig, nullptr);
        EXPECT_FALSE(ptrOrig->is_set());
        EXPECT_TRUE(ptrMoved->is_set());
    }
}

// Basic serialization test
TEST(CerealTest, SerializationToBinary)
{
    std::stringstream ss;

    ASSERT_TRUE(ss.good());

    {
        cereal::BinaryOutputArchive archive{ss};

        auto ptr = make_observer<SimpleObsTargetTestClass>();
        auto var = std::make_shared<SimpleObsTargetTestClass>();
        ptr->set(var);

        EXPECT_NO_THROW(archive(var));
        EXPECT_NO_THROW(archive(ptr));
    }
    {
        cereal::BinaryInputArchive archive{ss};

        auto ptr = make_observer<SimpleObsTargetTestClass>();
        auto var = std::make_shared<SimpleObsTargetTestClass>();

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(ptr));

        EXPECT_TRUE(ptr->is_set());
        EXPECT_EQ(ptr, var);
        EXPECT_EQ(var->Observers(), 1);
        EXPECT_TRUE(var->IsObserver(ptr));
    }
}

TEST(CerealTest, SerializeUnsetObserver)
{
    // Serialize an unset observer (no target saved).
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive{ss};
        auto ptr = make_observer<SimpleObsTargetTestClass>(); // default, unset
        EXPECT_FALSE(ptr->is_set());
        ASSERT_NO_THROW(oarchive(ptr));
    }

    // Deserialize into a fresh observer and verify it remains unset.
    {
        cereal::BinaryInputArchive iarchive{ss};
        auto ptr = make_observer<SimpleObsTargetTestClass>();
        ASSERT_NO_THROW(iarchive(ptr));
        EXPECT_FALSE(ptr->is_set());
        EXPECT_EQ(ptr, nullptr);
    }
}

TEST(CerealTest, SerializeMultipleObserversToSameTarget)
{
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive archive{ss};

        auto var = std::make_shared<SimpleObsTargetTestClass>();
        auto p1 = make_observer(var);
        auto p2 = make_observer(var);
        auto p3 = make_observer(var);

        // sanity before serialize
        EXPECT_EQ(var->Observers(), 3);
        EXPECT_TRUE(var->IsObserver(p1));
        EXPECT_TRUE(var->IsObserver(p2));
        EXPECT_TRUE(var->IsObserver(p3));

        // Save target first, then the observers (order required so weak_ptr can be resolved)
        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(p1));
        ASSERT_NO_THROW(archive(p2));
        ASSERT_NO_THROW(archive(p3));
    }

    {
        cereal::BinaryInputArchive archive{ss};

        // Load target and three observers sequentially
        auto varL = std::make_shared<SimpleObsTargetTestClass>();
        auto p1L = make_observer<SimpleObsTargetTestClass>();
        auto p2L = make_observer<SimpleObsTargetTestClass>();
        auto p3L = make_observer<SimpleObsTargetTestClass>();

        ASSERT_NO_THROW(archive(varL));
        ASSERT_NO_THROW(archive(p1L));
        ASSERT_NO_THROW(archive(p2L));
        ASSERT_NO_THROW(archive(p3L));

        // All observers must point to same target instance (varL) and be registered
        EXPECT_TRUE(p1L->is_set());
        EXPECT_TRUE(p2L->is_set());
        EXPECT_TRUE(p3L->is_set());

        EXPECT_EQ(p1L, varL);
        EXPECT_EQ(p2L, varL);
        EXPECT_EQ(p3L, varL);

        EXPECT_EQ(varL->Observers(), 3);
        EXPECT_TRUE(varL->IsObserver(p1L));
        EXPECT_TRUE(varL->IsObserver(p2L));
        EXPECT_TRUE(varL->IsObserver(p3L));
    }
}

TEST(CerealTest, SerializeAfterMoveObserver)
{
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive archive{ss};

        auto var = std::make_shared<SimpleObsTargetTestClass>();
        auto orig = make_observer(var);
        // Move semantics via helper
        auto moved = move_observer(orig);

        // orig should be unset after move_observer
        ASSERT_FALSE(orig->is_set());
        ASSERT_TRUE(moved->is_set());
        ASSERT_EQ(var->Observers(), 1);

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(moved));
    }

    {
        cereal::BinaryInputArchive archive{ss};

        auto varL = std::make_shared<SimpleObsTargetTestClass>();
        auto movedL = make_observer<SimpleObsTargetTestClass>();

        ASSERT_NO_THROW(archive(varL));
        ASSERT_NO_THROW(archive(movedL));

        EXPECT_TRUE(movedL->is_set());
        EXPECT_EQ(movedL, varL);
        EXPECT_EQ(varL->Observers(), 1);
        EXPECT_TRUE(varL->IsObserver(movedL));
    }
}

TEST(CerealTest, DestructionAfterSerialize)
{
    std::stringstream ss;
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    {
        cereal::BinaryOutputArchive archive{ss};
        auto ptr = make_observer(var);

        EXPECT_TRUE(ptr->is_set());
        EXPECT_TRUE(var->IsObserver(ptr));
        EXPECT_EQ(var->Observers(), 1);

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(ptr));
    }

    EXPECT_EQ(var->Observers(), 0);
    var.reset();

    {
        cereal::BinaryInputArchive archive{ss};

        var = std::make_shared<SimpleObsTargetTestClass>();
        auto ptr = make_observer<SimpleObsTargetTestClass>();

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(ptr));

        EXPECT_TRUE(ptr->is_set());
        EXPECT_EQ(ptr, var);
        EXPECT_EQ(var->Observers(), 1);
        EXPECT_TRUE(var->IsObserver(ptr));
    }

    EXPECT_EQ(var->Observers(), 0);
    var.reset();
    ss.clear();
    ss.seekg(0, std::ios::beg);

    {
        cereal::BinaryInputArchive archive{ss};

        var = std::make_shared<SimpleObsTargetTestClass>();
        auto varLocal = std::make_shared<SimpleObsTargetTestClass>();
        auto ptr = make_observer<SimpleObsTargetTestClass>();

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(ptr));

        EXPECT_TRUE(ptr->is_set());
        EXPECT_EQ(ptr, var);
        EXPECT_EQ(var->Observers(), 1);
        EXPECT_TRUE(var->IsObserver(ptr));

        ptr->unset();

        EXPECT_FALSE(ptr->is_set());
        EXPECT_EQ(ptr, nullptr);
        EXPECT_EQ(var->Observers(), 0);
        EXPECT_FALSE(var->IsObserver(ptr));

        ptr->set(varLocal);

        EXPECT_TRUE(ptr->is_set());
        EXPECT_EQ(ptr, varLocal);
        EXPECT_EQ(varLocal->Observers(), 1);
        EXPECT_TRUE(varLocal->IsObserver(ptr));
    }
}

TEST(CerealTest, SomePointersNotSerialized)
{
    // GUI may use obs_ptr. We do not want to serialize these.
    std::stringstream ss;
    auto var = std::make_shared<SimpleObsTargetTestClass>();
    {
        // Create two pointers, serialize only one
        cereal::BinaryOutputArchive archive{ss};
        auto ptr1 = make_observer(var);
        auto ptr2 = make_observer(var);

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(ptr1));

        EXPECT_TRUE(ptr1->is_set());
        EXPECT_TRUE(ptr2->is_set());
        EXPECT_EQ(ptr1, var);
        EXPECT_EQ(ptr2, var);
        EXPECT_EQ(var->Observers(), 2);
        EXPECT_TRUE(var->IsObserver(ptr1));
        EXPECT_TRUE(var->IsObserver(ptr2));
    }

    EXPECT_EQ(var->Observers(), 0);
    var.reset();

    {

        var = std::make_shared<SimpleObsTargetTestClass>();
        auto ptr = make_observer<SimpleObsTargetTestClass>();
        // Recreate the serialized pointer, other pointer does not yet exist
        {
            cereal::BinaryInputArchive archive{ss};

            ASSERT_NO_THROW(archive(var));
            ASSERT_NO_THROW(archive(ptr));
        }

        // Should now only have observer. Other should NOT have been created
        EXPECT_TRUE(ptr->is_set());
        EXPECT_EQ(ptr, var);
        EXPECT_EQ(var->Observers(), 1);
        EXPECT_TRUE(var->IsObserver(ptr));
    }
}

TEST(CerealTest, StaticPointerNotSerialized)
{
    // GUI may use obs_ptr. We do not want to serialize these.
    std::stringstream ss;
    static auto pStatic = make_observer<SimpleObsTargetTestClass>();
    {
        auto var = std::make_shared<SimpleObsTargetTestClass>();
        // Create two pointers, serialize only one
        cereal::BinaryOutputArchive archive{ss};
        auto ptr = make_observer(var);
        pStatic->set(var);

        ASSERT_EQ(var->Observers(), 2);
        ASSERT_TRUE(var->IsObserver(ptr));
        ASSERT_TRUE(var->IsObserver(pStatic));

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(ptr));
    }

    ASSERT_FALSE(pStatic->is_set());

    {

        auto var = std::make_shared<SimpleObsTargetTestClass>();
        auto ptr = make_observer<SimpleObsTargetTestClass>();
        // Recreate the serialized pointer, other pointer does not yet exist
        {
            cereal::BinaryInputArchive archive{ss};

            ASSERT_NO_THROW(archive(var));
            ASSERT_NO_THROW(archive(ptr));
        }

        EXPECT_EQ(var->Observers(), 1);
        EXPECT_TRUE(var->IsObserver(ptr));
        EXPECT_FALSE(var->IsObserver(pStatic));
    }

    ASSERT_FALSE(pStatic->is_set());
}

struct ObsPtrOwner
{
    int a = 1;
    std::shared_ptr<obs_ptr<SimpleObsTargetTestClass>> pObserver;

    ObsPtrOwner()
    {
        pObserver = make_observer<SimpleObsTargetTestClass>(nullptr, std::bind(&ObsPtrOwner::handle_target_deletion, this));
    }

    void handle_target_deletion()
    {
        a++;
    }

    template <class Archive>
    void save(Archive &archive) const
    {
        archive(pObserver);
    }

    template <class Archive>
    void load(Archive &archive)
    {
        archive(pObserver);
        pObserver->set_cb(std::bind(&ObsPtrOwner::handle_target_deletion, this));
    }
};

TEST(CallbackObsTest, ConstructionAndDestruction)
{
    auto obsPtrOwner = ObsPtrOwner();
    auto &ptr = obsPtrOwner.pObserver;
    ptr = make_observer<SimpleObsTargetTestClass>();

    ASSERT_EQ(obsPtrOwner.a, 1);

    {
        auto var = std::make_shared<SimpleObsTargetTestClass>();
        ptr->set(var, std::bind(&ObsPtrOwner::handle_target_deletion, &obsPtrOwner));

        ASSERT_TRUE(ptr->is_set());
        EXPECT_TRUE(var->IsObserver(ptr));
        EXPECT_EQ(var->Observers(), 1);

        var.reset();
    }

    ASSERT_EQ(obsPtrOwner.a, 2);

    {
        auto var = std::make_shared<SimpleObsTargetTestClass>();

        ptr->set(var, nullptr); // Need to explicitly deregister the callback

        ASSERT_TRUE(ptr->is_set());
        EXPECT_TRUE(var->IsObserver(ptr));
        EXPECT_EQ(var->Observers(), 1);

        var.reset();
    }

    ASSERT_EQ(obsPtrOwner.a, 2);

    auto var1 = std::make_shared<SimpleObsTargetTestClass>();

    {
        auto var2 = std::make_shared<SimpleObsTargetTestClass>();

        ptr->set(var1, std::bind(&ObsPtrOwner::handle_target_deletion, &obsPtrOwner));
        ptr->set(var2, std::bind(&ObsPtrOwner::handle_target_deletion, &obsPtrOwner));
    }
    var1.reset();

    ASSERT_EQ(obsPtrOwner.a, 3);
}

struct Observed1 : public IObserved
{
    int a = 0;
};

struct Observed2 : public IObserved
{
    float a = 1.0;
};
using obs_ptr_Observed1SPtr = std::shared_ptr<obs_ptr<Observed1>>;
using obs_ptr_Observed2SPtr = std::shared_ptr<obs_ptr<Observed2>>;
using obs_variant = std::variant<std::monostate, obs_ptr_Observed1SPtr, obs_ptr_Observed2SPtr>;
struct ObsPtrVariantOwner
{
    int a = 1;
    obs_variant obsVariant;
    void handle_target_deletion()
    {
        obsVariant = std::monostate();
    }

    template <typename T>
    void SetTarget(T pTarget)
    {
        obsVariant = make_observer(pTarget, std::bind(&ObsPtrVariantOwner::handle_target_deletion, this));
    }
};

TEST(CallbackObsTest, VariantAssignment)
{
    auto pOwner = std::make_shared<ObsPtrVariantOwner>();

    EXPECT_EQ(pOwner->obsVariant.index(), 0);

    {
        auto observed1 = std::make_shared<Observed1>();

        pOwner->SetTarget(observed1);

        EXPECT_EQ(pOwner->obsVariant.index(), 1);
        auto ptr = std::get<obs_ptr_Observed1SPtr>(pOwner->obsVariant);
        EXPECT_TRUE(ptr->is_set());
        EXPECT_TRUE(observed1->IsObserver(ptr));
        EXPECT_EQ(observed1->Observers(), 1);
    }

    EXPECT_EQ(pOwner->obsVariant.index(), 0);

    {
        auto observed2 = std::make_shared<Observed2>();

        pOwner->SetTarget(observed2);

        EXPECT_EQ(pOwner->obsVariant.index(), 2);
    }

    EXPECT_EQ(pOwner->obsVariant.index(), 0);

    {
        auto observed2 = std::make_shared<Observed2>();

        pOwner->SetTarget(observed2);

        pOwner.reset();
    }
}

TEST(CerealTest, CallbackReconstruction)
{
    auto obsPtrOwner = ObsPtrOwner();
    std::stringstream ss;

    ASSERT_TRUE(ss.good());

    {
        cereal::BinaryOutputArchive archive{ss};

        auto var = std::make_shared<SimpleObsTargetTestClass>();
        obsPtrOwner.pObserver->set(var);

        EXPECT_NO_THROW(archive(var));
        EXPECT_NO_THROW(archive(obsPtrOwner));

        var.reset();
    }

    EXPECT_EQ(obsPtrOwner.a, 2);

    {
        cereal::BinaryInputArchive archive{ss};

        auto var = std::make_shared<SimpleObsTargetTestClass>();

        ASSERT_NO_THROW(archive(var));
        ASSERT_NO_THROW(archive(obsPtrOwner));

        EXPECT_TRUE(obsPtrOwner.pObserver->is_set());
        EXPECT_EQ(obsPtrOwner.pObserver, var);
        EXPECT_EQ(var->Observers(), 1);
        EXPECT_TRUE(var->IsObserver(obsPtrOwner.pObserver));

        var.reset();
    }

    EXPECT_EQ(obsPtrOwner.a, 3);
}
