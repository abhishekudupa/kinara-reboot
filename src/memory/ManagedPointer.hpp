// ManagedPointer.hpp ---
//
// Filename: ManagedPointer.hpp
// Author: Abhishek Udupa
// Created: Sun Jun 29 14:10:50 2014 (-0400)
//
//
// Copyright (c) 2015, Abhishek Udupa, University of Pennsylvania
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by The University of Pennsylvania
// 4. Neither the name of the University of Pennsylvania nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//

// Code:

#if !defined KINARA_MEMORY_MANAGED_POINTER_HPP_
#define KINARA_MEMORY_MANAGED_POINTER_HPP_

#include <type_traits>

#include "../common/KinaraBase.hpp"

namespace kinara {
namespace memory {

namespace detail {

namespace kmd = kinara::memory::detail;

template <typename T, bool ISCONSTPTR>
class ManagedPointerBase final
{
private:
    typedef typename std::conditional<ISCONSTREF, const T*, T*>::type RawPointerType;
    typedef typename std::conditional<ISCONSTREF, const T&, T&>::type ReferenceType;

    RawPointerType m_ptr;

    template <typename U, bool OTHERISCONSTREF>
    inline i64 compare_(const kmd::ManagedPointerBase<U, OTHERISCONSTREF>& other_managed_ptr);

    template <typename U>
    inline i64 compare_(const U* other_ptr) const;

public:
    static const ManagedPointerBase null_pointer;

    inline ManagedPointerBase();

    // Copy constructor
    template <bool OTHERISCONSTREF>
    inline ManagedPointerBase(const kmd::ManagedPointerBase<T, OTHERISCONSTREF>& other);

    // Move constructor
    template <bool OTHERISCONSTREF>
    inline ManagedPointerBase(kmd::ManagedPointerBase<T, OTHERISCONSTREF>&& other);

    // From raw pointer
    inline ManagedPointerBase(RawPointerType* raw_pointer);

    // Destructor
    inline ~ManagedPointer();

    // Accessor
    inline RawPointerType* get_raw_pointer() const;

    // cast to raw pointer
    inline operator RawPointerType* () const;

    // Assignment operator
    template <bool OTHERISCONSTREF>
    inline ManagedPointer& operator = (const kmd::ManagedPointerBase<T, OTHERISCONSTREF>& other);

    // Move assignment operator
    template <bool OTHERISCONSTREF>
    inline ManagedPointer& operator = (kmd::ManagedPointerBase<T, OTHERISCONSTREF>&& other);

    // Assignment to raw pointer
    inline ManagedPointer& operator = (RawPointerType* raw_pointer);

    inline RawPointerType* operator -> () const;
    inline ReferenceType operator * () const;

    template <typename U>
    inline bool operator == (const U& other) const;
    template <typename U>
    inline bool operator != (const U& other) const;
    template<typename U>
    inline bool operator < (const U& other) const;
    template <typename U>
    inline bool operator <= (const U& other) const;
    template <typename U>
    inline bool operator > (const U& other) const;
    template <typename U>
    inline bool operator >= (const U& other) const;

    inline bool operator == (const T* other_pointer) const;
    inline bool operator != (const T* other_pointer) const;
    inline bool operator < (const T* other_pointer) const;
    inline bool operator <= (const T* other_pointer) const;
    inline bool operator > (const T* other_pointer) const;
    inline bool operator >= (const T* other_pointer) const;

