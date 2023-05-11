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

#include <wtf/HashSet.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/RefTracking.h>
#include <wtf/StackShot.h>

namespace WTF {

static constexpr size_t maxStackSize = 512;
static constexpr size_t framesToSkip = 4;

static auto& nodeRefBacktraceSet()
{
    static NeverDestroyed<HashSet<uintptr_t>> set { };
    return set.get();
}

RefTrackingToken RefTracker::getNextRefToken()
{
    static RefTrackingToken::ValueType tokenValue{};
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

RefTracker::RefTracker() = default;
RefTracker::~RefTracker() = default;

void RefTracker::trackNodeRef(void* node)
{
    nodeRefBacktraceSet().add((uintptr_t)node);
}

void RefTracker::trackNodeDeref(void* node)
{
    nodeRefBacktraceSet().remove((uintptr_t)node);
}

void RefTracker::trackDocRef(void* node, const String& tag)
{
    auto result = m_docRefBacktraceMap.add((uintptr_t)node, std::make_pair(tag, std::make_unique<StackShot>(maxStackSize)));
    if (!result)
        WTFLogAlways("RefTracker::trackDocRef: Tracking a ref from a Node which already refed the document!");
}

void RefTracker::trackDocDeref(void* node)
{
    auto it = m_docRefBacktraceMap.find((uintptr_t)node);
    if (it == m_docRefBacktraceMap.end())
        return WTFLogAlways("RefTracker::trackDocDeref: Node derefing document either did not ref it or has already derefed it.");

    m_docRefBacktraceMap.remove(it);
}

RefTrackingToken RefTracker::trackRef(const String& url)
{
    auto token = getNextRefToken();
    m_refBacktraceMap.add(token.value(), std::make_pair(url, std::make_unique<StackShot>(maxStackSize)));
    //WTFLogAlways("RefTracker: Added token %u (%s)", token.value(), url.utf8().data());
    return token;
}

void RefTracker::trackDeref(RefTrackingToken token)
{
    if (!token.value()) {
        m_untrackableDerefs.append(std::make_unique<StackShot>(maxStackSize));
        return;
    }

    if (!m_refBacktraceMap.remove(token.value()))
        WTFLogAlways("RefTracker: trackDeref passed token %u that was not tracked or already removed.", token.value());

//    WTFLogAlways("RefTracker: Removed token %u", token.value());
}

void RefTracker::showRemainingReferences() const
{
    if (m_refBacktraceMap.size() == 0 && m_untrackableDerefs.size() == 0)
        WTFLogAlways("RefTracker: No remaining references.");
    for (const auto& [token, stack] : m_refBacktraceMap) {
        WTFLogAlways("RefTracker: Backtrace for token %u (%s)\n", token, stack.first.utf8().data());
        WTFPrintBacktrace(stack.second->array() + framesToSkip, stack.second->size() - framesToSkip);
        WTFLogAlways("\n");
    }

    if (!m_docRefBacktraceMap.size())
        WTFLogAlways("RefTracker: No remaining nodes which refed a document.");

    for (const auto& [ptr, stack] : m_docRefBacktraceMap) {
        WTFLogAlways("RefTracker: Backtrace for node %p (%s)", (void*)ptr, stack.first.utf8().data());
        WTFPrintBacktrace(stack.second->array() + framesToSkip, stack.second->size() - framesToSkip);
        WTFLogAlways("\n");
    }

#if 0
    WTFLogAlways("RefTracker: %zu untracked derefs.", m_untrackableDerefs.size());
#endif
#if 1
    for (const auto& stack : m_untrackableDerefs) {
        WTFLogAlways("RefTracker: Refs for the following were not tracked:\n");
        WTFPrintBacktrace(stack->array() + framesToSkip, stack->size() - framesToSkip);
    }
#endif
}

void RefTracker::showRemainingNodes()
{
    if (!nodeRefBacktraceSet().size()) {
        WTFLogAlways("RefTracker: No remaining nodes.");
        return;
    }

    WTFLogAlways("RefTracker: Nodes which were created but not destroyed:\n");
    for (const auto nodePtr : nodeRefBacktraceSet())
        WTFLogAlways("Node (%p)", (void*)nodePtr);
}

void RefTracker::showBacktraceForToken(RefTrackingToken token) const
{
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
}

} // namespace WTF
