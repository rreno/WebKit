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
#include <wtf/Ref.h>
#include <wtf/RefPtr.h>
#include <wtf/RefDerefTraits.h>
#include <wtf/RefTracking.h>
#include <wtf/RefTracker.h>

namespace TestWebKitAPI {
struct TrackedRefCheckingRefLogger;
struct DerivedTrackedRefCheckingRefLogger;
}

namespace WTF {

struct RefLoggerRefTrackingTraits {
    static constexpr bool isRefTracked = true;
    void ref(const auto& obj)
    {
        WTFLogAlways("RefDerefTraits(RefLogger) ref.");
        m_refTrackingToken = RefTracker::sharedTracker().trackRef();
        obj.ref();
    }
    void refIfNotNull(const auto* ptr)
    {
        WTFLogAlways("RefDerefTraits(RefLogger) refIfNotNull. %" PRIuPTR " (ptr)", (uintptr_t)ptr);
        if (LIKELY(ptr != nullptr)) {
            m_refTrackingToken = RefTracker::sharedTracker().trackRef();
            ptr->ref();
            return;
        }

        m_refTrackingToken = UntrackedRefToken();
    }
    void derefIfNotNull(const auto* ptr)
    {
        WTFLogAlways("RefDerefTraits(RefLogger) derefIfNotNull. %" PRIuPTR " (ptr)", (uintptr_t)ptr);
        if (LIKELY(ptr != nullptr)) {
            RefTracker::sharedTracker().trackDeref(std::exchange(m_refTrackingToken, UntrackedRefToken()));
            ptr->deref();
        }
    }
    void adoptRef(const auto* ptr)
    {
        WTFLogAlways("RefDerefTraits(RefLogger): adoptRef.");
        if (LIKELY(ptr != nullptr)) {
            m_refTrackingToken =  RefTracker::sharedTracker().trackRef();
            return;
        }

        m_refTrackingToken = UntrackedRefToken();
    }
    void takeRef(RefTrackingSmartPtr auto& smartPtr)
    {
        WTFLogAlways("RefDerefTraits(Node RefTracking): takeRef (RefTrackingSmartPtr)");
        m_refTrackingToken = smartPtr.refTrackingToken();
        smartPtr.setRefTrackingToken(UntrackedRefToken());
    }

    void takeRef(const SmartPtr auto& smartPtr)
    {
        WTFLogAlways("RefDerefTraits(Node RefTracking): takeRef (SmartPtr)");
        adoptRef(smartPtr.ptr());
    }

    void swapRef(RefTrackingSmartPtr auto& smartPtr)
    {
        WTFLogAlways("RefDerefTraits(Node RefTracking): swap (RefTrackingSmartPtr)");
        RefTrackingToken tmp = m_refTrackingToken;
        m_refTrackingToken = smartPtr.refTrackingToken();
        smartPtr.setRefTrackingToken(tmp);
    }

    void swapRef(const SmartPtr auto&)
    {
        WTFLogAlways("RefDerefTraits(Node RefTracking): swap (SmartPtr)");
        m_refTrackingToken = UntrackedRefToken();
    }

    RefTrackingToken refTrackingToken() const
    {
        return m_refTrackingToken;
    }

    void setRefTrackingToken(RefTrackingToken token)
    {
        m_refTrackingToken = token;
    }
private:
    RefTrackingToken m_refTrackingToken;
};

template<> struct RefDerefTraits<TestWebKitAPI::TrackedRefCheckingRefLogger> : public RefLoggerRefTrackingTraits { };
template<> struct RefDerefTraits<TestWebKitAPI::DerivedTrackedRefCheckingRefLogger> : public RefLoggerRefTrackingTraits { };
}

using WTF::RefLoggerRefTrackingTraits;

