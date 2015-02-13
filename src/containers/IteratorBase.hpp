// IteratorBase.hpp ---
//
// Filename: IteratorBase.hpp
// Author: Abhishek Udupa
// Created: Thu Feb 12 17:53:07 2015 (-0500)
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

#if !defined KINARA_CONTAINERS_ITERATOR_BASE_HPP_
#define KINARA_CONTAINERS_ITERATOR_BASE_HPP_

#include <exception>
#include <string>
#include <utility>

#include "../common/KinaraBase.hpp"

namespace kinara {
namespace containers {

template <typename Kind,
          typename T,
          typename PointerType = T*,
          typename ReferenceType = T&,
          typename DistType = u64>
class IteratorBase
{
public:
    typedef T ValueType;
    typedef PointerType PtrType;
    typedef ReferenceType RefType;
    typedef DistType DistanceType;
    typedef Kind IteratorKind;
};

// abstract base classes for various kinds of iterators
class InputIteratorKind
{

};

class OutputIteratorKind
{

};

class ForwardIteratorKind
{

};

class BidirectionalIteratorKind
{

};

class RandomAccessIteratorKind
{

};

class IteratorError : public std::exception
{
private:
    std::string m_exception_info;

public:
    IteratorError();
    IteratorError(const std::string& exception_info) noexcept;
    IteratorError(const IteratorError& other) noexcept;
    IteratorError(IteratorError&& other) noexcept;
    IteratorError& operator = (const IteratorError& other) noexcept;
    IteratorError& operator = (IteratorError&& other) noexcept;

