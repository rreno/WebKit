/*
 * Copyright (C) 2013-2023 Apple Inc. All rights reserved.
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

#include <wtf/Assertions.h>
#include <wtf/Forward.h>
#include <wtf/GetPtr.h>
#include <wtf/RawPtrTraits.h>
#include <wtf/RefDerefTraits.h>
#include <wtf/RefTracking.h>
#include <wtf/StdLibExtras.h>
#include <wtf/TypeCasts.h>

#if ASAN_ENABLED
extern "C" void __asan_poison_memory_region(void const volatile *addr, size_t size);
extern "C" void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
extern "C" int __asan_address_is_poisoned(void const volatile *addr);
#endif

namespace WTF {

inline void adopted(const void*) { /*WTFLogAlways("Empty adopted.");*/ }

template<typename T, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg> class Ref;
template<typename T, typename PtrTraitsArg = RawPtrTraits<T>, template <typename> typename RefDerefTraitsArg = RefDerefTraits> Ref<T, PtrTraitsArg, RefDerefTraitsArg> adoptRef(T&);

template<typename T, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg>
class Ref : private RefDerefTraitsArg<T> {
public:
    using PtrTraits = PtrTraitsArg;
    using RefDerefBase = RefDerefTraitsArg<T>;
    using ValueType = T;

    static constexpr bool isRef = true;
    static constexpr bool isRefTracking = RefTracked<T>;

    ~Ref()
    {
#if ASAN_ENABLED
        if (__asan_address_is_poisoned(this))
            __asan_unpoison_memory_region(this, sizeof(*this));
#endif
     /*   WTFLogAlways("Ref dtor. %" PRIuPTR" (m_ptr).", (uintptr_t)PtrTraits::unwrap(m_ptr));*/
        RefDerefBase::derefIfNotNull(PtrTraits::exchange(m_ptr, nullptr));
    }

    Ref(T& object) : m_ptr(&object) { /*WTFLogAlways("Ref ctor.");*/ RefDerefBase::ref(object); }
    Ref(const Ref& other) : m_ptr(other.ptr()) { ASSERT(m_ptr); /*WTFLogAlways("Ref copy ctor.");*/ RefDerefBase::ref(*PtrTraits::unwrap(m_ptr)); }

    template<typename X, typename Y, template <typename> typename Z>
    Ref(const Ref<X, Y, Z>& other)
        : m_ptr(other.ptr())
    {
        /*WTFLogAlways("Ref templated copy ctor.");*/
        ASSERT(m_ptr);
        RefDerefBase::ref(*PtrTraits::unwrap(m_ptr));
    }

    // In ASAN builds Ref will poison itself in leakRef. We need to take the ref tracking token first (if applicable)
    // in these move constructors.
    Ref(Ref&& other)
    {
        /*WTFLogAlways("Ref move ctor");*/
        RefDerefBase::takeRef(other);
        m_ptr = &other.leakRef();
        ASSERT(m_ptr);
    }

    template<typename X, typename Y, template <typename> typename Z>
    Ref(Ref<X, Y, Z>&& other)
    {
        /*WTFLogAlways("Ref templated move ctor");*/
        RefDerefBase::takeRef(other);
        m_ptr = &other.leakRef();
        ASSERT(m_ptr);
    }

    Ref& operator=(T&);
    Ref& operator=(Ref&&);
    template<typename X, typename Y, template <typename> typename Z> Ref& operator=(Ref<X, Y, Z>&&);

    Ref& operator=(const Ref&);
    template<typename X, typename Y, template <typename> typename Z> Ref& operator=(const Ref<X, Y, Z>&);

    template<typename X, typename Y, template <typename> typename Z> void swap(Ref<X, Y, Z>&);

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    Ref(HashTableDeletedValueType) : m_ptr(PtrTraits::hashTableDeletedValue()) { }
    bool isHashTableDeletedValue() const { return PtrTraits::isHashTableDeletedValue(m_ptr); }
    
