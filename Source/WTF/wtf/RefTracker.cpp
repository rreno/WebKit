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
#include "RefTracker.h"

#include <atomic>
#include <wtf/NeverDestroyed.h>
#include <wtf/RefTracking.h>
#include <wtf/StackShot.h>

namespace WTF {

static constexpr size_t maxStackSize = 512;

RefTrackingToken RefTracker::getNextRefToken()
{
    static std::atomic<RefTrackingToken::ValueType> tokenValue{};
    return RefTrackingToken(++tokenValue);
}

RefTracker& RefTracker::sharedTracker()
{
    static NeverDestroyed<RefTracker> tracker {};
    return tracker.get();
}

RefTracker& RefTracker::strongTracker()
{
    static NeverDestroyed<RefTracker> tracker {};
    return tracker.get();
}

RefTracker& RefTracker::retainTracker()
{
    static NeverDestroyed<RefTracker> tracker {};
    return tracker.get();
}

RefTracker::RefTracker() = default;
RefTracker::~RefTracker() = default;

RefTrackingToken RefTracker::trackRef(const String& url)
{
    auto token = getNextRefToken();
    {
        Locker lock { m_refBacktraceLock };
        m_refBacktraceMap.add(token.value(), std::make_pair(url, std::make_unique<StackShot>(maxStackSize)));
    }
    //WTFLogAlways("RefTracker: Added token %u (%s)", token.value(), url.utf8().data());
    return token;
}

void RefTracker::trackDeref(RefTrackingToken token)
{
    if (!token.value()) {
        Locker lock { m_untrackableDerefsLock };
        m_untrackableDerefs.append(std::make_unique<StackShot>(maxStackSize));
        return;
    }

    bool removed = false;
    {
        Locker lock { m_refBacktraceLock };
        removed = m_refBacktraceMap.remove(token.value());
    }

    if (!removed)
        WTFLogAlways("RefTracker: trackDeref passed token %u that was not tracked or already removed.", token.value());

//    WTFLogAlways("RefTracker: Removed token %u", token.value());
}

size_t RefTracker::refBacktraceMapSize() const
{
    Locker lock { m_refBacktraceLock };
    return m_refBacktraceMap.size();
}

size_t RefTracker::untrackableDerefsSize() const
{
    Locker lock { m_untrackableDerefsLock };
    return m_untrackableDerefs.size();
}

void RefTracker::showRemainingReferences() const
{
    if (refBacktraceMapSize() == 0 && untrackableDerefsSize() == 0)
        WTFLogAlways("RefTracker: No remaining references.");
    const size_t framesToSkip = 4;
    {
        Locker lock { m_refBacktraceLock };
        for (const auto& [token, stack] : m_refBacktraceMap) {
            WTFLogAlways("RefTracker: Backtrace for token %u (%s)\n", token, stack.first.utf8().data());
            WTFPrintBacktrace(stack.second->array() + framesToSkip, stack.second->size() - framesToSkip);
            WTFLogAlways("\n");
        }
    }

#if 0
    WTFLogAlways("RefTracker: %zu untracked derefs.", m_untrackableDerefs.size());
#endif
#if 1
    {
        Locker lock { m_untrackableDerefsLock };
        for (const auto& stack : m_untrackableDerefs) {
            WTFLogAlways("RefTracker: Refs for the following were not tracked:\n:");
            WTFPrintBacktrace(stack->array() + framesToSkip, stack->size() - framesToSkip);
        }
    }
#endif
}

#if 0
void RefTracker::showBacktraceForToken(RefTrackingToken token) const
{
    const size_t framesToSkip = 4;
    if (token.value() == 0) {
        WTFLogAlways("RefTracker: untracked ref token - no backtrace to print.");
        return;
    }
    if (!m_refBacktraceMap.contains(token.value())) {
        WTFLogAlways("RefTracker: token %u not tracked.", token.value());
        return;
    }
    const auto it = m_refBacktraceMap.find(token.value());
    WTFLogAlways("RefTracker: Backtrace for token %u (%s)\n", token.value(), it->value.first.utf8().data());
    WTFPrintBacktrace(it->value.second->array() + framesToSkip, it->value.second->size() - framesToSkip);
    WTFLogAlways("\n");
}
#endif

} // namespace WTF