    virtual ~IteratorError();
    virtual const char* what() const noexcept override;
};

// Some utility functions common to all containers
// which implement some form of iterable
template <typename IterableContainerType>
static inline typename IterableContainerType::iterator
begin(IterableContainerType& the_container)
{
    return the_container.begin();
}

template <typename IterableContainerType>
static inline typename IterableContainerType::const_iterator
begin(const IterableContainerType& the_container)
{
    return the_container.begin();
}

template <typename IterableContainerType>
static inline typename IterableContainerType::iterator
end(IterableContainerType& the_container)
{
    return the_container.end();
}

template <typename IterableContainerType>
static inline typename IterableContainerType::const_iterator
end(const IterableContainerType& the_container)
{
    return the_container.end();
}

template <typename ForwardIterator>
static inline ForwardIterator next(const ForwardIterator& current_iterator)
{
    auto retval = current_iterator;
    ++retval;
    return retval;
}

template <typename BidirectionalIterator>
static inline BidirectionalIterator prev(const BidirectionalIterator& current_iterator)
{
    auto retval = current_iterator;
    --retval;
    return retval;
}

template <typename RandomAccessIterator>
static inline u64 distance(const RandomAccessIterator& from,
                           const RandomAccessIterator& to,
                           const std::true_type& is_random_access_iterator_value)
{
    return (to - from);
}

template <typename ForwardIterator>
static inline u64 distance(const ForwardIterator& from,
                           const ForwardIterator& to,
                           const std::false_type& is_random_access_iterator_value)
{
    auto it = from;
    u64 retval = 0;
    while (it != to) {
        ++it;
        ++retval;
    }
    return retval;
}

template <typename RandomOrForwardIterator>
static inline u64 distance(const RandomOrForwardIterator& from,
                           const RandomOrForwardIterator& to)
{
    typename
        std::is_same<RandomAccessIteratorKind,
                     typename RandomOrForwardIterator::IteratorKind>::type
        is_random_access_iterator_value;

    return distance(from, to, is_random_access_iterator_value);
}

template <typename RandomAccessIterator>
static inline void advance(RandomAccessIterator& current_iterator, i64 n,
                           const std::true_type& is_random_access_iterator_value,
                           const std::false_type& is_bidirectional_iterator_value)
{
    current_iterator += n;
}

template <typename RandomAccessIterator>
static inline void advance(RandomAccessIterator& current_iterator, i64 n,
                           const std::true_type& is_random_access_iterator_value,
                           const std::true_type& is_bidirectional_iterator_value)
{
    current_iterator += n;
}

template <typename BidirectionalIterator>
static inline void advance_negative(BidirectionalIterator& current_iterator, i64 n,
                                    const std::true_type& is_bidirectional_iterator_value)
{
    i64 num_left = n;
    while(num_left > 0) {
        --current_iterator;
        --num_left;
    }
    return;
}

template <typename ForwardIterator>
static inline void advance_negative(ForwardIterator& current_iterator, i64 n,
                                    const std::false_type& is_bidirectional_iterator_value)
{
    throw IteratorError((std::string)"Cannot advance() with negative value on forward iterator");
}

template <typename BidirectionalIterator>
static inline void advance(BidirectionalIterator& current_iterator, i64 n,
                           const std::false_type& is_random_access_iterator_value,
                           const std::true_type& is_bidirectional_iterator_value)
{
    if (n < 0) {
        advance_negative(current_iterator, -n, is_bidirectional_iterator_value);
    } else {
        i64 num_left = n;
        while (num_left > 0) {
            ++current_iterator;
            --num_left;
        }
    }
}

template <typename BidirectionalIterator>
static inline void advance(BidirectionalIterator& current_iterator, i64 n,
                           const std::false_type& is_random_access_iterator_value,
                           const std::false_type& is_bidirectional_iterator_value)
{
    if (n < 0) {
        advance_negative(current_iterator, -n, is_bidirectional_iterator_value);
    } else {
        i64 num_left = n;
        while (num_left > 0) {
            ++current_iterator;
            --num_left;
        }
    }
}

template <typename InputIterator>
static inline void advance(InputIterator& current_iterator, i64 n)
{
    typename std::is_same<RandomAccessIteratorKind,
                          typename InputIterator::IteratorKind>::type
        is_random_access_iterator_value;
    typename std::is_same<BidirectionalIteratorKind,
                          typename InputIterator::IteratorKind>::type
        is_bidirectional_iterator_value;
    advance(current_iterator, n, is_random_access_iterator_value,
            is_bidirectional_iterator_value);
}

template <typename ContainerType>
class InsertIterator
    : public IteratorBase<OutputIteratorKind, void, void, void, void>
{
private:
    ContainerType& m_the_container;
    typename ContainerType::iterator m_the_iterator;

public:
    inline InsertIterator(ContainerType& the_container)
        : m_the_container(the_container),
          m_the_iterator(begin(the_container))
    {
        // Nothing here
    }

    inline InsertIterator(ContainerType& the_container,
                          const typename ContainerType::iterator& it)
        : m_the_container(the_container),
          m_the_iterator(it)
    {
        // Nothing here
    }

    inline ~InsertIterator()
    {
        // Nothing here
    }

    inline InsertIterator& operator = (const typename ContainerType::ValueType& the_value)
    {
        m_the_container.insert(m_the_iterator, the_value);
        return *this;
    }

    inline InsertIterator& operator = (typename ContainerType::ValueType&& the_value)
    {
        m_the_container.insert(m_the_iterator, std::move(the_value));
        return *this;
    }

    inline InsertIterator& operator * ()
    {
        return *this;
    }

    inline InsertIterator& operator ++ ()
    {
        return *this;
    }

    inline InsertIterator& operator ++ (int unused)
    {
        return *this;
    }
};

template <typename ContainerType>
class BackInsertIterator
    : public IteratorBase<OutputIteratorKind, void, void, void, void>
{
private:
    ContainerType& m_the_container;

public:
    inline BackInsertIterator(ContainerType& the_container)
        : m_the_container(the_container)
    {
        // Nothing here
    }

    inline ~BackInsertIterator()
    {
        // Nothing here
    }

    inline BackInsertIterator& operator = (const typename ContainerType::ValueType& the_value)
    {
        m_the_container.push_back(the_value);
        return *this;
    }

    inline BackInsertIterator& operator = (typename ContainerType::ValueType&& the_value)
    {
        m_the_container.push_back(std::move(the_value));
        return *this;
    }

    inline BackInsertIterator& operator * ()
    {
        return *this;
    }

    inline BackInsertIterator& operator ++ ()
    {
        return *this;
    }

    inline BackInsertIterator& operator ++ (int unused)
    {
        return *this;
    }
};

template <typename ContainerType>
class FrontInsertIterator
    : public IteratorBase<OutputIteratorKind, void, void, void, void>
{
private:
    ContainerType& m_the_container;

public:
    inline FrontInsertIterator(ContainerType& the_container)
        : m_the_container(the_container)
    {
        // Nothing here
    }

    inline ~FrontInsertIterator()
    {
        // Nothing here
    }

    inline FrontInsertIterator& operator = (const typename ContainerType::ValueType& the_value)
    {
        m_the_container.push_front(the_value);
        return *this;
    }

    inline FrontInsertIterator& operator = (typename ContainerType::ValueType&& the_value)
    {
        m_the_container.push_front(std::move(the_value));
        return *this;
    }

    inline FrontInsertIterator& operator * ()
    {
        return *this;
    }

    inline FrontInsertIterator& operator ++ ()
    {
        return *this;
    }

    inline FrontInsertIterator& operator ++ (int unused)
    {
        return *this;
    }
};

template <typename ContainerType>
static inline InsertIterator<ContainerType>
inserter(ContainerType& the_container,
         const typename ContainerType::iterator& it)
{
    return InsertIterator<ContainerType>(the_container, it);
}

template <typename ContainerType>
static inline InsertIterator<ContainerType>
inserter(ContainerType& the_container)
{
    return InsertIterator<ContainerType>(the_container);
}

template <typename ContainerType>
static inline BackInsertIterator<ContainerType>
back_inserter(ContainerType& the_container)
{
    return BackInsertIterator<ContainerType>(the_container);
}

template <typename ContainerType>
static inline FrontInsertIterator<ContainerType>
front_inserter(ContainerType& the_container)
{
    return FrontInsertIterator<ContainerType>(the_container);
}

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_CONTAINERS_ITERATOR_BASE_HPP_ */

//
// IteratorBase.hpp ends here