    Ref(HashTableEmptyValueType) : m_ptr(hashTableEmptyValue()) { }
    bool isHashTableEmptyValue() const { return m_ptr == hashTableEmptyValue(); }
    static T* hashTableEmptyValue() { return nullptr; }
    
    const T* ptrAllowingHashTableEmptyValue() const { ASSERT(m_ptr || isHashTableEmptyValue()); return PtrTraits::unwrap(m_ptr); }
    T* ptrAllowingHashTableEmptyValue() { ASSERT(m_ptr || isHashTableEmptyValue()); return PtrTraits::unwrap(m_ptr); }
    
    T* operator->() const { ASSERT(m_ptr); return PtrTraits::unwrap(m_ptr); }
    T* ptr() const RETURNS_NONNULL { ASSERT(m_ptr); return PtrTraits::unwrap(m_ptr); }
    T& get() const { ASSERT(m_ptr); return *PtrTraits::unwrap(m_ptr); }
    operator T&() const { ASSERT(m_ptr); return *PtrTraits::unwrap(m_ptr); }
    bool operator!() const { ASSERT(m_ptr); return !*m_ptr; }
    ALWAYS_INLINE bool operator==(std::nullptr_t) const { ASSERT(m_ptr); return false; }
    ALWAYS_INLINE bool operator!=(std::nullptr_t) const { ASSERT(m_ptr); return true; }

    template<typename X, typename Y, template <typename> typename Z> Ref replace(Ref<X, Y, Z>&&) WARN_UNUSED_RETURN;

    // The following function is deprecated.
    Ref copyRef() && = delete;
    Ref copyRef() const & WARN_UNUSED_RETURN { return Ref(*m_ptr); }

    T& leakRef() WARN_UNUSED_RETURN
    {
        ASSERT(m_ptr);

        T& result = *PtrTraits::exchange(m_ptr, nullptr);
#if ASAN_ENABLED
        __asan_poison_memory_region(this, sizeof(*this));
#endif
        return result;
    }

    RefTrackingToken refTrackingToken() const requires isRefTracking { return RefDerefBase::refTrackingToken(); }
    void setRefTrackingToken(RefTrackingToken token) requires isRefTracking { RefDerefBase::setRefTrackingToken(token); }

    static Ref takeRef(T& object, RefTrackingToken token) requires isRefTracking
    {
        auto result = takeRef(object);
        result.setRefTrackingToken(token);
        return result;
    }

private:
    friend Ref adoptRef<T>(T&);
    template<typename X, typename Y, template <typename> typename Z> friend class Ref;
    template<typename X, typename Y, template <typename> typename Z> friend class RefPtr;

    enum AdoptTag { Adopt, Take };
    Ref(T& object, AdoptTag tag)
    : m_ptr(&object)
    {
        /*WTFLogAlways("Ref adopting ctor. Tag %s", tag == Adopt ? "Adopt" : "Take");*/
        if (tag == Adopt)
            RefDerefBase::adoptRef(&object);
    }

    static Ref takeRef(T& object)
    {
        adopted(&object);
        return Ref(object, AdoptTag::Take);
    }

    typename PtrTraits::StorageType m_ptr;
};

template<typename T, typename U, template <typename> typename V> Ref<T, U, V> adoptRef(T&);

