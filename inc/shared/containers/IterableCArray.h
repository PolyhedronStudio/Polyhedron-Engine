// LICENSE HERE.

//
// shared/containers/IterableCArray.h
//
// N&C Container Library: IterableCArray
// 
// Can be used to wrap up C arrays, so they can be nicely iterated in a C++ friendly style.
// This method we are using here should be performance prove, so it won't be a burden.
//
#ifndef __SHARED_CONTAINERS_ITERABLECARRAY_H__
#define __SHARED_CONTAINERS_ITERABLECARRAY_H__

template <typename T, std::size_t SIZE_, T(&ARRAY_)[SIZE_]>
class IteratableCArray {
public:
    // Proper typedefs.
    typedef T                                     value_type;
    typedef std::size_t                           size_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Indexes for positions in the array.
    enum {
        SIZE = SIZE_,
        MAX_SIZE = SIZE_,
        FRONT = 0,
        BACK = SIZE - 1,
        BEGIN = 0,
        END = SIZE,
        RBEGIN = SIZE - 1,
        REND = -1
    };

    //===============
    /// Returns a reference to the first element.
    //===============
    reference front() {
        return *&ARRAY_[FRONT];
    }

    //===============
    /// Returns a const reference to the first element.
    //===============
    constexpr const_reference front() const {
        return *&ARRAY_[FRONT];
    }

    //===============
    /// Returns a reference to the last element.
    //===============
    reference back() {
        return *&ARRAY_[BACK];
    }

    //===============
    /// Returns a const reference to the last element.
    //===============
    constexpr const_reference back() const {
        return *&ARRAY_[BACK];
    }

    //*************************************************************************
    /// Returns a pointer to the first element of the internal storage.
    //*************************************************************************
    pointer data() {
        return &ARRAY_[BEGIN];
    }

    //*************************************************************************
    /// Returns a const pointer to the first element of the internal storage.
    //*************************************************************************
    constexpr const_pointer data() const {
        return &ARRAY_[BEGIN];
    }

    //*************************************************************************
    /// Returns an iterator to the beginning of the array.
    //*************************************************************************
    iterator begin() {
        return &ARRAY_[BEGIN];
    }

    //*************************************************************************
    /// Returns a const iterator to the beginning of the array.
    //*************************************************************************
    constexpr const_iterator begin() const {
        return &ARRAY_[BEGIN];
    }

    //*************************************************************************
    /// Returns a const iterator to the beginning of the array.
    //*************************************************************************
    constexpr const_iterator cbegin() const {
        return &ARRAY_[BEGIN];
    }

    //*************************************************************************
    /// Returns an iterator to the end of the array.
    //*************************************************************************
    iterator end() {
        return &ARRAY_[END];
    }

    //*************************************************************************
    /// Returns a const iterator to the end of the array.
    //*************************************************************************
    constexpr const_iterator end() const {
        return &ARRAY_[END];
    }

    //*************************************************************************
    // Returns a const iterator to the end of the array.
    //*************************************************************************
    constexpr const_iterator cend() const {
        return &ARRAY_[END];
    }

    //*************************************************************************
    // Returns an reverse iterator to the reverse beginning of the array.
    //*************************************************************************
    reverse_iterator rbegin() {
        return reverse_iterator(&ARRAY_[END]);
    }

    //*************************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the array.
    //*************************************************************************
    constexpr const_reverse_iterator rbegin() const {
        return const_reverse_iterator(&ARRAY_[END]);
    }

    //*************************************************************************
    /// Returns a const reverse iterator to the reverse beginning of the array.
    //*************************************************************************
    constexpr const_reverse_iterator crbegin() const {
        return const_reverse_iterator(&ARRAY_[END]);
    }

    //*************************************************************************
    /// Returns a reverse iterator to the end of the array.
    //*************************************************************************
    reverse_iterator rend() {
        return reverse_iterator(&ARRAY_[BEGIN]);
    }

    //*************************************************************************
    /// Returns a const reverse iterator to the end of the array.
    //*************************************************************************
    constexpr const_reverse_iterator rend() const {
        return const_reverse_iterator(&ARRAY_[BEGIN]);
    }

    //*************************************************************************
    /// Returns a const reverse iterator to the end of the array.
    //*************************************************************************
    constexpr const_reverse_iterator crend() const {
        return const_reverse_iterator(&ARRAY_[BEGIN]);
    }

    //*************************************************************************
    /// Returns a reference to the indexed value.
    //*************************************************************************
    reference operator[](size_t i) {
        return ARRAY_[i];
    }

    //*************************************************************************
    /// Returns a const reference to the indexed value.
    //*************************************************************************
    constexpr const_reference operator[](size_t i) const {
        return ARRAY_[i];
    }

