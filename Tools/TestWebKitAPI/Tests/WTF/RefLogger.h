/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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

#include "Logger.h"
#include <wtf/FastMalloc.h>
#include <wtf/RefDerefTraits.h>
#include <wtf/RefTracker.h>

namespace TestWebKitAPI {
struct RefTrackedRefLogger;
struct DerivedRefTrackedRefLogger;
struct RTCheckingRefPtrLogger;
struct RTCheckingRefLogger;

struct RefLogger {
    WTF_MAKE_STRUCT_FAST_COMPACT_ALLOCATED;

    RefLogger(const char* name);
    void ref() const;
    void deref() const;
    const char& name;
};

struct DerivedRefLogger : RefLogger {
    DerivedRefLogger(const char* name);
};
} // namespace TestWebKitAPI

namespace WTF {
DEFINE_REF_TRACKING_TRAITS_FOR(TestWebKitAPI::RefTrackedRefLogger);
DEFINE_REF_TRACKING_TRAITS_FOR(TestWebKitAPI::DerivedRefTrackedRefLogger);
DEFINE_REF_TRACKING_TRAITS_FOR(TestWebKitAPI::RTCheckingRefPtrLogger);
DEFINE_REF_TRACKING_TRAITS_FOR(TestWebKitAPI::RTCheckingRefLogger);
} // namespace WTF

namespace TestWebKitAPI {

struct RefTrackedRefLogger : private RefLogger {
    RefTrackedRefLogger(const char* name) : RefLogger(name) { }
    
    using RefLogger::name;
    using RefLogger::ref;
    using RefLogger::deref;
    
    RefTrackingToken trackRef() const { return RefTracker::sharedTracker().trackRef(); }
    void trackDeref(RefTrackingToken token) const { return RefTracker::sharedTracker().trackDeref(token); }
};

struct DerivedRefTrackedRefLogger : public RefTrackedRefLogger {
    DerivedRefTrackedRefLogger(const char* name) : RefTrackedRefLogger(name) { }
};

struct RTCheckingRefPtrLogger : private RefTrackedRefLogger {
    RTCheckingRefPtrLogger(const char* name) : RefTrackedRefLogger(name) { }

    using RefTrackedRefLogger::name;
    using RefTrackedRefLogger::ref;
    using RefTrackedRefLogger::deref;
    using RefTrackedRefLogger::trackRef;
    using RefTrackedRefLogger::trackDeref;

    RefPtr<RefTrackedRefLogger>* slotToCheck;
};

struct RTCheckingRefLogger : private RefTrackedRefLogger {
    RTCheckingRefLogger(const char* name) : RefTrackedRefLogger(name) { }

    using RefTrackedRefLogger::name;
    using RefTrackedRefLogger::ref;
    using RefTrackedRefLogger::deref;
    using RefTrackedRefLogger::trackRef;
    using RefTrackedRefLogger::trackDeref;

    Ref<RefTrackedRefLogger>* slotToCheck;
};
} // namespace TestWebKitAPI