template<typename T, typename U, template <typename> typename V>
inline Ref<T, U, V>& Ref<T, U, V>::operator=(T& reference)
{
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
inline Ref<T, U, V>& Ref<T, U, V>::operator=(Ref&& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref movedReference = WTFMove(reference);
    swap(movedReference);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline Ref<T, U, V>& Ref<T, U, V>::operator=(Ref<X, Y, Z>&& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref movedReference = WTFMove(reference);
    swap(movedReference);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
inline Ref<T, U, V>& Ref<T, U, V>::operator=(const Ref& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline Ref<T, U, V>& Ref<T, U, V>::operator=(const Ref<X, Y, Z>& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline void Ref<T, U, V>::swap(Ref<X, Y, Z>& other)
{
    U::swap(m_ptr, other.m_ptr);
    RefDerefBase::swapRef(other);
}

template<typename T, typename U, template <typename> typename V, typename X, typename Y, template <typename> typename Z>
inline void swap(Ref<T, U, V>& a, Ref<X, Y, Z>& b) requires(!(std::is_same_v<U, RawPtrTraits<T>> && std::is_same_v<Y, RawPtrTraits<X>>))
{
    a.swap(b);
}

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline Ref<T, U, V> Ref<T, U, V>::replace(Ref<X, Y, Z>&& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    /*WTFLogAlways("Ref::replace called.");*/
    auto oldReference = adoptRef(*m_ptr);
    m_ptr = &reference.leakRef();
    return oldReference;
}

template<typename T, typename U = RawPtrTraits<T>, template <typename> typename V = RefDerefTraits, typename X, typename Y, template <typename> typename Z>
inline Ref<T, U, V> static_reference_cast(Ref<X, Y, Z>&& reference)
{
    if constexpr (RefTrackingSmartPtr<Ref<T, U, V>> && RefTrackingSmartPtr<Ref<X, Y, Z>>) {
        auto token = reference.refTrackingToken();
        return Ref<T, U, V>::takeRef(static_cast<T&>(reference.leakRef()), token);
    } else
        return adoptRef(static_cast<T&>(reference.leakRef()));
}

template<typename T, typename U = RawPtrTraits<T>, template <typename> typename V = RefDerefTraits, typename X, typename Y, template <typename> typename Z>
ALWAYS_INLINE Ref<T, U, V> static_reference_cast(const Ref<X, Y, Z>& reference)
{
    return static_reference_cast<T, U, V>(reference.copyRef());
}

template <typename T, typename U, template <typename> typename V>
struct GetPtrHelper<Ref<T, U, V>> {
    typedef T* PtrType;
    static T* getPtr(const Ref<T, U, V>& p) { return const_cast<T*>(p.ptr()); }
};

template <typename T, typename U, template <typename> typename V>
struct IsSmartPtr<Ref<T, U, V>> {
    static constexpr bool value = true;
};

template<typename T, typename U, template <typename> typename V>
inline Ref<T, U, V> adoptRef(T& reference)
{
    /*WTFLogAlways("Ref adoptRef.");*/
    adopted(&reference);
    return Ref<T, U, V>(reference, Ref<T, U, V>::Adopt);
}

template<typename ExpectedType, typename ArgType, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg>
inline bool is(const Ref<ArgType, PtrTraitsArg, RefDerefTraitsArg>& source)
{
    return is<ExpectedType>(source.get());
}

template<typename Target, typename Source, typename TargetPtrTraits = RawPtrTraits<Target>, template <typename> typename TargetRefDerefTraits = RefDerefTraits, typename SourcePtrTraits, template <typename> typename SourceRefDerefTraits>
inline Ref<Target> downcast(Ref<Source, SourcePtrTraits, SourceRefDerefTraits> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    ASSERT_WITH_SECURITY_IMPLICATION(is<Target>(source));
    return static_reference_cast<Target>(WTFMove(source));
}

template <typename Target, typename Source, typename TargetPtrTraits = RawPtrTraits<Target>, template <typename> typename TargetRefDerefTraits = RefDerefTraits, typename SourcePtrTraits, template <typename> typename SourceRefDerefTraits>
inline RefPtr<Target, TargetPtrTraits, TargetRefDerefTraits> dynamicDowncast(Ref<Source, SourcePtrTraits, SourceRefDerefTraits> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    if (!is<Target>(source))
        return nullptr;
    return static_reference_cast<Target, TargetPtrTraits, TargetRefDerefTraits>(WTFMove(source));
}

} // namespace WTF

using WTF::Ref;
using WTF::adoptRef;
using WTF::static_reference_cast;
