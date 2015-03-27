// Comparators.hpp ---
//
// Filename: Comparators.hpp
// Author: Abhishek Udupa
// Created: Tue Mar 17 16:09:39 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_BASETYPES_COMPARATORS_HPP_
#define KINARA_KINARA_COMMON_BASETYPES_COMPARATORS_HPP_

#include "KinaraTypes.hpp"

namespace kinara {
namespace utils {

template <typename T, template <typename> class CompareFunction>
class Comparer
{
private:
    inline bool compare(const T& object1, const T& object2,
                        const std::true_type is_comparable) const
    {
        CompareFunction<i64> cmp_fun;
        return (cmp_fun(object1.compare(object2), 0));
    }

    inline bool compare(const T& object1, const T& object2,
                        const std::false_type is_comparable) const
    {
        CompareFunction<T> cmp_fun;
        return cmp_fun(object1, object2);
    }

public:
    inline bool operator () (const T& object1, const T& object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T, template <typename> class CompareFunction>
class Comparer<T*, CompareFunction>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        const std::true_type is_comparable) const
    {
        CompareFunction<i64> cmp_fun;
        return (cmp_fun(object1->compare(*object2), 0));
    }

    inline bool compare(const T* object1, const T* object2,
                        const std::false_type is_comparable) const
    {
        CompareFunction<const T*> cmp_fun;
        return cmp_fun(object1, object2);
    }

public:
    inline bool operator () (const T* object1, const T* object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T, template <typename> class CompareFunction>
class Comparer<const T*, CompareFunction>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        const std::true_type is_comparable) const
    {
        CompareFunction<i64> cmp_fun;
        return (cmp_fun(object1->compare(*object2), 0));
    }

    inline bool compare(const T* object1, const T* object2,
                        const std::false_type is_comparable) const
    {
        CompareFunction<T*> cmp_fun;
        return cmp_fun(object1, object2);
    }

public:
    inline bool operator () (const T* object1, const T* object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T, template <typename> class CompareFunction>
class Comparer<memory::ManagedPointer<T>, CompareFunction>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        std::true_type is_comparable) const
    {
        CompareFunction<i64> cmp_fun;
        return (cmp_fun(object1->compare(*object2)));
    }

    inline bool compare(const T* object1, const T* object2,
                        std::false_type is_comparable) const
    {
        CompareFunction<T*> cmp_fun;
        return (cmp_fun(object1->compare(&(*object2))));
    }

public:
    inline bool operator () (const memory::ManagedPointer<T>& object1,
                             const memory::ManagedPointer<T>& object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T, template <typename> class CompareFunction>
class Comparer<memory::ManagedConstPointer<T>, CompareFunction>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        std::true_type is_comparable) const
    {
        CompareFunction<i64> cmp_fun;
        return (cmp_fun(object1->compare(*object2)));
    }

    inline bool compare(const T* object1, const T* object2,
                        std::false_type is_comparable) const
    {
        CompareFunction<T*> cmp_fun;
        return (cmp_fun(object1->compare(&(*object2))));
    }

public:
    inline bool operator () (const memory::ManagedConstPointer<T>& object1,
                             const memory::ManagedConstPointer<T>& object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T>
class Equality
{
private:
    inline bool compare(const T& object1, const T& object2,
                        const std::true_type is_comparable) const
    {
        return object1.equals(object2);
    }

    inline bool compare(const T& object1, const T& object2,
                        const std::false_type is_comparable) const
    {
        return (object1 == object2);
    }

public:
    inline bool operator () (const T& object1, const T& object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T>
class Equality<T*>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        const std::true_type is_comparable) const
    {
        return object1->equals(*object2);
    }

    inline bool compare(const T* object1, const T* object2,
                        const std::false_type is_comparable) const
    {
        return (object1 == object2);
    }

public:
    inline bool operator () (const T* object1, const T* object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T>
class Equality<const T*>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        const std::true_type is_comparable) const
    {
        return object1->equals(*object2);
    }

    inline bool compare(const T* object1, const T* object2,
                        const std::false_type is_comparable) const
    {
        return (object1 == object2);
    }

public:
    inline bool operator () (const T* object1, const T* object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T>
class Equality<memory::ManagedPointer<T>>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        const std::true_type is_comparable) const
    {
        return object1->equals(*object2);
    }

    inline bool compare(const T* object1, const T* object2,
                        const std::false_type is_comparable) const
    {
        return (object1 == object2);
    }

public:
    inline bool operator () (const memory::ManagedPointer<T>& object1,
                             const memory::ManagedPointer<T>& object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T>
class Equality<memory::ManagedConstPointer<T>>
{
private:
    inline bool compare(const T* object1, const T* object2,
                        const std::true_type is_comparable) const
    {
        return object1->equals(*object2);
    }

    inline bool compare(const T* object1, const T* object2,
                        const std::false_type is_comparable) const
    {
        return (object1 == object2);
    }

public:
    inline bool operator () (const memory::ManagedConstPointer<T>& object1,
                             const memory::ManagedConstPointer<T>& object2) const
    {
        typename std::is_base_of<ComparableEBC, T>::type is_comparable;
        return compare(object1, object2, is_comparable);
    }
};

template <typename T>
class NotEquality
{
public:
    inline bool operator () (const T& object1, const T& object2) const
    {
        Equality<T> equals_func;
        return (!(equals_func(object1, object2)));
    }
};

template <typename T>
using Less = Comparer<T, std::less>;

template <typename T>
using LessEqual = Comparer<T, std::less_equal>;

template <typename T>
using Greater = Comparer<T, std::greater>;

template <typename T>
using GreaterEqual = Comparer<T, std::greater_equal>;

template <typename T>
using Equal = Equality<T>;

template <typename T>
using NEqual = NotEquality<T>;

} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_BASETYPES_COMPARATORS_HPP_ */

//
// Comparators.hpp ends here
