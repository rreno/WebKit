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

#if USE(CF)
typedef struct __CVBuffer* CVPixelBufferRef;
typedef struct CGImage* CGImageRef;
#endif

namespace WebKit {
class ShareableBitmap;
}

namespace WebCore {
class ImageDecoderAVFObjCSample;
class MediaSampleAVFObjC;
class MediaSample;
class NativeImage;
class Image;
class RenderingResource;
class ImageSource;
class Node;
class Element;
class ContainerNode;
class HTMLDocument;
class Document;
class EventTarget;
class MutationObserver;
class HTMLElement;
class HTMLSpanElement;
class HTMLImageElement;
class HTMLParagraphElement;
class HTMLAnchorElement;
class HTMLLIElement;
class HTMLUListElement;
class HTMLDivElement;
class HTMLButtonElement;
} // namespace WebCore

namespace WTF {

template <typename T> concept IsRef = T::isRef;
template <typename T> concept IsRefPtr = T::isRefPtr;
template <typename T> concept SmartPtr = IsRef<T> || IsRefPtr<T>;

template <typename T> concept RefTracked = RefDerefTraits<T>::isRefTracked || RetainReleaseTraits<T>::isRefTracked;
template <typename T> concept RefTrackingSmartPtr = SmartPtr<T> && T::isRefTracking;

// Default Traits - this should cover almost all uses of Ref/RefPtr.
// This can be specialized if you want different behavior for a default Ref<T> or RefPtr<T>.
// For instance instrumenting Ref/Deref call pairs for leak checking (see RefTrackingTraits below).
template <typename T>
struct RefDerefTraits {
    static ALWAYS_INLINE T& ref(T& object)
    {
        object.ref();
        return object;
    }

    static ALWAYS_INLINE T* refIfNotNull(T* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->ref();
        return ptr;
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

struct EventTargetRefTrackingTraits {
    static constexpr bool isRefTracked = false;

    static ALWAYS_INLINE auto& ref(auto& object)
    {
        object.ref();
        return object;
    }

    static ALWAYS_INLINE auto* refIfNotNull(auto* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->ref();
        return ptr;
    }

    static ALWAYS_INLINE void derefIfNotNull(auto* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->deref();
    }

    static void adoptRef(auto*) { }
    ALWAYS_INLINE static void swapRef(RefTrackingSmartPtr auto& ptr)
    {
        // We're going to give up on ref tracking this object for now.
        if (LIKELY(ptr != nullptr)) {
            ptr->trackDeref(ptr.refTrackingToken());
            ptr.setRefTrackingToken(UntrackedRefToken());
        }
    }
    static void swapRef(SmartPtr auto&) { }
    ALWAYS_INLINE static void takeRef(RefTrackingSmartPtr auto& ptr)
    {
        if (LIKELY(ptr != nullptr)) {
            ptr->trackDeref(ptr.refTrackingToken());
            ptr.setRefTrackingToken(UntrackedRefToken());
        }
    }
    static void takeRef(SmartPtr auto&) { }
};

template <typename T>
struct RetainReleaseTraits {

    // Defined in RetainPtr.h
    static ALWAYS_INLINE void retainIfNotNull(auto*);
    static ALWAYS_INLINE void releaseIfNotNull(auto*);

    static void adopt(auto*) { }
    static ALWAYS_INLINE void swap(auto&) { }
    static void take(auto&) { }
    static void leak(auto*) { }
};

struct RetainTrackingTraits {
    static constexpr bool isRefTracked = true;

    // Defined in RefTracker.h to avoid circular dependencies between RetainPtr and the RefTracker's HashMap/Vector.
    ALWAYS_INLINE void retainIfNotNull(auto*);
    ALWAYS_INLINE void releaseIfNotNull(auto*);
    ALWAYS_INLINE void adopt(auto*);

    ALWAYS_INLINE void swap(auto& rhs)
    {
        RefTrackingToken tmp = m_trackingToken;
        m_trackingToken = rhs.trackingToken();
        rhs.setTrackingToken(tmp);
    }

    void take(auto& o)
    {
        m_trackingToken = o.trackingToken();
        o.setTrackingToken(UntrackedRefToken());
    }

    ALWAYS_INLINE void leak(auto* ptr)
    {
        if (ptr != nullptr)
            WTFLogAlways("RetainReleaseTraits(RefTracking): leak %p (retain count %lu)", ptr, (unsigned long)CFGetRetainCount(ptr));
    }

    ALWAYS_INLINE RefTrackingToken trackingToken() const { return m_trackingToken; }
    ALWAYS_INLINE void setTrackingToken(RefTrackingToken token) { m_trackingToken = token; }

private:
    RefTrackingToken m_trackingToken;
};

template <> struct RetainReleaseTraits<CVPixelBufferRef> : public RetainTrackingTraits { };
template <> struct RetainReleaseTraits<CGImageRef> : public RetainTrackingTraits { };

#define DEFINE_REF_TRACKING_TRAITS_FOR(anObject) \
template<> struct RefDerefTraits<anObject> : public RefTrackingTraits { }

//DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::EventTarget);
DEFINE_REF_TRACKING_TRAITS_FOR(WebKit::ShareableBitmap);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::NativeImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ImageSource);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Image);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::RenderingResource);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Node);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ContainerNode);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Document);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Element);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLSpanElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLImageElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLParagraphElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLAnchorElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLLIElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLUListElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDivElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLButtonElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::MutationObserver);

template <> struct RefDerefTraits<WebCore::EventTarget> : public EventTargetRefTrackingTraits { };


#undef DEFINE_REF_TRACKING_TRAITS_FOR
} // namespace WTF

using WTF::RefDerefTraits;
