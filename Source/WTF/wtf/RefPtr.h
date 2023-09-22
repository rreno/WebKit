/*
 *  Copyright (C) 2005-2023 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

// RefPtr is documented at http://webkit.org/coding/RefPtr.html

#pragma once

#include <algorithm>
#include <utility>
#include <wtf/FastMalloc.h>
#include <wtf/Ref.h>
#include <wtf/RefDerefTraits.h>

namespace WTF {

template<typename T, typename PtrTraits, template <typename> typename RefDerefTraitsArg> class RefPtr;
template<typename T, typename PtrTraits = RawPtrTraits<T>, template <typename> typename RefDerefTraitsArg = RefDerefTraits> RefPtr<T, PtrTraits, RefDerefTraitsArg> adoptRef(T*);

template<typename T, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg>
class RefPtr : private RefDerefTraitsArg<T> {
    WTF_MAKE_FAST_ALLOCATED;
public:
    using PtrTraits = PtrTraitsArg;
    using RefDerefBase = RefDerefTraitsArg<T>;
    typedef T ValueType;
    typedef ValueType* PtrType;

    static constexpr bool isRefPtr = true;
    static constexpr bool isRefTracking = RefTracked<T>;

    ALWAYS_INLINE constexpr RefPtr() : m_ptr(nullptr) { }
    ALWAYS_INLINE constexpr RefPtr(std::nullptr_t) : m_ptr(nullptr) { }

    ALWAYS_INLINE RefPtr(T* ptr) : m_ptr(ptr) { RefDerefBase::refIfNotNull(PtrTraits::unwrap(m_ptr)); }
    ALWAYS_INLINE RefPtr(const RefPtr& o) : m_ptr(o.m_ptr) { RefDerefBase::refIfNotNull(PtrTraits::unwrap(m_ptr)); }

    template<typename X, typename Y, template <typename> typename Z> RefPtr(const RefPtr<X, Y, Z>& o) : m_ptr(o.get()) { RefDerefBase::refIfNotNull(PtrTraits::unwrap(m_ptr)); }

    ALWAYS_INLINE RefPtr(RefPtr&& o) : m_ptr(o.leakRef()) { RefDerefBase::moveRef(o); }
    template<typename X, typename Y, template <typename> typename Z> RefPtr(RefPtr<X, Y, Z>&& o) { RefDerefBase::moveRef(o); m_ptr = o.leakRef(); }

    template <typename X, typename Y, template <typename> typename Z> RefPtr(Ref<X, Y, Z>&&);

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    RefPtr(HashTableDeletedValueType) : m_ptr(PtrTraits::hashTableDeletedValue()) { }
    bool isHashTableDeletedValue() const { return PtrTraits::isHashTableDeletedValue(m_ptr); }

    ALWAYS_INLINE ~RefPtr() { RefDerefBase::derefIfNotNull(PtrTraits::exchange(m_ptr, nullptr)); }

    T* get() const { return PtrTraits::unwrap(m_ptr); }
    // Redundant but symmetrical with Ref::ptr which is helpful for writing generic code over both smart pointers.
    T* ptr() const { return get(); }

    Ref<T> releaseNonNull()
    {
        ASSERT(m_ptr);
        if constexpr (isRefTracking)
            return adoptRef(*PtrTraits::exchange(m_ptr, nullptr), std::exchange(refTrackingToken(), UntrackedRefToken()));
        else
            return adoptRef(*PtrTraits::exchange(m_ptr, nullptr));
    }

    T* leakRef() WARN_UNUSED_RETURN;

    T& operator*() const { ASSERT(m_ptr); return *PtrTraits::unwrap(m_ptr); }
    ALWAYS_INLINE T* operator->() const { return PtrTraits::unwrap(m_ptr); }

    bool operator!() const { return !m_ptr; }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    using UnspecifiedBoolType = void (RefPtr::*)() const;
    operator UnspecifiedBoolType() const { return m_ptr ? &RefPtr::unspecifiedBoolTypeInstance : nullptr; }

    explicit operator bool() const { return !!m_ptr; }
    
    RefPtr& operator=(const RefPtr&);
    RefPtr& operator=(T*);
    RefPtr& operator=(std::nullptr_t);
    template<typename X, typename Y, template <typename> typename Z> RefPtr& operator=(const RefPtr<X, Y, Z>&);
    RefPtr& operator=(RefPtr&&);
    template<typename X, typename Y, template <typename> typename Z> RefPtr& operator=(RefPtr<X, Y, Z>&&);
    template<typename X> RefPtr& operator=(Ref<X>&&);

    template<typename X, typename Y, template <typename> typename Z> void swap(RefPtr<X, Y, Z>&);

    RefPtr copyRef() && = delete;
    RefPtr copyRef() const & WARN_UNUSED_RETURN { return RefPtr(m_ptr); }

    const RefTrackingToken& refTrackingToken() const requires isRefTracking { return RefDerefBase::refTrackingToken(); }
    RefTrackingToken& refTrackingToken() requires isRefTracking { return RefDerefBase::refTrackingToken(); }

    static RefPtr takeRef(T* ptr, RefTrackingToken token) requires isRefTracking
    {
        adopted(ptr);
        return RefPtr(ptr, token);
    }

private:
    void unspecifiedBoolTypeInstance() const { }

    friend RefPtr adoptRef<T>(T*);
    template<typename X, typename Y, template <typename> typename Z> friend class RefPtr;

    template<typename T1, typename U, template <typename> typename V, typename X, typename Y, template <typename> typename Z>
    friend bool operator==(const RefPtr<T1, U, V>&, const RefPtr<X, Y, Z>&);
    template<typename T1, typename U, template <typename> typename V, typename X>
    friend bool operator==(const RefPtr<T1, U, V>&, X*);
    template<typename T1, typename X, typename Y, template <typename> typename Z>
    friend bool operator==(T1*, const RefPtr<X, Y, Z>&);

    enum AdoptTag { Adopt };
    RefPtr(T* ptr, AdoptTag)
        : m_ptr(ptr)
    {
        RefDerefBase::adoptRef(PtrTraits::unwrap(m_ptr));
    }

    RefPtr(T* ptr, RefTrackingToken token) requires isRefTracking
        : m_ptr(ptr)
    {
        refTrackingToken() = token;
    }

    typename PtrTraits::StorageType m_ptr;
};

// Template deduction guide.
template<typename X, typename Y, template <typename> typename Z> RefPtr(Ref<X, Y, Z>&&) -> RefPtr<X, Y, Z>;

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline RefPtr<T, U, V>::RefPtr(Ref<X, Y, Z>&& reference)
{
    RefDerefBase::moveRef(reference);
    m_ptr = &reference.leakRef();
}

template<typename T, typename U, template <typename> typename V>
inline T* RefPtr<T, U, V>::leakRef()
{
    return U::exchange(m_ptr, nullptr);
}

template<typename T, typename U, template <typename> typename V>
inline RefPtr<T, U, V>& RefPtr<T, U, V>::operator=(const RefPtr& o)
{
    RefPtr ptr = o;
    swap(ptr);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline RefPtr<T, U, V>& RefPtr<T, U, V>::operator=(const RefPtr<X, Y, Z>& o)
{
    RefPtr ptr = o;
    swap(ptr);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
inline RefPtr<T, U, V>& RefPtr<T, U, V>::operator=(T* optr)
{
    RefPtr ptr = optr;
    swap(ptr);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
inline RefPtr<T, U, V>& RefPtr<T, U, V>::operator=(std::nullptr_t)
{
    RefDerefBase::derefIfNotNull(U::exchange(m_ptr, nullptr));
    return *this;
}

template<typename T, typename U, template <typename> typename V>
inline RefPtr<T, U, V>& RefPtr<T, U, V>::operator=(RefPtr&& o)
{
    RefPtr ptr = WTFMove(o);
    swap(ptr);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline RefPtr<T, U, V>& RefPtr<T, U, V>::operator=(RefPtr<X, Y, Z>&& o)
{
    RefPtr ptr = WTFMove(o);
    swap(ptr);
    return *this;
}

template<typename T, typename V, template <typename> typename W>
template<typename U>
inline RefPtr<T, V, W>& RefPtr<T, V, W>::operator=(Ref<U>&& reference)
{
    RefPtr ptr = WTFMove(reference);
    swap(ptr);
    return *this;
}

template<typename T, typename U, template <typename> typename V>
template<typename X, typename Y, template <typename> typename Z>
inline void RefPtr<T, U, V>::swap(RefPtr<X, Y, Z>& o)
{
    U::swap(m_ptr, o.m_ptr);
    RefDerefBase::swapRef(o);
}

template<typename T, typename U, template <typename> typename V>
inline void swap(RefPtr<T, U, V>& a, RefPtr<T, U, V>& b)
{
    a.swap(b);
}

template<typename T, typename U, template <typename> typename V, typename X, typename Y, template <typename> typename Z>
inline bool operator==(const RefPtr<T, U, V>& a, const RefPtr<X, Y, Z>& b)
{
    return a.m_ptr == b.m_ptr;
}

template<typename T, typename U, template <typename> typename V, typename X>
inline bool operator==(const RefPtr<T, U, V>& a, X* b)
{
    return a.m_ptr == b;
}

template<typename T, typename X, typename Y, template <typename> typename Z>
inline bool operator==(T* a, const RefPtr<X, Y, Z>& b)
{
    return a == b.m_ptr;
}

template<typename T, typename U, template <typename> typename V, typename X, typename Y, template <typename> typename Z>
inline bool operator!=(const RefPtr<T, U, V>& a, const RefPtr<X, Y, Z>& b)
{
    return !(a == b);
}

template<typename T, typename U, template <typename> typename V, typename X>
inline bool operator!=(const RefPtr<T, U, V>& a, X* b)
{
    return !(a == b);
}

template<typename T, typename X, typename Y, template <typename> typename Z>
inline bool operator!=(T* a, const RefPtr<X, Y, Z>& b)
{
    return !(a == b);
}

template<typename T, typename U, template <typename> typename V>
inline RefPtr<T, U, V> adoptRef(T* p)
{
    adopted(p);
    return RefPtr<T, U, V>(p, RefPtr<T, U, V>::Adopt);
}

template<typename T, typename U = RawPtrTraits<T>, template <typename> typename V = RefDerefTraits, typename X, typename Y, template <typename> typename Z>
inline RefPtr<T, U, V> static_pointer_cast(const RefPtr<X, Y, Z>& p)
{ 
    return RefPtr<T, U, V>(static_cast<T*>(p.get()));
}

template<typename T, typename U = RawPtrTraits<T>, template <typename> typename V = RefDerefTraits, typename X, typename Y, template <typename> typename Z>
inline RefPtr<T, U, V> static_pointer_cast(RefPtr<X, Y, Z>&& p)
{
    if constexpr (RefPtr<T, U, V>::isRefTracking && RefPtr<X, Y, Z>::isRefTracking)
        return RefPtr<T, U, V>::takeRef(static_cast<T*>(p.leakRef()), p.refTrackingToken());
    else
        return adoptRef(static_cast<T*>(p.leakRef()));
}

template <typename T, typename U, template <typename> typename V>
struct IsSmartPtr<RefPtr<T, U, V>> {
    static constexpr bool value = true;
    static constexpr bool isNullable = true;
};

template<typename ExpectedType, typename ArgType, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg>
inline bool is(const RefPtr<ArgType, PtrTraitsArg, RefDerefTraitsArg>& source)
{
    return is<ExpectedType>(source.get());
}

template<typename Target, typename Source, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg>
inline RefPtr<match_constness_t<Source, Target>> uncheckedDowncast(RefPtr<Source, PtrTraitsArg, RefDerefTraitsArg> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    ASSERT_WITH_SECURITY_IMPLICATION(!source || is<Target>(*source));
    return static_pointer_cast<match_constness_t<Source, Target>>(WTFMove(source));
}

template<typename Target, typename Source, typename PtrTraitsArg, template <typename> typename RefDerefTraitsArg>
inline RefPtr<match_constness_t<Source, Target>> downcast(RefPtr<Source, PtrTraitsArg, RefDerefTraitsArg> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    RELEASE_ASSERT(!source || is<Target>(*source));
    return static_pointer_cast<match_constness_t<Source, Target>>(WTFMove(source));
}

template<typename Target, typename Source, typename TargetPtrTraits = RawPtrTraits<Target>, template <typename> typename TargetRefDerefTraits = RefDerefTraits,
    typename SourcePtrTraits, template <typename> typename SourceRefDerefTraits>
inline RefPtr<match_constness_t<Source, Target>, TargetPtrTraits, TargetRefDerefTraits> dynamicDowncast(RefPtr<Source, SourcePtrTraits, SourceRefDerefTraits> source)
{
    static_assert(!std::is_same_v<Source, Target>, "Unnecessary cast to same type");
    static_assert(std::is_base_of_v<Source, Target>, "Should be a downcast");
    if (!is<Target>(source))
        return nullptr;
    return static_pointer_cast<match_constness_t<Source, Target>, TargetPtrTraits, TargetRefDerefTraits>(WTFMove(source));
}

template<typename T, typename U>
ALWAYS_INLINE void lazyInitialize(const RefPtr<T>& ptr, Ref<U>&& obj)
{
    RELEASE_ASSERT(!ptr);
    const_cast<RefPtr<T>&>(ptr) = WTFMove(obj);
}

} // namespace WTF

using WTF::RefPtr;
using WTF::adoptRef;
using WTF::static_pointer_cast;
using WTF::lazyInitialize;
