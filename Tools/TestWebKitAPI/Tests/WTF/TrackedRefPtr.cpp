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
#if 0
#include "config.h"

#if ENABLE(REF_TRACKING)

#include "RefLogger.h"
#include "Utilities.h"
#include <wtf/MainThread.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/RefDerefTraits.h>
#include <wtf/RefTracker.h>
#include <wtf/RefTracking.h>
#include <wtf/RunLoop.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/Threading.h>

namespace TestWebKitAPI {

struct RefTrackingBase : private WTF::RefTrackable<RefTrackingBase> {
    using RefTrackable<RefTrackingBase>::RefTrackingId;
};
} // namespace TestWebKitAPI

namespace WTF {
template<>
struct RefDerefTraits<TestWebKitAPI::RefTrackingBase::RefTrackingId> {
    template <typename T>
    ALWAYS_INLINE void ref(const T& object)
    {
        WTFLogAlways("RefDerefTraits(RefTracking) ref.");
        m_refTrackingToken = RefTracker::sharedTracker().trackRef();
        object.ref();
    }

    template <typename T>
    ALWAYS_INLINE void refIfNotNull(const T* ptr)
    {
        WTFLogAlways("RefDerefTraits(RefTracking) refIfNotNull. %" PRIuPTR " (ptr)", (uintptr_t)ptr);
        if (UNLIKELY(ptr == nullptr))
            m_refTrackingToken = UntrackedRefToken();

        m_refTrackingToken = RefTracker::sharedTracker().trackRef();
        ptr->ref();
    }

    template <typename T>
    ALWAYS_INLINE void derefIfNotNull(const T* ptr)
    {
        WTFLogAlways("RefDerefTraits(RefTracking) derefIfNotNull.");
        if (UNLIKELY(ptr == nullptr))
            return;
        RefTracker::sharedTracker().trackDeref(std::exchange(m_refTrackingToken, UntrackedRefToken()));
        ptr->deref();
    }

    template <typename T>
    ALWAYS_INLINE void adoptRef(const T* ptr)
    {
        WTFLogAlways("RefDerefTraits(RefTracking): adoptRef.");
        if (UNLIKELY(ptr == nullptr)) {
            m_refTrackingToken = UntrackedRefToken();
            return;
        }

        m_refTrackingToken =  RefTracker::sharedTracker().trackRef();
    }

    ALWAYS_INLINE void takeRef(WTF::RefTrackingSmartPtr auto& smartPtr)
    {
        WTFLogAlways("RefDerefTraits(RefTracking): takeRef (RefTrackingSmartPtr)");
        m_refTrackingToken = smartPtr.refTrackingToken();
        smartPtr.setRefTrackingToken(UntrackedRefToken());
    }

    ALWAYS_INLINE void takeRef(const WTF::SmartPtr auto& smartPtr)
    {
        WTFLogAlways("RefDerefTraits(RefTracking): takeRef (SmartPtr)");
        adoptRef(smartPtr.ptr());
    }

    ALWAYS_INLINE void swapRef(WTF::RefTrackingSmartPtr auto& smartPtr)
    {
        WTFLogAlways("RefDerefTraits(Node RefTracking): swap (RefTrackingSmartPtr)");
        RefTrackingToken tmp = m_refTrackingToken;
        m_refTrackingToken = smartPtr.refTrackingToken();
        smartPtr.setRefTrackingToken(tmp);
    }

    ALWAYS_INLINE void swapRef(WTF::SmartPtr auto&)
    {
        WTFLogAlways("RefDerefTraits(RefTracking): swap (SmartPtr)");
        m_refTrackingToken = UntrackedRefToken();
    }

    RefTrackingToken refTrackingToken() const { return m_refTrackingToken; }
    void setRefTrackingToken(RefTrackingToken token) { m_refTrackingToken = token; }

private:
    RefTrackingToken m_refTrackingToken;
};
} // namespace WTF