    inline bool operator ! () const;
    inline bool is_null_() const;
};

// Implementation of ManagedPointerBase
template <typename T, bool ISCONSTREF>
const ManagedPointerBase<T, ISCONSTREF> ManagedPointerBase<T, ISCONSTREF>::null_pointer;

template <typename T, bool ISCONSTREF>
template <typename U, bool OTHERISCONSTREF>
inline i64
ManagedPointerBase<T, ISCONSTREF>::compare_(const kmd::ManagedPointerBase<U, OTHERISCONSTREF>&
                                            other_managed_ptr)
{
    return (i64)((char*)m_ptr - (char*)other_managed_ptr.m_ptr);
}

template <typename T, bool ISCONSTREF>
template <typename U>
inline i64 ManagedPointerBase<T, ISCONSTREF>::compare_(const U* other_ptr) const
{
    return (i64)((char*)m_ptr - (char*)other_ptr);
}

template <typename T, bool ISCONSTREF>
inline ManagedPointerBase<T, ISCONSTREF>::ManagedPointer()
    : Ptr_(nullptr)
{
    // Nothing here
}

template <typename T>
inline ManagedPointer<T>::ManagedPointer(const ManagedPointer<T>& Other)
    : Ptr_(nullptr)
{
    Ptr_ = Other.Ptr_;
    if (Ptr_ != nullptr) {
        Ptr_->IncRef_();
    }
}

template <typename T>
inline ManagedPointer<T>::ManagedPointer(ManagedPointer<T>&& Other)
    : ManagedPointer<T>()
{
    swap(Ptr_, Other.Ptr_);
}

template <typename T>
inline ManagedPointer<T>::ManagedPointer(T* OtherPtr)
    : Ptr_(OtherPtr)
{
    if (Ptr_ != nullptr) {
        Ptr_->IncRef_();
    }
}

template <typename T>
inline ManagedPointer<T>::~ManagedPointer()
{
    if (Ptr_ != nullptr) {
        Ptr_->DecRef_();
    }
    Ptr_ = nullptr;
}

template <typename T>
inline T* ManagedPointer<T>::GetPtr_() const
{
    return Ptr_;
}

template <typename T>
inline ManagedPointer<T>::operator T* () const
{
    return (GetPtr_());
}

template <typename T>
inline ManagedPointer<T>& ManagedPointer<T>::operator = (ManagedPointer<T> Other)
{
    swap(Ptr_, Other.Ptr_);
    return (*this);
}

template <typename T>
inline ManagedPointer<T>& ManagedPointer<T>::operator = (T* OtherPtr)
{
    ManagedPointer<T> Dummy(OtherPtr);
    swap(Ptr_, Dummy.Ptr_);
    return (*this);
}

template <typename T>
inline T* ManagedPointer<T>::operator -> () const
{
    return (GetPtr_());
}

template <typename T>
inline T& ManagedPointer<T>::operator * () const
{
    return (*Ptr_);
}

template <typename T>
template <typename U>
inline bool ManagedPointer<T>::operator == (const U& Other) const
{
    return (Compare_(Other) == 0);
}

template <typename T>
template <typename U>
inline bool ManagedPointer<T>::operator != (const U& Other) const
{
    return (Compare_(Other) != 0);
}

template <typename T>
template <typename U>
inline bool ManagedPointer<T>::operator < (const U& Other) const
{
    return (Compare_(Other) < 0);
}

template <typename T>
template <typename U>
inline bool ManagedPointer<T>::operator <= (const U& Other) const
{
    return (Compare_(Other) <= 0);
}

template <typename T>
template <typename U>
inline bool ManagedPointer<T>::operator > (const U& Other) const
{
    return (Compare_(Other) > 0);
}

template <typename T>
template <typename U>
inline bool ManagedPointer<T>::operator >= (const U& Other) const
{
    return (Compare_(Other) >= 0);
}

template <typename T>
inline bool ManagedPointer<T>::operator == (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) == 0);
}

template <typename T>
inline bool ManagedPointer<T>::operator != (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) != 0);
}

template <typename T>
inline bool ManagedPointer<T>::operator < (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) < 0);
}

template <typename T>
inline bool ManagedPointer<T>::operator <= (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) <= 0);
}

template <typename T>
inline bool ManagedPointer<T>::operator > (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) > 0);
}

template <typename T>
inline bool ManagedPointer<T>::operator >= (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) >= 0);
}

template <typename T>
inline bool ManagedPointer<T>::IsNull_() const
{
    return (Ptr_ == nullptr);
}

template <typename T>
inline bool ManagedPointer<T>::operator ! () const
{
    return (IsNull_());
}

// implementation of ManagedConstPointer

template<typename T>
const ManagedConstPointer<T> ManagedConstPointer<T>::NullPtr;

template <typename T>
template <typename U>
inline i64 ManagedConstPointer<T>::Compare_(const U &Other) const
{
    return (i64)((char*)Ptr_ - (char*)Other.Ptr_);
}

template <typename T>
inline i64 ManagedConstPointer<T>::Compare_(const T *OtherPtr) const
{
    return (i64)((char*)Ptr_ - (char*)OtherPtr);
}

template <typename T>
inline ManagedConstPointer<T>::ManagedConstPointer()
    : Ptr_(nullptr)
{
    // Nothing here
}

template <typename T>
inline ManagedConstPointer<T>::ManagedConstPointer(const ManagedConstPointer<T>& Other)
    : Ptr_(nullptr)
{
    Ptr_ = Other.Ptr_;
    if (Ptr_ != nullptr) {
        Ptr_->IncRef_();
    }
}

template <typename T>
inline ManagedConstPointer<T>::ManagedConstPointer(ManagedConstPointer<T>&& Other)
    : ManagedConstPointer<T>()
{
    swap(Ptr_, Other.Ptr_);
}

template <typename T>
inline ManagedConstPointer<T>::ManagedConstPointer(const ManagedPointer<T>& Other)
    : Ptr_(nullptr)
{
    Ptr_ = Other.Ptr_;
    if (Ptr_ != nullptr) {
        Ptr_->IncRef_();
    }
}

template <typename T>
inline ManagedConstPointer<T>::ManagedConstPointer(ManagedPointer<T>&& Other)
    : ManagedConstPointer()
{
    swap(const_cast<T*>(Ptr_), Other.Ptr_);
}

template <typename T>
inline ManagedConstPointer<T>::ManagedConstPointer(const T* OtherPtr)
    : Ptr_(nullptr)
{
    Ptr_ = OtherPtr;
    if (Ptr_ != nullptr) {
        Ptr_->IncRef_();
    }
}

