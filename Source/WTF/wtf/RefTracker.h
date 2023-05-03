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

namespace WTF {

class StackShot;

class RefTracker {
public:
    WTF_EXPORT_PRIVATE RefTracker();
    WTF_EXPORT_PRIVATE ~RefTracker();

    WTF_EXPORT_PRIVATE static RefTracker& sharedTracker();
    WTF_EXPORT_PRIVATE static RefTracker& strongTracker();
    WTF_EXPORT_PRIVATE void showRemainingReferences() const;
    WTF_EXPORT_PRIVATE void showBacktraceForToken(RefTrackingToken) const;

    WTF_EXPORT_PRIVATE RefTrackingToken trackRef(const String& = nullString());
    WTF_EXPORT_PRIVATE void trackDeref(RefTrackingToken);

    WTF_EXPORT_PRIVATE static RefTrackingToken trackDocRef(void*, void*);
    WTF_EXPORT_PRIVATE static void trackDocDeref(RefTrackingToken, void*);
    WTF_EXPORT_PRIVATE static void showRemainingDocReferences();

private:
    RefTrackingToken getNextRefToken();
    static HashMap<RefTrackingToken::ValueType, std::pair<std::pair<void*, void*>, std::unique_ptr<StackShot>>>& docRefBacktraceMap();

    HashMap<RefTrackingToken::ValueType, std::pair<String, std::unique_ptr<StackShot>>> m_refBacktraceMap;
    Vector<std::unique_ptr<StackShot>> m_untrackableDerefs;
};

} // namespace WTF

using WTF::RefTracker;
