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

#include <wtf/Forward.h>
#include <wtf/RefTracking.h>

namespace WebCore {
class EventTarget;
class Node;
class ContainerNode;
class Document;
class HTMLDocument;
class FTPDirectoryDocument;
class ImageDocument;
class MediaDocument;
class ModelDocument;
class PluginDocument;
class SinkDocument;
class TextDocument;
class EditCommand;
class CompositeEditCommand;
class InsertIntoTextNodeCommand;
class InsertTextCommand;
class SimpleEditCommand;
} // namespace WebCore

namespace WTF {

template <typename T> concept IsRef = T::isRef;
template <typename T> concept IsRefPtr = T::isRefPtr;
template <typename T> concept SmartPtr = IsRef<T> || IsRefPtr<T>;

template <typename T> concept RefTracked = RefDerefTraits<T>::isRefTracked;
template <typename T> concept RefTrackingSmartPtr = SmartPtr<T> && T::isRefTracking;

// Default Traits - this should cover almost all uses of Ref/RefPtr.
// This can be specialized if you want different behavior for a default Ref<T> or RefPtr<T>.
// For instance instrumenting Ref/Deref call pairs for leak checking (see RefTrackingTraits below).
template <typename T>
struct RefDerefTraits {
    static ALWAYS_INLINE void ref(T& object)
    {
        object.ref();
    }

    static ALWAYS_INLINE void refIfNotNull(T* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->ref();
    }

    static ALWAYS_INLINE void derefIfNotNull(T* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->deref();
    }

    static void adoptRef(auto*) { }
    static void swapRef(SmartPtr auto&) { }
    static void takeRef(SmartPtr auto&) { }
};

// Defined in RefTracker.h to avoid circular dependencies between Ref/RefPtr and the RefTracker's HashMap/Vector.
struct RefTrackingTraits {
    static constexpr bool isRefTracked = true;
    ALWAYS_INLINE void ref(auto&);
    ALWAYS_INLINE void refIfNotNull(auto*);
    ALWAYS_INLINE void derefIfNotNull(auto*);
    ALWAYS_INLINE void adoptRef(auto*);
    ALWAYS_INLINE void swapRef(RefTrackingSmartPtr auto&);
    ALWAYS_INLINE void swapRef(const SmartPtr auto&);
    ALWAYS_INLINE void takeRef(RefTrackingSmartPtr auto&);
    ALWAYS_INLINE void takeRef(const SmartPtr auto&);

    ALWAYS_INLINE RefTrackingToken refTrackingToken() const;
    ALWAYS_INLINE void setRefTrackingToken(RefTrackingToken);
private:
    RefTrackingToken m_refTrackingToken;
};

#define DEFINE_REF_TRACKING_TRAITS_FOR(anObject) \
template<> struct RefDerefTraits<anObject> : public RefTrackingTraits { }


DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::EditCommand);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::InsertTextCommand);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SimpleEditCommand);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::InsertIntoTextNodeCommand);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::CompositeEditCommand);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::EventTarget);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Node);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ContainerNode);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Document);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::FTPDirectoryDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ImageDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::MediaDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ModelDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::PluginDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SinkDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::TextDocument);

#undef DEFINE_REF_TRACKING_TRAITS_FOR
} // namespace WTF

using WTF::RefDerefTraits;