template <typename T>
inline ManagedConstPointer<T>::~ManagedConstPointer()
{
    if (Ptr_ != nullptr) {
        Ptr_->DecRef_();
    }
    Ptr_ = nullptr;
}

template <typename T>
inline ManagedConstPointer<T>& ManagedConstPointer<T>::operator = (ManagedConstPointer<T> Other)
{
    swap(Ptr_, Other.Ptr_);
    return (*this);
}

template <typename T>
inline ManagedConstPointer<T>& ManagedConstPointer<T>::operator = (ManagedPointer<T> Other)
{
    T* TempPtr = const_cast<T*>(Ptr_);
    swap(TempPtr, Other.Ptr_);
    Ptr_ = TempPtr;
    return (*this);
}

template <typename T>
inline ManagedConstPointer<T>& ManagedConstPointer<T>::operator = (const T* OtherPtr)
{
    ManagedConstPointer<T> Dummy(OtherPtr);
    swap(Ptr_, Dummy.Ptr_);
    return (*this);
}

template <typename T>
inline const T* ManagedConstPointer<T>::GetPtr_() const
{
    return Ptr_;
}

template <typename T>
inline ManagedConstPointer<T>::operator const T* () const
{
    return (GetPtr_());
}

template <typename T>
inline const T* ManagedConstPointer<T>::operator -> () const
{
    return (GetPtr_());
}

template <typename T>
inline const T& ManagedConstPointer<T>::operator * () const
{
    return (*Ptr_);
}

template <typename T>
template <typename U>
inline bool ManagedConstPointer<T>::operator == (const U& Other) const
{
    return (Compare_(Other) == 0);
}

template <typename T>
template <typename U>
inline bool ManagedConstPointer<T>::operator != (const U& Other) const
{
    return (Compare_(Other) != 0);
}

template <typename T>
template <typename U>
inline bool ManagedConstPointer<T>::operator < (const U& Other) const
{
    return (Compare_(Other) < 0);
}

template <typename T>
template <typename U>
inline bool ManagedConstPointer<T>::operator <= (const U& Other) const
{
    return (Compare_(Other) <= 0);
}

template <typename T>
template <typename U>
inline bool ManagedConstPointer<T>::operator > (const U& Other) const
{
    return (Compare_(Other) > 0);
}

template <typename T>
template <typename U>
inline bool ManagedConstPointer<T>::operator >= (const U& Other) const
{
    return (Compare_(Other) >= 0);
}

template <typename T>
inline bool ManagedConstPointer<T>::operator == (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) == 0);
}

template <typename T>
inline bool ManagedConstPointer<T>::operator != (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) != 0);
}

template <typename T>
inline bool ManagedConstPointer<T>::operator < (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) < 0);
}

template <typename T>
inline bool ManagedConstPointer<T>::operator <= (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) <= 0);
}

template <typename T>
inline bool ManagedConstPointer<T>::operator > (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) > 0);
}

template <typename T>
inline bool ManagedConstPointer<T>::operator >= (const T* OtherPtr) const
{
    return (Compare_(OtherPtr) >= 0);
}

template <typename T>
inline bool ManagedConstPointer<T>::IsNull_() const
{
    return (Ptr_ == nullptr);
}

template <typename T>
inline bool ManagedConstPointer<T>::operator ! () const
{
    return (IsNull_());
}

template <typename T>
static inline ManagedPointer<T> ConstCast(const ManagedConstPointer<T>& CPtr)
{
    return ManagedPointer<T>(const_cast<T*>(CPtr->GetPtr_()));
}

template <typename T>
static inline void PrintManagedPointer_(ostream& Out, const ManagedPointer<T>& Ptr,
                                  const false_type& Ununsed)
{
    Out << Ptr->GetPtr_();
}

template <typename T>
static inline void PrintManagedPointer_(ostream& Out, const ManagedPointer<T>& Ptr,
                                  const true_type& Unused)
{
    Out << Ptr->ToString();
}

template <typename T>
static inline void PrintManagedPointer_(ostream& Out, const ManagedConstPointer<T>& Ptr,
                                  const false_type& Ununsed)
{
    Out << Ptr->GetPtr_();
}

template <typename T>
static inline void PrintManagedPointer_(ostream& Out, const ManagedConstPointer<T>& Ptr,
                                  const true_type& Unused)
{
    Out << Ptr->ToString();
}


template <typename T>
static inline ostream& operator << (ostream& Out, const ManagedPointer<T>& Ptr)
{
    typedef typename is_base_of<ESMC::Stringifiable, T>::type StringifiableType;
    PrintManagedPointer_(Out, Ptr, StringifiableType());
    return Out;
}

template <typename T>
static inline ostream& operator << (ostream& Out, const ManagedConstPointer<T>& Ptr)
{
    typedef typename is_base_of<ESMC::Stringifiable, T>::type StringifiableType;
    PrintManagedPointer_(Out, Ptr, StringifiableType());
    return Out;
}

} /* end namespace memory */
} /* end namespace kinara */

#endif /* KINARA_MEMORY_MANAGED_POINTER_HPP_ */

//
// ManagedPointer.hpp ends here
