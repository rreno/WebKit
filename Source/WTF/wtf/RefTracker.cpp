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

#include <wtf/Locker.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/RefTracking.h>
#include <wtf/StackShot.h>

namespace WTF {

static constexpr size_t maxStackSize = 512;

RefTrackingToken RefTracker::getNextRefToken()
{
    static RefTrackingToken::ValueType tokenValue{};
    return RefTrackingToken(++tokenValue);
}

HashMap<RefTrackingToken::ValueType, std::pair<std::pair<void*,void*>, std::unique_ptr<StackShot>>>& RefTracker::docRefBacktraceMap()
{
    static NeverDestroyed<HashMap<RefTrackingToken::ValueType, std::pair<std::pair<void*,void*>, std::unique_ptr<StackShot>>>> map;
    return map.get();
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

RefTracker::RefTracker() = default;
RefTracker::~RefTracker() = default;

RefTrackingToken RefTracker::trackRef(const String& tag)
{
    auto token = getNextRefToken();
    m_refBacktraceMap.add(token.value(), std::make_pair(tag, std::make_unique<StackShot>(maxStackSize)));
    return token;
}

RefTrackingToken RefTracker::trackDocRef(void* document, void* node)
{
    static RefTrackingToken::ValueType value { 0 };
    auto token = RefTrackingToken(++value);
    RefTracker::docRefBacktraceMap().add(token.value(), std::make_pair(std::make_pair(document, node), std::make_unique<StackShot>(maxStackSize)));
    return token;
}

void RefTracker::trackDocDeref(RefTrackingToken token, void* node)
{
    if (!token.value())
        return;
    
    auto& map = RefTracker::docRefBacktraceMap();
    auto it = map.find(token.value());
    if (it == map.end())
        return;
    
    if (it->value.first.second != node) {
        WTFLogAlways("RefTracker: trackDocDeref called with a different Node than the one that reffed it.");
        return;
    }
    RefTracker::docRefBacktraceMap().remove(token.value());
}

void RefTracker::trackDeref(RefTrackingToken token)
{
    if (!token.value()) {
        m_untrackableDerefs.append(std::make_unique<StackShot>(maxStackSize));
        return;
    }

    if (!m_refBacktraceMap.remove(token.value()))
        WTFLogAlways("RefTracker: trackDeref passed token %u that was not tracked or already removed.", token.value());
}

void RefTracker::showRemainingReferences() const
{
    if (m_refBacktraceMap.size() == 0 && m_untrackableDerefs.size() == 0)
        WTFLogAlways("RefTracker: No remaining references.");
    constexpr size_t framesToSkip = 4;
    for (const auto& [token, stack] : m_refBacktraceMap) {
        WTFLogAlways("RefTracker: Backtrace for token %u (%s)\n", token, stack.first.utf8().data());
        WTFPrintBacktrace(stack.second->array() + framesToSkip, stack.second->size() - framesToSkip);
        WTFLogAlways("\n");
    }
    
    for (const auto& stack : m_untrackableDerefs) {
        WTFLogAlways("RefTracker: Refs for the following were not tracked:\n:");
        WTFPrintBacktrace(stack->array() + framesToSkip, stack->size() - framesToSkip);
    }
    
    if (!RefTracker::docRefBacktraceMap().size())
        return;
}

void RefTracker::showRemainingDocReferences()
{
    constexpr size_t framesToSkip = 4;
    WTFLogAlways("RefTracker: Nodes which refed a document but did not deref");
    for (const auto& [token, stack] : RefTracker::docRefBacktraceMap()) {
        if (!stack.first.first && !stack.first.second) continue;
        WTFLogAlways("RefTracker: Backtrace for token %u (document %p) (node %p)", token, stack.first.first, stack.first.second);
        WTFPrintBacktrace(stack.second->array() + framesToSkip, stack.second->size() - framesToSkip);
    }
}

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

} // namespace WTF
