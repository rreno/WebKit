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
#include <wtf/RefTrackingToken.h>
#include <wtf/StdLibExtras.h>
#include <wtf/TypeCasts.h>

#if ASAN_ENABLED
extern "C" void __asan_poison_memory_region(void const volatile *addr, size_t size);
extern "C" void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
extern "C" int __asan_address_is_poisoned(void const volatile *addr);
#endif

namespace WTF {

inline void adopted(const void*) { }

template<typename T, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg> class Ref;
template<typename T, typename PtrTraitsArg = RawPtrTraits<T>, template <typename> typename RefDerefTraitsArg = RefDerefTraits> Ref<T, PtrTraitsArg, RefDerefTraitsArg> adoptRef(T&);
template<typename T, typename PtrTraitsArg = RawPtrTraits<T>, template <typename> typename RefDerefTraitsArg = RefDerefTraits> Ref<T, PtrTraitsArg, RefDerefTraitsArg> adoptRef(T&, RefTrackingToken);

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
        RefDerefBase::derefIfNotNull(PtrTraits::exchange(m_ptr, nullptr));
    }

    Ref(T& object) : m_ptr(&object) { RefDerefBase::ref(object); }
    Ref(const Ref& other) : m_ptr(other.ptr()) { ASSERT(m_ptr); RefDerefBase::ref(*PtrTraits::unwrap(m_ptr)); }

    template<typename X, typename Y, template <typename> typename Z>
    Ref(const Ref<X, Y, Z>& other)
        : m_ptr(other.ptr())
    {
        ASSERT(m_ptr);
        RefDerefBase::ref(*PtrTraits::unwrap(m_ptr));
    }

    // In ASAN builds Ref will poison itself in leakRef. We need to take the ref tracking token first (if applicable)
    // in these move constructors.
    Ref(Ref&& other)
    {
        RefDerefBase::moveRef(other);
        m_ptr = &other.leakRef();
        ASSERT(m_ptr);
    }

    template<typename X, typename Y, template <typename> typename Z>
    Ref(Ref<X, Y, Z>&& other)
    {
        RefDerefBase::moveRef(other);
        m_ptr = &other.leakRef();
        ASSERT(m_ptr);
    }

    Ref(T& object, RefTrackingToken token) requires isRefTracking
        : m_ptr(&object)
    {
        refTrackingToken() = token;
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

    const RefTrackingToken& refTrackingToken() const requires isRefTracking { return RefDerefBase::refTrackingToken(); }
    RefTrackingToken& refTrackingToken() requires isRefTracking { return RefDerefBase::refTrackingToken(); }

private:
    friend Ref adoptRef<T>(T&);

    friend Ref adoptRef<T>(T&, RefTrackingToken);
    template<typename X, typename Y, template <typename> typename Z> friend class Ref;
    template<typename X, typename Y, template <typename> typename Z> friend class RefPtr;

    template<typename X, typename Y, template <typename> typename Z, typename U, typename V, template <typename> typename W>
    friend bool operator==(const Ref<X, Y, Z>&, const Ref<U, V, W>&);

    enum AdoptTag { Adopt };
    Ref(T& object, AdoptTag)
        : m_ptr(&object)
    {
        RefDerefBase::adoptRef(&object);
    }

    Ref(T& object, RefTrackingToken token, AdoptTag) requires isRefTracking
        : m_ptr(&object)
    {
        refTrackingToken() = token;
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

template<typename T, typename _PtrTraits, template <typename> typename _RefDerefTraits>
template<typename U, typename _OtherPtrTraits, template <typename> typename _OtherRefDerefTraits>
inline Ref<T, _PtrTraits, _RefDerefTraits>& Ref<T, _PtrTraits, _RefDerefTraits>::operator=(Ref<U, _OtherPtrTraits, _OtherRefDerefTraits>&& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref movedReference = WTFMove(reference);
    swap(movedReference);
    return *this;
}

template<typename T, typename _PtrTraits, template <typename> typename _RefDerefTraits>
inline Ref<T, _PtrTraits, _RefDerefTraits>& Ref<T, _PtrTraits, _RefDerefTraits>::operator=(const Ref& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
}

template<typename T, typename _PtrTraits, template <typename> typename _RefDerefTraits>
template<typename U, typename _OtherPtrTraits, template <typename> typename _OtherRefDerefTraits>
inline Ref<T, _PtrTraits, _RefDerefTraits>& Ref<T, _PtrTraits, _RefDerefTraits>::operator=(const Ref<U, _OtherPtrTraits, _OtherRefDerefTraits>& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    Ref copiedReference = reference;
    swap(copiedReference);
    return *this;
}

template<typename X, typename APtrTraits, template <typename> typename ARefDerefTraits, typename Y, typename BPtrTraits, template <typename> typename BRefDerefTraits>
inline bool operator==(const Ref<X, APtrTraits, ARefDerefTraits>& a, const Ref<Y, BPtrTraits, BRefDerefTraits>& b)
{
    if constexpr (Ref<X, APtrTraits, ARefDerefTraits>::isRefTracking || Ref<Y, BPtrTraits, BRefDerefTraits>::isRefTracking)
        return a.m_ptr == b.m_ptr && a.refTrackingToken() == b.refTrackingToken();
    return a.m_ptr == b.m_ptr;
}

template<typename X, typename _PtrTraits, template <typename> typename _RefDerefTraits>
template<typename Y, typename _OtherPtrTraits, template <typename> typename _OtherRefDerefTraits>
inline void Ref<X, _PtrTraits, _RefDerefTraits>::swap(Ref<Y, _OtherPtrTraits, _OtherRefDerefTraits>& other)
{
    _PtrTraits::swap(m_ptr, other.m_ptr);
    RefDerefBase::swapRef(other);
}

template<typename X, typename APtrTraits, template <typename> typename ARefDerefTraits, typename Y, typename BPtrTraits, template <typename> typename BRefDerefTraits>
inline void swap(Ref<X, APtrTraits, ARefDerefTraits>& a, Ref<Y, BPtrTraits, BRefDerefTraits>& b) requires(!(std::is_same_v<APtrTraits, RawPtrTraits<X>> && std::is_same_v<BPtrTraits, RawPtrTraits<Y>>))
{
    a.swap(b);
}

template<typename X, typename _PtrTraits, template <typename> typename _RefDerefTraits>
template<typename Y, typename _OtherPtrTraits, template <typename> typename _OtherRefDerefTraits>
inline Ref<X, _PtrTraits, _RefDerefTraits> Ref<X, _PtrTraits, _RefDerefTraits>::replace(Ref<Y, _OtherPtrTraits, _OtherRefDerefTraits>&& reference)
{
#if ASAN_ENABLED
    if (__asan_address_is_poisoned(this))
        __asan_unpoison_memory_region(this, sizeof(*this));
#endif
    auto oldReference = adoptRef(*m_ptr);
    m_ptr = &reference.leakRef();
    return oldReference;
}

template<typename X, typename _PtrTraits = RawPtrTraits<X>, template <typename> typename _RefDerefTraits = RefDerefTraits, typename Y, typename _OtherPtrTraits, template <typename> typename _OtherRefDerefTraits>
inline Ref<X, _PtrTraits, _RefDerefTraits> static_reference_cast(Ref<Y, _OtherPtrTraits, _OtherRefDerefTraits>&& reference)
{
    if constexpr (Ref<X, _PtrTraits, _RefDerefTraits>::isRefTracking && Ref<Y, _OtherPtrTraits, _OtherRefDerefTraits>::isRefTracking) {
        auto token = reference.refTrackingToken();
        return Ref<X, _PtrTraits, _RefDerefTraits>(static_cast<X&>(reference.leakRef()), token);
    } else
        return adoptRef(static_cast<X&>(reference.leakRef()));
}

template<typename X, typename _PtrTraits = RawPtrTraits<X>, template <typename> typename _RefDerefTraits = RefDerefTraits, typename Y, typename _OtherPtrTraits, template <typename> typename _OtherRefDerefTraits>
ALWAYS_INLINE Ref<X, _PtrTraits, _RefDerefTraits> static_reference_cast(const Ref<Y, _OtherPtrTraits, _OtherRefDerefTraits>& reference)
{
    return static_reference_cast<X, _PtrTraits, _RefDerefTraits>(reference.copyRef());
}

template <typename T, typename _PtrTraits, template <typename> typename _RefDerefTraits>
struct GetPtrHelper<Ref<T, _PtrTraits, _RefDerefTraits>> {
    using PtrType = T*;
    using UnderlyingType = T;
    static T* getPtr(const Ref<T, _PtrTraits, _RefDerefTraits>& p) { return const_cast<T*>(p.ptr()); }
};


template <typename T, typename _PtrTraits, template <typename> typename _RefDerefTraits>
struct IsSmartPtr<Ref<T, _PtrTraits, _RefDerefTraits>> {
    static constexpr bool value = true;
    static constexpr bool isNullable = false;
};

template<typename T, typename _PtrTraits, template <typename> typename _RefDerefTraits>
inline Ref<T, _PtrTraits, _RefDerefTraits> adoptRef(T& reference)
{
    adopted(&reference);
    return Ref<T, _PtrTraits, _RefDerefTraits>(reference, Ref<T, _PtrTraits, _RefDerefTraits>::Adopt);
}

template<typename T, typename _PtrTraits, template <typename> typename _RefDerefTraits>
inline Ref<T, _PtrTraits, _RefDerefTraits> adoptRef(T& reference, RefTrackingToken token)
{
    adopted(&reference);
    return Ref<T, _PtrTraits, _RefDerefTraits>(reference, token, Ref<T, _PtrTraits, _RefDerefTraits>::Adopt);
}

template<typename ExpectedType, typename ArgType, typename PtrTraits, template <typename> typename _RefDerefTraits>
inline bool is(const Ref<ArgType, PtrTraits, _RefDerefTraits>& source)
{
    return is<ExpectedType>(source.get());
}

template<typename Target, typename Source, typename PtrTraits, template <typename> typename _RefDerefTraits>
inline Ref<match_constness_t<Source, Target>> uncheckedDowncast(Ref<Source, PtrTraits, _RefDerefTraits> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    ASSERT_WITH_SECURITY_IMPLICATION(is<Target>(source));
    return static_reference_cast<match_constness_t<Source, Target>>(WTFMove(source));
}

template<typename Target, typename Source, typename PtrTraits, template <typename> typename _RefDerefTraits>
inline Ref<match_constness_t<Source, Target>> downcast(Ref<Source, PtrTraits, _RefDerefTraits> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    RELEASE_ASSERT(is<Target>(source));
    return static_reference_cast<match_constness_t<Source, Target>>(WTFMove(source));
}

template<typename Target, typename Source, typename PtrTraits, template <typename> typename _RefDerefTraits>
inline RefPtr<match_constness_t<Source, Target>> dynamicDowncast(Ref<Source, PtrTraits, _RefDerefTraits> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    if (!is<Target>(source))
        return nullptr;
    return static_reference_cast<match_constness_t<Source, Target>>(WTFMove(source));
}

} // namespace WTF

using WTF::Ref;
using WTF::adoptRef;
using WTF::static_reference_cast;