namespace TestWebKitAPI {

struct TrackedRefCheckingRefLogger : RefLogger {
    TrackedRefCheckingRefLogger(const char *name) : RefLogger(name) { }
    void ref() const
    {
        if (slotToCheck)
            log() << "slot=" << slotToCheck->get().name << " ";
        RefLogger::ref();
    }
    void deref() const
    {
        if (slotToCheck)
            log() << "slot=" << slotToCheck->get().name << " ";
        RefLogger::deref();
    }
    const Ref<TrackedRefCheckingRefLogger>* slotToCheck { nullptr };
};

struct DerivedTrackedRefCheckingRefLogger : TrackedRefCheckingRefLogger {
    DerivedTrackedRefCheckingRefLogger(const char* name) : TrackedRefCheckingRefLogger(name) { }
};

TEST(WTF_TrackedRef, Basic)
{
    DerivedRefLogger a("a");

    {
        Ref<RefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_EQ(&a.name, &ref->name);
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        Ref<RefLogger> ref(adoptRef(a));
        EXPECT_EQ(&a, ref.ptr());
        EXPECT_EQ(&a.name, &ref->name);
    }
    EXPECT_STREQ("deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRef, Assignment)
{
    DerivedRefLogger a("a");
    RefLogger b("b");
    DerivedRefLogger c("c");

    {
        Ref<RefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        ref = b;
        EXPECT_EQ(&b, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(b) deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        Ref<RefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        ref = c;
        EXPECT_EQ(&c, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(c) deref(a) | deref(c) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        Ref<RefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        ref = adoptRef(b);
        EXPECT_EQ(&b, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(b) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();

    {
        Ref<RefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        ref = adoptRef(c);
        EXPECT_EQ(&c, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | deref(a) | deref(c) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

namespace {
static Ref<RefLogger> passWithRef(Ref<RefLogger>&& reference)
{
    return WTFMove(reference);
}
}

TEST(WTF_TrackedRef, ReturnValue)
{
    DerivedRefLogger a("a");
    RefLogger b("b");
    DerivedRefLogger c("c");

    {
        Ref<RefLogger> ref(passWithRef(Ref<RefLogger>(a)));
        EXPECT_EQ(&a, ref.ptr());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());

    {
        Ref<RefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        ref = passWithRef(b);
        EXPECT_EQ(&b, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | ref(b) deref(a) | deref(b) ", takeLogStr().c_str());

    {
        RefPtr<RefLogger> ptr(passWithRef(a));
        EXPECT_EQ(&a, ptr.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());

    {
        RefPtr<DerivedRefLogger> ptr(&a);
        RefPtr<RefLogger> ptr2(WTFMove(ptr));
        EXPECT_EQ(nullptr, ptr.get());
        EXPECT_EQ(&a, ptr2.get());
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());

    {
        Ref<DerivedRefLogger> derivedReference(a);
        Ref<RefLogger> baseReference(passWithRef(derivedReference.copyRef()));
        EXPECT_EQ(&a, derivedReference.ptr());
        EXPECT_EQ(&a, baseReference.ptr());
    }
    EXPECT_STREQ("ref(a) ref(a) deref(a) deref(a) ", takeLogStr().c_str());
}

TEST(WTF_TrackedRef, Swap)
{
    RefLogger a("a");
    RefLogger b("b");

    {
        Ref<RefLogger> p1(a);
        Ref<RefLogger> p2(b);
        log() << "| ";
        EXPECT_EQ(&a, p1.ptr());
        EXPECT_EQ(&b, p2.ptr());
        p1.swap(p2);
        EXPECT_EQ(&b, p1.ptr());
        EXPECT_EQ(&a, p2.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());

    {
        Ref<RefLogger> p1(a);
        Ref<RefLogger> p2(b);
        log() << "| ";
        EXPECT_EQ(&a, p1.ptr());
        EXPECT_EQ(&b, p2.ptr());
        std::swap(p1, p2);
        EXPECT_EQ(&b, p1.ptr());
        EXPECT_EQ(&a, p2.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) ref(b) | | deref(a) deref(b) ", takeLogStr().c_str());
}

TEST(WTF_TrackedRef, AssignBeforeDeref)
{
    DerivedTrackedRefCheckingRefLogger a("a");
    TrackedRefCheckingRefLogger b("b");
    DerivedTrackedRefCheckingRefLogger c("c");

    {
        Ref<TrackedRefCheckingRefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        a.slotToCheck = &ref;
        b.slotToCheck = &ref;
        ref = b;
        a.slotToCheck = nullptr;
        b.slotToCheck = nullptr;
        EXPECT_EQ(&b, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | slot=a ref(b) slot=b deref(a) | deref(b) ", takeLogStr().c_str());

    {
        Ref<TrackedRefCheckingRefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        a.slotToCheck = &ref;
        c.slotToCheck = &ref;
        ref = c;
        a.slotToCheck = nullptr;
        c.slotToCheck = nullptr;
        EXPECT_EQ(&c, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | slot=a ref(c) slot=c deref(a) | deref(c) ", takeLogStr().c_str());

    {
        Ref<TrackedRefCheckingRefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        a.slotToCheck = &ref;
        ref = adoptRef(b);
        a.slotToCheck = nullptr;
        EXPECT_EQ(&b, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | slot=b deref(a) | deref(b) ", takeLogStr().c_str());

    {
        Ref<TrackedRefCheckingRefLogger> ref(a);
        EXPECT_EQ(&a, ref.ptr());
        log() << "| ";
        a.slotToCheck = &ref;
        ref = adoptRef(c);
        a.slotToCheck = nullptr;
        EXPECT_EQ(&c, ref.ptr());
        log() << "| ";
    }
    EXPECT_STREQ("ref(a) | slot=c deref(a) | deref(c) ", takeLogStr().c_str());
}

TEST(WTF_TrackedRef, StaticReferenceCastFromConstReference)
{
    {
        DerivedTrackedRefCheckingRefLogger a("a");
        const Ref<DerivedTrackedRefCheckingRefLogger> ref(a);
        auto ref2 = static_reference_cast<TrackedRefCheckingRefLogger>(ref);
    }
    EXPECT_STREQ("ref(a) ref(a) deref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

TEST(WTF_TrackedRef, StaticReferenceCastFromRValueReference)
{
    {
        DerivedTrackedRefCheckingRefLogger a("a");
        Ref<DerivedTrackedRefCheckingRefLogger> ref(a);
        auto ref2 = static_reference_cast<TrackedRefCheckingRefLogger>(WTFMove(ref));
    }
    EXPECT_STREQ("ref(a) deref(a) ", takeLogStr().c_str());
    RefTracker::sharedTracker().showRemainingReferences();
}

} // namespace TestWebKitAPI

