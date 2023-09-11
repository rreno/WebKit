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

#pragma once

#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/RefDerefTraits.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

#if USE(CF)
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace WTF {

class StackShot;

class RefTracker {
public:
    WTF_EXPORT_PRIVATE RefTracker();
    WTF_EXPORT_PRIVATE ~RefTracker();

    WTF_EXPORT_PRIVATE static RefTracker& sharedTracker();
    WTF_EXPORT_PRIVATE static RefTracker& strongTracker();
    WTF_EXPORT_PRIVATE static RefTracker& retainTracker();
    WTF_EXPORT_PRIVATE void showRemainingReferences() const;
//    WTF_EXPORT_PRIVATE void showBacktraceForToken(RefTrackingToken) const;

    WTF_EXPORT_PRIVATE RefTrackingToken trackRef(const String& url = nullString());
    WTF_EXPORT_PRIVATE void trackDeref(RefTrackingToken);

private:
    RefTrackingToken getNextRefToken();
    size_t refBacktraceMapSize() const;
    size_t untrackableDerefsSize() const;

    HashMap<RefTrackingToken::ValueType, std::pair<String, std::unique_ptr<StackShot>>> m_refBacktraceMap WTF_GUARDED_BY_LOCK(m_refBacktraceLock);
    Vector<std::unique_ptr<StackShot>> m_untrackableDerefs WTF_GUARDED_BY_LOCK(m_untrackableDerefsLock);

    // ThreadSafeRefCounted means we might ref/deref on another thread.
    mutable Lock m_refBacktraceLock;
    mutable Lock m_untrackableDerefsLock;
};

inline void RefTrackingTraits::ref(auto& object)
{
//    WTFLogAlways("RefDerefTraits(RefTracking) ref.");
    m_refTrackingToken = object.trackRef();
    object.ref();
}

inline void RefTrackingTraits::refIfNotNull(auto* ptr)
{
//    WTFLogAlways("RefDerefTraits(RefTracking) refIfNotNull. %" PRIuPTR " (ptr)", (uintptr_t)ptr);
    if (LIKELY(ptr != nullptr)) {
        m_refTrackingToken = ptr->trackRef();
        ptr->ref();
        return;
    }

    m_refTrackingToken = UntrackedRefToken();
}

inline void RefTrackingTraits::derefIfNotNull(auto* ptr)
{
//    WTFLogAlways("RefDerefTraits(RefTracking) derefIfNotNull. %" PRIuPTR " (ptr)"/* %u (token)"*/, (uintptr_t)ptr/*, m_refTrackingToken.value()*/);
    if (LIKELY(ptr != nullptr)) {
        ptr->trackDeref(std::exchange(m_refTrackingToken, UntrackedRefToken()));
        ptr->deref();
    }
}

inline void RefTrackingTraits::adoptRef(auto* ptr)
{
//    WTFLogAlways("RefDerefTraits(RefTracking): adoptRef. %" PRIuPTR " (ptr)", (uintptr_t)ptr);
    if (LIKELY(ptr != nullptr)) {
        m_refTrackingToken =  ptr->trackRef();
        return;
    }

    m_refTrackingToken = UntrackedRefToken();
}

inline void RefTrackingTraits::takeRef(RefTrackingSmartPtr auto& smartPtr)
{
//    WTFLogAlways("RefDerefTraits(Node RefTracking): takeRef (RefTrackingSmartPtr)");
    m_refTrackingToken = smartPtr.refTrackingToken();
    smartPtr.setRefTrackingToken(UntrackedRefToken());
}

inline void RefTrackingTraits::takeRef(const SmartPtr auto& smartPtr)
{
//    WTFLogAlways("RefDerefTraits(Node RefTracking): takeRef (SmartPtr)");
    adoptRef(smartPtr.ptr());
}

inline void RefTrackingTraits::swapRef(RefTrackingSmartPtr auto& smartPtr)
{
//    WTFLogAlways("RefDerefTraits(Node RefTracking): swap (RefTrackingSmartPtr)");
    RefTrackingToken tmp = m_refTrackingToken;
    m_refTrackingToken = smartPtr.refTrackingToken();
    smartPtr.setRefTrackingToken(tmp);
}

inline void RefTrackingTraits::swapRef(const SmartPtr auto&)
{
//    WTFLogAlways("RefDerefTraits(Node RefTracking): swap (SmartPtr)");
    m_refTrackingToken = UntrackedRefToken();
}

inline RefTrackingToken RefTrackingTraits::refTrackingToken() const
{
    return m_refTrackingToken;
}

inline void RefTrackingTraits::setRefTrackingToken(RefTrackingToken token)
{
    m_refTrackingToken = token;
}

inline void RetainTrackingTraits::retainIfNotNull(auto* ptr)
{
    WTFLogAlways("RetainReleaseTraits(RefTracking) retainIfNotNull. %" PRIuPTR " (ptr)", (uintptr_t)ptr);
    if (LIKELY(ptr != nullptr)) {
        m_trackingToken = RefTracker::retainTracker().trackRef();
        CFRetain(ptr);
        return;
    }

    m_trackingToken = UntrackedRefToken();
}

inline void RetainTrackingTraits::releaseIfNotNull(auto* ptr)
{
    WTFLogAlways("RetainReleaseTraits(RefTracking) releaseIfNotNull. %" PRIuPTR " (ptr)"/* %u (token)"*/, (uintptr_t)ptr/*, m_refTrackingToken.value()*/);
    if (LIKELY(ptr != nullptr)) {
        RefTracker::retainTracker().trackDeref(std::exchange(m_trackingToken, UntrackedRefToken()));
        CFRelease(ptr);
    }
}

inline void RetainTrackingTraits::adopt(auto* ptr)
{
    WTFLogAlways("RetainReleaseTraits(RefTracking) adopt. %" PRIuPTR " (ptr)"/* %u (token)"*/, (uintptr_t)ptr/*, m_refTrackingToken.value()*/);
    if (LIKELY(ptr != nullptr)) {
        m_trackingToken = RefTracker::retainTracker().trackRef();
        WTFLogAlways("    Retain Count: %u", (unsigned)((CFIndex)CFGetRetainCount(ptr)));
        return;
    }
    m_trackingToken = UntrackedRefToken();
}

} // namespace WTF

using WTF::RefTracker;