namespace TestWebKitAPI {
namespace {
struct TrackedRefLogger : public RefLogger, public RefTrackingBase {
    TrackedRefLogger(const char *name) : RefLogger { name } { }
};
} // anonymous namespace


TEST(WTF_TrackedRefPtr, Basic)
{
    WTFLogAlways("-------- Basic --------");
    TrackedRefLogger a("a");

    RefPtr<TrackedRefLogger> empty;
    EXPECT_EQ(nullptr, empty.get());

    {
        RefPtr<TrackedRefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        EXPECT_EQ(&a, &*ptr);
        EXPECT_EQ(&a.name, &ptr->name);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> ptr = &a;
        EXPECT_EQ(&a, ptr.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> p1 = &a;
        RefPtr<TrackedRefLogger> p2(p1);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) ref(a) deref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> p1 = &a;
        RefPtr<TrackedRefLogger> p2 = p1;
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) ref(a) deref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> p1 = &a;
        RefPtr<TrackedRefLogger> p2 = WTFMove(p1);
        EXPECT_EQ(nullptr, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> p1 = &a;
        RefPtr<TrackedRefLogger> p2(WTFMove(p1));
        EXPECT_EQ(nullptr, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> p1 = &a;
        RefPtr<TrackedRefLogger> p2 = p1;
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) ref(a) deref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> p1 = &a;
        RefPtr<TrackedRefLogger> p2 = WTFMove(p1);
        EXPECT_EQ(nullptr, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        ptr = nullptr;
        EXPECT_EQ(nullptr, ptr.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRefPtr, AssignPassRefToRefPtr)
{
    TrackedRefLogger a("a");
    {
        Ref<RefLogger> passRef(a);
        RefPtr<RefLogger> ptr = WTFMove(passRef);
        EXPECT_EQ(&a, ptr.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRefPtr, Adopt)
{
    TrackedRefLogger a("a");

    RefPtr<RefLogger> empty;
    EXPECT_EQ(nullptr, empty.get());

    {
        RefPtr<RefLogger> ptr(adoptRef(&a));
        EXPECT_EQ(&a, ptr.get());
        EXPECT_EQ(&a, &*ptr);
        EXPECT_EQ(&a.name, &ptr->name);
    }
    EXPECT_STREQ("deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> ptr = adoptRef(&a);
        EXPECT_EQ(&a, ptr.get());
    }
    EXPECT_STREQ("deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRefPtr, Assignment)
{
    TrackedRefLogger a("a");
    RefLogger b("b");
    TrackedRefLogger c("c");
    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<RefLogger> p2(&b);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&b, p2.get());
        log() << "| ";
        p1 = p2;
        EXPECT_EQ(&b, p1.get());
        EXPECT_EQ(&b, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | ref(b) deref(a) | deref(b) deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        log() << "| ";
        ptr = &b;
        EXPECT_EQ(&b, ptr.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(b) deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
    {
        RefPtr<RefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        log() << "| ";
        ptr = adoptRef(&b);
        EXPECT_EQ(&b, ptr.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
    {
        RefPtr<RefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        ptr = nullptr;
        EXPECT_EQ(nullptr, ptr.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<RefLogger> p2(&b);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&b, p2.get());
        log() << "| ";
        p1 = WTFMove(p2);
        EXPECT_EQ(&b, p1.get());
        EXPECT_EQ(nullptr, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<TrackedRefLogger> p2(&c);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&c, p2.get());
        log() << "| ";
        p1 = p2;
        EXPECT_EQ(&c, p1.get());
        EXPECT_EQ(&c, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(c) | ref(c) deref(a) | deref(c) deref(c) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        log() << "| ";
        ptr = &c;
        EXPECT_EQ(&c, ptr.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(c) deref(a) | deref(c) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        log() << "| ";
        ptr = adoptRef(&c);
        EXPECT_EQ(&c, ptr.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(c) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<TrackedRefLogger> p2(&c);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&c, p2.get());
        log() << "| ";
        p1 = WTFMove(p2);
        EXPECT_EQ(&c, p1.get());
        EXPECT_EQ(nullptr, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(c) | deref(a) | deref(c) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        log() << "| ";
#if COMPILER(CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        ptr = ptr;
#if COMPILER(CLANG)
#pragma clang diagnostic pop
#endif
        EXPECT_EQ(&a, ptr.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(a) deref(a) | deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        IGNORE_WARNINGS_BEGIN("self-move")
        ptr = WTFMove(ptr);
        IGNORE_WARNINGS_END
        EXPECT_EQ(&a, ptr.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRefPtr, Swap)
{
    RefLogger a("a");
    RefLogger b("b");

    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<RefLogger> p2(&b);
        log() << "| ";
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&b, p2.get());
        p1.swap(p2);
        EXPECT_EQ(&b, p1.get());
        EXPECT_EQ(&a, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<RefLogger> p2(&b);
        log() << "| ";
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&b, p2.get());
        std::swap(p1, p2);
        EXPECT_EQ(&b, p1.get());
        EXPECT_EQ(&a, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRefPtr, ReleaseNonNull)
{
    RefLogger a("a");

    {
        RefPtr<RefLogger> refPtr = &a;
        RefPtr<RefLogger> ref = refPtr.releaseNonNull();
    }

    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRefPtr, Release)
{
    TrackedRefLogger a("a");
    RefLogger b("b");
    TrackedRefLogger c("c");

    {
        RefPtr<RefLogger> p1 = &a;
        RefPtr<RefLogger> p2 = WTFMove(p1);
        EXPECT_EQ(nullptr, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> p1 = &a;
        RefPtr<RefLogger> p2(WTFMove(p1));
        EXPECT_EQ(nullptr, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefLogger> p1 = &a;
        RefPtr<RefLogger> p2 = WTFMove(p1);
        EXPECT_EQ(nullptr, p1.get());
        EXPECT_EQ(&a, p2.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<RefLogger> p2(&b);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&b, p2.get());
        log() << "| ";
        p1 = WTFMove(p2);
        EXPECT_EQ(&b, p1.get());
        EXPECT_EQ(nullptr, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<RefLogger> p1(&a);
        RefPtr<TrackedRefLogger> p2(&c);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&c, p2.get());
        log() << "| ";
        p1 = WTFMove(p2);
        EXPECT_EQ(&c, p1.get());
        EXPECT_EQ(nullptr, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(c) | deref(a) | deref(c) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

namespace {
static RefPtr<RefLogger> f1(RefLogger& logger)
{
    return RefPtr<RefLogger>(&logger);
}
}

TEST(WTF_TrackedRefPtr, ReturnValue)
{
    TrackedRefLogger a("a");

    {
        f1(a);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        auto ptr = f1(a);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

namespace {
struct ConstRefCounted : RefCounted<ConstRefCounted>, public RefTrackingBase {
    static Ref<ConstRefCounted> create() { WTFLogAlways("-------- ConstRefCounted::create (Tracked) --------"); return adoptRef(*new ConstRefCounted); }
};

static const ConstRefCounted& returnConstRefCountedRef()
{
    static NeverDestroyed<Ref<ConstRefCounted>> instance { ConstRefCounted::create() };
    return instance.get().get();
}
static ConstRefCounted& returnRefCountedRef()
{
    static NeverDestroyed<Ref<ConstRefCounted>> instance { ConstRefCounted::create() };
    return instance.get().get();
}
} // anonymous namespace

TEST(WTF_TrackedRefPtr, Const)
{
    // This test passes if it compiles without an error.
    {
    auto a = ConstRefCounted::create();
    Ref<const ConstRefCounted> b = WTFMove(a);
    RefPtr<const ConstRefCounted> c = b.ptr();
    Ref<const ConstRefCounted> d = returnConstRefCountedRef();
    RefPtr<const ConstRefCounted> e = &returnConstRefCountedRef();
    RefPtr<ConstRefCounted> f = ConstRefCounted::create();
    RefPtr<const ConstRefCounted> g = f;
    RefPtr<const ConstRefCounted> h(f);
    Ref<const ConstRefCounted> i(returnRefCountedRef());
    }
    RefTracker::sharedTracker().showRemainingReferences();
}
namespace {
struct TrackedRefPtrCheckingRefLogger : RefLogger {
    TrackedRefPtrCheckingRefLogger(const char* name);
    void ref() const;
    void deref() const;
    const RefPtr<TrackedRefPtrCheckingRefLogger>* slotToCheck { nullptr };
};

TrackedRefPtrCheckingRefLogger::TrackedRefPtrCheckingRefLogger(const char* name)
: RefLogger { name }
{
}

static const char* loggerName(const RefPtr<TrackedRefPtrCheckingRefLogger>& pointer)
{
    return pointer ? &pointer->name : "null";
}

void TrackedRefPtrCheckingRefLogger::ref() const
{
    if (slotToCheck)
        log() << "slot=" << loggerName(*slotToCheck) << " ";
    RefLogger::ref();
}

void TrackedRefPtrCheckingRefLogger::deref() const
{
    if (slotToCheck)
        log() << "slot=" << loggerName(*slotToCheck) << " ";
    RefLogger::deref();
}
} // anonymous namespace
TEST(WTF_TrackedRefPtr, AssignBeforeDeref)
{
    TrackedRefPtrCheckingRefLogger a("a");
    TrackedRefPtrCheckingRefLogger b("b");

    {
        RefPtr<TrackedRefPtrCheckingRefLogger> p1(&a);
        RefPtr<TrackedRefPtrCheckingRefLogger> p2(&b);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&b, p2.get());
        log() << "| ";
        a.slotToCheck = &p1;
        b.slotToCheck = &p1;
        p1 = p2;
        a.slotToCheck = nullptr;
        b.slotToCheck = nullptr;
        EXPECT_EQ(&b, p1.get());
        EXPECT_EQ(&b, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | slot=a ref(b) slot=b deref(a) | deref(b) deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefPtrCheckingRefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        log() << "| ";
        a.slotToCheck = &ptr;
        b.slotToCheck = &ptr;
        ptr = &b;
        a.slotToCheck = nullptr;
        b.slotToCheck = nullptr;
        EXPECT_EQ(&b, ptr.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | slot=a ref(b) slot=b deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefPtrCheckingRefLogger> ptr(&a);
        EXPECT_EQ(&a, ptr.get());
        a.slotToCheck = &ptr;
        ptr = nullptr;
        a.slotToCheck = nullptr;
        EXPECT_EQ(nullptr, ptr.get());
    }
    EXPECT_STREQ("ref(a) slot=null deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        RefPtr<TrackedRefPtrCheckingRefLogger> p1(&a);
        RefPtr<TrackedRefPtrCheckingRefLogger> p2(&b);
        EXPECT_EQ(&a, p1.get());
        EXPECT_EQ(&b, p2.get());
        log() << "| ";
        a.slotToCheck = &p1;
        b.slotToCheck = &p1;
        p1 = WTFMove(p2);
        a.slotToCheck = nullptr;
        b.slotToCheck = nullptr;
        EXPECT_EQ(&b, p1.get());
        EXPECT_EQ(nullptr, p2.get());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | slot=b deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRefPtr, ReleaseNonNullBeforeDeref)
{
    TrackedRefPtrCheckingRefLogger a("a");

    {
        RefPtr<TrackedRefPtrCheckingRefLogger> refPtr = &a;
        a.slotToCheck = &refPtr;
        refPtr.releaseNonNull();
        a.slotToCheck = nullptr;
    }

    EXPECT_STREQ("ref(a) slot=null deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

// FIXME: Enable these tests once Win platform supports TestWebKitAPI::Util::run
#if! PLATFORM(WIN)
namespace {
static bool done;
static bool isDestroyedInMainThread;
struct ThreadSafeRefCountedObject : ThreadSafeRefCounted<ThreadSafeRefCountedObject> {
    static Ref<ThreadSafeRefCountedObject> create() { return adoptRef(*new ThreadSafeRefCountedObject); }
    
    ~ThreadSafeRefCountedObject()
    {
        isDestroyedInMainThread = isMainThread();
        done = true;
    }
};

struct MainThreadSafeRefCountedObject : ThreadSafeRefCounted<MainThreadSafeRefCountedObject, WTF::DestructionThread::Main> {
    static Ref<MainThreadSafeRefCountedObject> create() { return adoptRef(*new MainThreadSafeRefCountedObject); }
    
    ~MainThreadSafeRefCountedObject()
    {
        isDestroyedInMainThread = isMainThread();
        done = true;
    }
};
} // anonymous namespace
TEST(WTF_TrackedRefPtr, ReleaseInNonMainThread)
{
    done = false;
    Thread::create("", [object = ThreadSafeRefCountedObject::create()] { });
    TestWebKitAPI::Util::run(&done);

    EXPECT_FALSE(isDestroyedInMainThread);
}

TEST(WTF_TrackedRefPtr, ReleaseInNonMainThreadDestroyInMainThread)
{
    WTF::initializeMainThread();
    done = false;
    Thread::create("", [object = MainThreadSafeRefCountedObject::create()] { });
    TestWebKitAPI::Util::run(&done);

    EXPECT_TRUE(isDestroyedInMainThread);
}

#endif // !PLATFORM(WIN)

#endif // ENABLE(REF_TRACKING)

} // namespace TestWebKitAPI
#endif // if 0
