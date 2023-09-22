/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"

#include "RefLogger.h"
#include "Utilities.h"
#include <wtf/MainThread.h>
#include <wtf/Ref.h>
#include <wtf/RefPtr.h>
#include <wtf/RefTracker.h>
#include <wtf/RunLoop.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/Threading.h>

namespace TestWebKitAPI {


TEST(WTF_RefTrackedRefPtr, Basic)
{
    RefTrackedRefLogger a("a");
    {
        RefPtr<RefTrackedRefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_EQ(&a.name, &ptr->name);
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
        EXPECT_TRUE(RefTracker::sharedTracker().hasRemainingReferences());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<RefTrackedRefLogger> ptr(adoptRef(&a));
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_EQ(&a.name, &ptr->name);
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
    }
    EXPECT_STREQ("deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRef, Basic)
{
    RefTrackedRefLogger a("a");
    {
        Ref<RefTrackedRefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_EQ(&a.name, &ref->name);
        EXPECT_NE(ref.refTrackingToken(), UntrackedRefToken());
        EXPECT_TRUE(RefTracker::sharedTracker().hasRemainingReferences());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<RefTrackedRefLogger> ref(adoptRef(a));
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_EQ(&a.name, &ref->name);
        EXPECT_NE(ref.refTrackingToken(), UntrackedRefToken());
    }
    EXPECT_STREQ("deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
}

TEST(WTF_RefTrackedRefPtr, Assignment)
{
    DerivedRefTrackedRefLogger a("a");
    RefTrackedRefLogger b("b");
    DerivedRefTrackedRefLogger c("c");
    
    {
        RefPtr<RefTrackedRefLogger> ptr(&a);
        auto aToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ptr = &b;
        auto bToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&b, ptr.ptr());
        EXPECT_NE(bToken, UntrackedRefToken());
        EXPECT_NE(aToken, bToken);
        EXPECT_TRUE(RefTracker::sharedTracker().hasRemainingReferences());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(b) deref(a) | deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<RefTrackedRefLogger> ptr(&a);
        auto aToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ptr = &c;
        auto cToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&c, ptr.ptr());
        EXPECT_NE(cToken, UntrackedRefToken());
        EXPECT_NE(aToken, cToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(c) deref(a) | deref(c) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<RefTrackedRefLogger> ptr(&a);
        auto aToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ptr = adoptRef(b);
        auto bToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&b, ptr.ptr());
        EXPECT_NE(bToken, UntrackedRefToken());
        EXPECT_NE(aToken, bToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<RefTrackedRefLogger> ptr(&a);
        auto aToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ptr = adoptRef(c);
        auto cToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&c, ptr.ptr());
        EXPECT_NE(cToken, UntrackedRefToken());
        EXPECT_NE(aToken, cToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(c) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRef, Assignment)
{
    DerivedRefTrackedRefLogger a("a");
    RefTrackedRefLogger b("b");
    DerivedRefTrackedRefLogger c("c");
    
    {
        Ref<RefTrackedRefLogger> ref(a);
        auto aToken = ref.refTrackingToken();
        
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ref = b;
        auto bToken = ref.refTrackingToken();
        
        EXPECT_EQ(&b, ref.ptr());
        EXPECT_NE(bToken, UntrackedRefToken());
        EXPECT_NE(aToken, bToken);
        EXPECT_TRUE(RefTracker::sharedTracker().hasRemainingReferences());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(b) deref(a) | deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<RefTrackedRefLogger> ref(a);
        auto aToken = ref.refTrackingToken();
        
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ref = c;
        auto cToken = ref.refTrackingToken();
        
        EXPECT_EQ(&c, ref.ptr());
        EXPECT_NE(cToken, UntrackedRefToken());
        EXPECT_NE(aToken, cToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(c) deref(a) | deref(c) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<RefTrackedRefLogger> ref(a);
        auto aToken = ref.refTrackingToken();
        
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ref = adoptRef(b);
        auto bToken = ref.refTrackingToken();
        
        EXPECT_EQ(&b, ref.ptr());
        EXPECT_NE(bToken, UntrackedRefToken());
        EXPECT_NE(aToken, bToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<RefTrackedRefLogger> ref(a);
        auto aToken = ref.refTrackingToken();
        
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ref = adoptRef(c);
        auto cToken = ref.refTrackingToken();
        
        EXPECT_EQ(&c, ref.ptr());
        EXPECT_NE(cToken, UntrackedRefToken());
        EXPECT_NE(aToken, cToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(c) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
}

namespace {
static RefPtr<RefTrackedRefLogger> passWithRefPtr(RefPtr<RefTrackedRefLogger>&& pointer) { return WTFMove(pointer); }
static Ref<RefTrackedRefLogger> passWithRef(Ref<RefTrackedRefLogger>&& reference) { return WTFMove(reference); }
}

TEST(WTF_RefTrackedRefPtr, ReturnValue)
{
    DerivedRefTrackedRefLogger a("a");
    RefTrackedRefLogger b("b");
    DerivedRefTrackedRefLogger c("c");
    
    {
        RefPtr<RefTrackedRefLogger> ptr(passWithRefPtr(&a));
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<RefTrackedRefLogger> ptr(&a);
        auto aToken = ptr.refTrackingToken();
        
        EXPECT_EQ(&a, ptr.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ptr = passWithRefPtr(&b);
        EXPECT_EQ(&b, ptr.ptr());
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
        EXPECT_NE(ptr.refTrackingToken(), aToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(b) deref(a) | deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<RefTrackedRefLogger> ptr(passWithRefPtr(&a));
        EXPECT_EQ(&a, ptr.get());
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<DerivedRefTrackedRefLogger> ptr(&a);
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
        
        RefPtr<RefTrackedRefLogger> ptr2(WTFMove(ptr));
        
        EXPECT_EQ(nullptr, ptr.get());
        EXPECT_EQ(ptr.refTrackingToken(), UntrackedRefToken());
        EXPECT_EQ(&a, ptr2.get());
        EXPECT_NE(ptr2.refTrackingToken(), UntrackedRefToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRef, ReturnValue)
{
    DerivedRefTrackedRefLogger a("a");
    RefTrackedRefLogger b("b");
    DerivedRefTrackedRefLogger c("c");
    
    {
        Ref<RefTrackedRefLogger> ref(passWithRef(a));
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_NE(ref.refTrackingToken(), UntrackedRefToken());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<RefTrackedRefLogger> ref(a);
        auto aToken = ref.refTrackingToken();
        
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_NE(aToken, UntrackedRefToken());
        log() << "| ";
        
        ref = passWithRef(b);
        EXPECT_EQ(&b, ref.ptr());
        EXPECT_NE(ref.refTrackingToken(), UntrackedRefToken());
        EXPECT_NE(ref.refTrackingToken(), aToken);
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(b) deref(a) | deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<RefTrackedRefLogger> ref(passWithRef(a));
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_NE(ref.refTrackingToken(), UntrackedRefToken());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<DerivedRefTrackedRefLogger> ref(a);
        EXPECT_NE(ref.refTrackingToken(), UntrackedRefToken());
        
        Ref<RefTrackedRefLogger> ref2(WTFMove(ref));
        EXPECT_EQ(&a, ref2.ptr());
        EXPECT_NE(ref2.refTrackingToken(), UntrackedRefToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRefPtr, Swap)
{
    RefTrackedRefLogger a("a");
    RefTrackedRefLogger b("b");
    
    {
        RefPtr<RefTrackedRefLogger> p1(&a);
        RefPtr<RefTrackedRefLogger> p2(&b);
        
        RefTrackingToken p1Token = p1.refTrackingToken();
        RefTrackingToken p2Token = p2.refTrackingToken();
        EXPECT_NE(p1Token, UntrackedRefToken());
        EXPECT_NE(p2Token, UntrackedRefToken());
        
        log() << "| ";
        EXPECT_EQ(&a, p1.ptr());
        EXPECT_EQ(&b, p2.ptr());
        p1.swap(p2);
        EXPECT_EQ(&b, p1.ptr());
        EXPECT_EQ(&a, p2.ptr());
        EXPECT_EQ(p1Token, p2.refTrackingToken());
        EXPECT_EQ(p2Token, p1.refTrackingToken());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        RefPtr<RefTrackedRefLogger> p1(&a);
        RefPtr<RefTrackedRefLogger> p2(&b);
        
        RefTrackingToken p1Token = p1.refTrackingToken();
        RefTrackingToken p2Token = p2.refTrackingToken();
        EXPECT_NE(p1Token, UntrackedRefToken());
        EXPECT_NE(p2Token, UntrackedRefToken());
        
        log() << "| ";
        EXPECT_EQ(&a, p1.ptr());
        EXPECT_EQ(&b, p2.ptr());
        std::swap(p1, p2);
        EXPECT_EQ(&b, p1.ptr());
        EXPECT_EQ(&a, p2.ptr());
        EXPECT_EQ(p1Token, p2.refTrackingToken());
        EXPECT_EQ(p2Token, p1.refTrackingToken());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
}

TEST(WTF_RefTrackedRef, Swap)
{
    RefTrackedRefLogger a("a");
    RefTrackedRefLogger b("b");
    
    {
        Ref<RefTrackedRefLogger> r1(a);
        Ref<RefTrackedRefLogger> r2(b);
        
        RefTrackingToken r1Token = r1.refTrackingToken();
        RefTrackingToken r2Token = r2.refTrackingToken();
        EXPECT_NE(r1Token, UntrackedRefToken());
        EXPECT_NE(r2Token, UntrackedRefToken());
        
        log() << "| ";
        EXPECT_EQ(&a, r1.ptr());
        EXPECT_EQ(&b, r2.ptr());
        r1.swap(r2);
        EXPECT_EQ(&b, r1.ptr());
        EXPECT_EQ(&a, r2.ptr());
        EXPECT_EQ(r1Token, r2.refTrackingToken());
        EXPECT_EQ(r2Token, r1.refTrackingToken());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
    {
        Ref<RefTrackedRefLogger> r1(a);
        Ref<RefTrackedRefLogger> r2(b);
        
        RefTrackingToken r1Token = r1.refTrackingToken();
        RefTrackingToken r2Token = r2.refTrackingToken();
        EXPECT_NE(r1Token, UntrackedRefToken());
        EXPECT_NE(r2Token, UntrackedRefToken());
        
        log() << "| ";
        EXPECT_EQ(&a, r1.ptr());
        EXPECT_EQ(&b, r2.ptr());
        std::swap(r1, r2);
        EXPECT_EQ(&b, r1.ptr());
        EXPECT_EQ(&a, r2.ptr());
        EXPECT_EQ(r1Token, r2.refTrackingToken());
        EXPECT_EQ(r2Token, r1.refTrackingToken());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
    
}

TEST(WTF_RefTrackedRefPtr, StaticPointerCastFromConstPointer)
{
    {
        DerivedRefTrackedRefLogger a("a");
        const RefPtr<DerivedRefTrackedRefLogger> ptr(&a);
        auto ptr2 = static_pointer_cast<RefTrackedRefLogger>(ptr);
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
        EXPECT_NE(ptr.refTrackingToken(), ptr2.refTrackingToken());
    }
    EXPECT_STREQ("ref(a) ref(a) deref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRef, StaticReferenceCastFromConstReference)
{
    {
        DerivedRefTrackedRefLogger a("a");
        const Ref<DerivedRefTrackedRefLogger> ref(a);
        auto ref2 = static_reference_cast<RefTrackedRefLogger>(ref);
        EXPECT_NE(ref.refTrackingToken(), UntrackedRefToken());
        EXPECT_NE(ref.refTrackingToken(), ref2.refTrackingToken());
    }
    EXPECT_STREQ("ref(a) ref(a) deref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRefPtr, StaticPointerCastFromRValueReference)
{
    {
        DerivedRefTrackedRefLogger a("a");
        RefPtr<DerivedRefTrackedRefLogger> ptr(&a);
        auto token = ptr.refTrackingToken();
        auto ptr2 = static_pointer_cast<RefTrackedRefLogger>(WTFMove(ptr));
        EXPECT_NE(token, UntrackedRefToken());
        EXPECT_EQ(token, ptr2.refTrackingToken());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRef, StaticReferenceCastFromRValueReference)
{
    {
        DerivedRefTrackedRefLogger a("a");
        Ref<DerivedRefTrackedRefLogger> ref(a);
        auto token = ref.refTrackingToken();
        auto ref2 = static_reference_cast<RefTrackedRefLogger>(WTFMove(ref));
        EXPECT_NE(token, UntrackedRefToken());
        EXPECT_EQ(token, ref2.refTrackingToken());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRefPtr, ReleaseNonNull)
{
    RefTrackedRefLogger a("a");

    {
        RefPtr<RefTrackedRefLogger> refPtr = &a;
        auto token = refPtr.refTrackingToken();
        EXPECT_NE(token, UntrackedRefToken());

        Ref<RefTrackedRefLogger> ref = refPtr.releaseNonNull();
        EXPECT_EQ(token, ref.refTrackingToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());

    {
        RefPtr<RefTrackedRefLogger> refPtr = &a;
        auto token = refPtr.refTrackingToken();
        EXPECT_NE(token, UntrackedRefToken());

        Ref<const RefTrackedRefLogger> ref = refPtr.releaseNonNull();
        EXPECT_EQ(token, ref.refTrackingToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}


// FIXME: Enable these tests once Win platform supports TestWebKitAPI::Util::run
#if! PLATFORM(WIN)

struct RefTrackedThreadSafeRefCountedObject;
struct RefTrackedMainThreadSafeRefCountedObject;

} // namespace TestWebKitAPI
namespace WTF {
template<> struct RefDerefTraits<TestWebKitAPI::RefTrackedThreadSafeRefCountedObject> : public RefTrackingTraits { };
template<> struct RefDerefTraits<TestWebKitAPI::RefTrackedMainThreadSafeRefCountedObject> : public RefTrackingTraits { };
}

namespace TestWebKitAPI {

static bool done;
static bool isDestroyedInMainThread;
struct RefTrackedThreadSafeRefCountedObject : ThreadSafeRefCounted<RefTrackedThreadSafeRefCountedObject> {
    static Ref<RefTrackedThreadSafeRefCountedObject> create() { return adoptRef(*new RefTrackedThreadSafeRefCountedObject); }
    static RefPtr<RefTrackedThreadSafeRefCountedObject> createPtr() { return adoptRef(new RefTrackedThreadSafeRefCountedObject); }
    
    ~RefTrackedThreadSafeRefCountedObject()
    {
        isDestroyedInMainThread = isMainThread();
        done = true;
    }

    RefTrackingToken trackRef() const { return RefTracker::sharedTracker().trackRef(); }
    void trackDeref(RefTrackingToken token) const { return RefTracker::sharedTracker().trackDeref(token); }
};

struct RefTrackedMainThreadSafeRefCountedObject : ThreadSafeRefCounted<RefTrackedMainThreadSafeRefCountedObject, WTF::DestructionThread::Main> {
    static Ref<RefTrackedMainThreadSafeRefCountedObject> create() { return adoptRef(*new RefTrackedMainThreadSafeRefCountedObject); }
    static RefPtr<RefTrackedMainThreadSafeRefCountedObject> createPtr() { return adoptRef(new RefTrackedMainThreadSafeRefCountedObject); }
    
    ~RefTrackedMainThreadSafeRefCountedObject()
    {
        isDestroyedInMainThread = isMainThread();
        done = true;
    }

    RefTrackingToken trackRef() const { return RefTracker::sharedTracker().trackRef(); }
    void trackDeref(RefTrackingToken token) const { return RefTracker::sharedTracker().trackDeref(token); }
};

TEST(WTF_RefTrackedRefPtr, ReleaseInNonMainThread)
{
    done = false;
    Thread::create(""_s, [ptr = RefTrackedThreadSafeRefCountedObject::createPtr()] {
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    });
    TestWebKitAPI::Util::run(&done);

    EXPECT_FALSE(isDestroyedInMainThread);
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}
TEST(WTF_RefTrackedRef, ReleaseInNonMainThread)
{
    done = false;
    Thread::create(""_s, [object = RefTrackedThreadSafeRefCountedObject::create()] {
        EXPECT_NE(object.refTrackingToken(), UntrackedRefToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    });
    TestWebKitAPI::Util::run(&done);

    EXPECT_FALSE(isDestroyedInMainThread);
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRefPtr, ReleaseInNonMainThreadDestroyInMainThread)
{
    WTF::initializeMainThread();
    done = false;
    Thread::create(""_s, [ptr = RefTrackedMainThreadSafeRefCountedObject::createPtr()] {
        EXPECT_NE(ptr.refTrackingToken(), UntrackedRefToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    });
    TestWebKitAPI::Util::run(&done);

    EXPECT_TRUE(isDestroyedInMainThread);
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

TEST(WTF_RefTrackedRef, ReleaseInNonMainThreadDestroyInMainThread)
{
    WTF::initializeMainThread();
    done = false;
    Thread::create(""_s, [object = RefTrackedMainThreadSafeRefCountedObject::create()] {
        EXPECT_NE(object.refTrackingToken(), UntrackedRefToken());
        EXPECT_EQ(RefTracker::sharedTracker().trackedReferencesCount(), 1u);
    });
    TestWebKitAPI::Util::run(&done);

    EXPECT_TRUE(isDestroyedInMainThread);
    EXPECT_FALSE(RefTracker::sharedTracker().hasRemainingReferences());
}

#endif // ! PLATFORM(WIN)

} // namespace TestWebKitAPI