    //*************************************************************************
    /// Returns a reference to the indexed value.
    //*************************************************************************
    reference at(size_t i) {
        return ARRAY_[i];
    }

    //*************************************************************************
    /// Returns a const reference to the indexed value.
    //*************************************************************************
    constexpr const_reference at(size_t i) const {
        return ARRAY_[i];
    }

    //*************************************************************************
    /// Returns the size of the array.
    //===============
    constexpr size_t size() const {
        return SIZE;
    }

    //*************************************************************************
    /// Returns the maximum possible size of the array.
    //*************************************************************************
    constexpr size_t max_size() const {
        return MAX_SIZE;
    }

    //*************************************************************************
    /// Fills the array.
    //*************************************************************************
    void fill(const T& value) {
        std::fill(begin(), end(), value);
    }

    //*************************************************************************
    /// Swaps the contents of arrays.
    //*************************************************************************
    //template <typename U, U(&ARRAYOTHER)[SIZE_]>
    //typename std::enable_if<std::is_same<T, U>::value, void>::type
    //    swap(IteratableCArray<U, SIZE_, ARRAYOTHER>& other) {
    //    for (size_t i = 0; i < SIZE; ++i)   {
    //        std::swap(ARRAY_[i], other.begin()[i]);
    //    }
    //}

    ////*************************************************************************
    ///// Equality for array wrappers.
    ////===============
    //template <typename TL, typename TR, std::size_t SIZEL, std::size_t SIZER, TL(&ARRAYL)[SIZEL], TR(&ARRAYR)[SIZER]>
    //bool operator == (const IteratableCArray<TL, SIZEL, ARRAYL>& lhs,
    //    const IteratableCArray<TR, SIZER, ARRAYR>& rhs) {
    //    return (SIZEL == SIZER) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    //}

    ////*************************************************************************
    ///// Inequality for array wrapper.
    ////*************************************************************************
    //template <typename TL, typename TR, std::size_t SIZEL, std::size_t SIZER, TL(&ARRAYL)[SIZEL], TR(&ARRAYR)[SIZER]>
    //bool operator != (const IteratableCArray<TL, SIZEL, ARRAYL>& lhs,
    //    const IteratableCArray<TR, SIZER, ARRAYR>& rhs) {
    //    return !(lhs == rhs);
    //}

    ////*************************************************************************
    ///// Less-than for array wrapper.
    ////*************************************************************************
    //template <typename TL, typename TR, std::size_t SIZEL, std::size_t SIZER, TL(&ARRAYL)[SIZEL], TR(&ARRAYR)[SIZER]>
    //bool operator < (const IteratableCArray<TL, SIZEL, ARRAYL>& lhs,
    //    const IteratableCArray<TR, SIZER, ARRAYR>& rhs) {
    //    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    //}

    ////*************************************************************************
    ///// Greater-than for array wrapper.
    ////*************************************************************************
    //template <typename TL, typename TR, std::size_t SIZEL, std::size_t SIZER, TL(&ARRAYL)[SIZEL], TR(&ARRAYR)[SIZER]>
    //bool operator > (const IteratableCArray<TL, SIZEL, ARRAYL>& lhs,
    //    const IteratableCArray<TR, SIZER, ARRAYR>& rhs) {
    //    return rhs < lhs;
    //}

    ////*************************************************************************
    ///// Less-than-equal for array wrapper.
    ////*************************************************************************
    //template <typename TL, typename TR, std::size_t SIZEL, std::size_t SIZER, TL(&ARRAYL)[SIZEL], TR(&ARRAYR)[SIZER]>
    //bool operator <= (const IteratableCArray<TL, SIZEL, ARRAYL>& lhs,
    //    const IteratableCArray<TR, SIZER, ARRAYR>& rhs) {
    //    return !(lhs > rhs);
    //}

    ////*************************************************************************
    ///// Greater-than-equal for array wrapper.
    ////*************************************************************************
    //template <typename TL, typename TR, std::size_t SIZEL, std::size_t SIZER, TL(&ARRAYL)[SIZEL], TR(&ARRAYR)[SIZER]>
    //bool operator >= (const IteratableCArray<TL, SIZEL, ARRAYL>& lhs,
    //    const IteratableCArray<TR, SIZER, ARRAYR>& rhs) {
    //    return !(lhs < rhs);
    //}

    //*************************************************************************
    /// Swap.
    //*************************************************************************
    template <typename T, std::size_t SIZE, T(&ARRAYL)[SIZE], T(&ARRAYR)[SIZE]>
    void swap(IteratableCArray<T, SIZE, ARRAYL>& lhs,
        IteratableCArray<T, SIZE, ARRAYR>& rhs) {
        lhs.swap(rhs);
    }
};

#endif // __SHARED_CONTAINERS_ITERABLECARRAY_H__
