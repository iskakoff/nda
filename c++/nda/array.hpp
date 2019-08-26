/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2011-2014 by O. Parcollet
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once
#include "./array_view.hpp"

namespace nda {

  // ---------------------- array--------------------------------

  template <typename ValueType, int Rank> class array : tag::concepts::_array, tag::containers::_array {
    static_assert(!std::is_const<ValueType>::value, "no const type");

    public:
    using value_t   = ValueType;
    using storage_t = mem::handle<ValueType, 'R'>;
    using idx_map_t = idx_map<Rank>;

    using regular_t    = array<ValueType, Rank>;
    using view_t       = array_view<ValueType, Rank>;
    using const_view_t = array_view<ValueType const, Rank>;

    static constexpr int rank      = Rank;
    static constexpr bool is_const = false;

    private:
    idx_map_t _idx_m;
    storage_t _storage;

    public:
    // ------------------------------- constructors --------------------------------------------

    /// Empty array
    array() = default;

    /// Makes a deep copy, since array is a regular type
    array(array const &x) : _idx_m(x.indexmap()), _storage(x.storage()) {}

    ///
    array(array &&X) = default;

    /** 
     * Construct with a shape [i0, is ...]. 
     * Int must be convertible to long, and there must be exactly Rank arguments.
     * 
     * @param i0, is ... lengths in each dimensions
     */
    template <typename... Int> explicit array(long i0, Int... is) {
      //    template <typename... Int> explicit array(long i0, Int... is) : _idx_m{{i0, is...}}, _storage{_idx_m.size()} {
      static_assert(sizeof...(Int) + 1 == Rank, "Incorrect number of arguments : should be exactly Rank. ");
      _idx_m   = idx_map_t{{i0, is...}};
      _storage = storage_t{_idx_m.size()};
      // NB first impl is more natural, but error message in case of false # of parameters (very common)
      // is better like this. FIXME to be tested in benchs
    }

    /** 
     * Construct with the given shape (memory is in C order).
     * @param shape  Shape of the array (lengths in each dimension)
     */
    explicit array(shape_t<Rank> const &shape) : _idx_m(shape), _storage(_idx_m.size()) {}

    /** 
     * [Advanced] Construct from an indexmap and a storage handle.
     *
     * @param idx index map
     * @param mem_handle  memory handle
     * NB: make a new copy.
     */
    template <char RBS> array(idx_map<Rank> const &idx, mem::handle<ValueType, RBS> mem_handle) : _idx_m(idx), _storage(std::move(mem_handle)) {}

    /// Construct from anything that has an indexmap and a storage compatible with this class
    //template <typename T> array(T const &a) REQUIRES(XXXX): array(a.indexmap(), a.storage()) {}

    // --- with memory layout

    /** 
     * Construct with the given shape and the memory layout ml
     * @param shape  Shape of the array (lengths in each dimension)
     * @param ml Memory layout
     */
    explicit array(shape_t<Rank> const &shape, std::array<int, Rank> ml) : _idx_m(shape, ml), _storage(_idx_m.size()) {}

    // FIXME : Template o
    /**
     * Construct from a copy of X, with a new memory layout
     * @param a array
     */
    //explicit array(array const &X, layout_t<Rank> ml) : array(X.indexmap(), ml) { triqs_arrays_assign_delegation(*this, X); }

    /** 
     * Build a new array from X.domain() and fill it with by evaluating X. X can be : 
     *  - another type of array, array_view, matrix,.... (any <IndexMap, Storage> pair)
     *  - the memory layout will be as given (ml)
     *  - a expression : e.g. array<int> A = B+ 2*C;
     */
    //template <typename T>
    //array(T const &X, layout_t<Rank> ml) //
    //REQUIRES(ImmutableCuboidArray<T>::value and std::is_convertible<typename T::value_t, value_t>::value)
    //: array{get_shape(X), ml} {
    //nda::details::assignment(*this, X);
    //}

    /** 
     * Build a new array from X.domain() and fill it with by evaluating X. X can be : 
     *  - another type of array, array_view, matrix,.... (any <IndexMap, Storage> pair)
     *  - the memory layout will be deduced from X if possible, or default constructed
     *  - a expression : e.g. array<int> A = B+ 2*C;
     */
    //template <typename T>
    //array(T const &X) //
    //REQUIRES(ImmutableCuboidArray<T>::value and std::is_convertible<typename T::value_t, value_t>::value)
    //: array(X, get_memory_layout<Rank, T>::invoke(X)) {}

    // --- with initializers

    /**
     * Construct from shape and a Lambda to initialize the element on site. 
     *
     * @param shape  Shape of the array (lengths in each dimension)
     * @param initializer A function/lambda.
     *
     * a(i,j,k,l) is initialized to initializer(i,j,k,l)
     *
     * EXAMPLE ?
     *
     * NB : Work even for non copy constructible ValueType.
     */
    //template <typename Initializer>
    //explicit array(shape_t<Rank> const &shape, Initializer &&initializer) : array(shape, std::forward<Initializer>(initializer)) {}

    ///// From shape and a Lambda to initialize the element
    //template <typename InitLambda>
    //explicit array(shape_t<Rank> const &shape, layout_t<Rank> ml, InitLambda &&lambda)
    //: _idx_m(shape, ml), _storage{_idx_m.size(), mem::do_not_initialize} {
    //nda::for_each(_idx_m.lengths(), [&](auto const &... x) { _storage.init_raw(_idx_m(x...), lambda(x...)); });
    //}

    /**
     * Construct from the initializer list 
     *
     * @tparam T Any type from which ValueType is constructible
     * @param l Initializer list
     *
     * @requires Rank == 1
     * T  Constructor from an initializer list for Rank 1
     */
    template <typename T>
    array(std::initializer_list<T> const &l) //
       REQUIRES((Rank == 1) and std::is_constructible_v<value_t, T>)
       : array{shape_t<Rank>{l.size()}} {
      long i = 0;
      for (auto const &x : l) (*this)(i++) = x;
    }

    private: // impl. detail for next function
    template <typename T> static shape_t<2> _comp_shape_from_list_list(std::initializer_list<std::initializer_list<T>> const &ll) {
      int s = -1;
      for (auto const &l1 : ll) {
        if (s == -1)
          s = l1.size();
        else if (s != l1.size())
          throw std::runtime_error("initializer list not rectangular !");
      }
      return {ll.size(), s};
    }

    public:
    /**
     * Construct from the initializer list of list 
     *
     * @tparam T Any type from which ValueType is constructible
     * @param ll Initializer list of list
     *
     * @requires Rank == 2
     * T  Constructor from an initializer list for Rank 1
     */
    template <typename T>
    array(std::initializer_list<std::initializer_list<T>> const &ll) //
       REQUIRES((Rank == 2) and std::is_constructible_v<value_t, T>)
       : array(_comp_shape_from_list_list(ll)) {
      long i = 0, j = 0;
      for (auto const &l1 : ll) {
        for (auto const &x : l1) { (*this)(i, j++) = x; }
        j = 0;
        ++i;
      }
    }

    /// From a temporary storage and an indexmap. Used for reshaping a temporary array
    //explicit array(typename indexmap_t::domain_t const &dom, storage_t &&sto, layout_t<Rank> ml = layout_t<Rank>{})
    //: IMPL_TYPE(indexmap_t(dom, ml), std::move(sto)) {}

    //------------------ Assignment -------------------------

    /// Move assignment
    array &operator=(array &&x) = default;

    /// Assignment. Copys the storage. All references to the storage are therefore invalidated.
    array &operator=(array const &X) = default;

    /** 
     * Assignement resizes the array (if necessary).
     * All references to the storage are therefore invalidated.
     * NB : to avoid that, do make_view(A) = X instead of A = X
     */
    template <typename RHS> array &operator=(RHS const &rhs) {
      static_assert(is_ndarray_v<RHS>, "Assignment : RHS not supported");
      resize(rhs.shape());
      //resize(get_shape(X));
      nda::details::assignment(*this, rhs);
      return *this;
    }

    //------------------ resize  -------------------------
    /** 
     * Resizes the array. NB : all references to the storage is invalidated.
     * Does not initialize the array
     * Content is undefined
     */
    template <typename... Args> void resize(Args const &... args) {
      static_assert(sizeof...(args) == Rank, "Incorrect number of arguments for resize. Should be Rank");
      static_assert(std::is_copy_constructible<ValueType>::value, "Can not resize an array if its value_t is not copy constructible");
      resize(shape_t<Rank>{args...});
    }

    void resize(shape_t<Rank> const &shape) {
      _idx_m = idx_map<Rank>(shape, _idx_m.layout());
      // build a new one with the lengths of IND BUT THE SAME layout !
      // optimisation. Construct a storage only if the new index is not compatible (size mismatch).
      if (_storage.size() != _idx_m.size()) _storage = mem::handle<ValueType, 'R'>{_idx_m.size()};
    }

    // -------------------------------  operator () --------------------------------------------

    // one can factorize the last part in a private static method, but I find clearer to have the repetition
    // here. In particular to check the && case carefully.

    /// DOC
    template <typename... T> decltype(auto) operator()(T const &... x) const & {
      if constexpr (sizeof...(T) == 0)
        return view_t{*this};
      else {

        static_assert((Rank == -1) or (sizeof...(T) == Rank) or (ellipsis_is_present<T...> and (sizeof...(T) <= Rank)),
                      "Incorrect number of parameters in call");
        //if constexpr (clef::is_any_lazy_v<T...>) return clef::make_expr_call(*this, std::forward<T>(x)...);

        auto idx_or_pos = _idx_m(x...);                                                     // we call the index map
        if constexpr (std::is_same_v<decltype(idx_or_pos), long>)                           // Case 1: we got a long, hence access a element
          return _storage[idx_or_pos];                                                      //
        else                                                                                // Case 2: we got a slice
          return array_view<ValueType, idx_or_pos.rank()>{std::move(idx_or_pos), _storage}; //
      }
    }

    ///
    template <typename... T> decltype(auto) operator()(T const &... x) & {
      if constexpr (sizeof...(T) == 0)
        return view_t{*this};
      else {

        static_assert((Rank == -1) or (sizeof...(T) == Rank) or (ellipsis_is_present<T...> and (sizeof...(T) <= Rank)),
                      "Incorrect number of parameters in call");
        //if constexpr (clef::is_any_lazy_v<T...>) return clef::make_expr_call(*this, std::forward<T>(x)...);

        auto idx_or_pos = _idx_m(x...);                                                     // we call the index map
        if constexpr (std::is_same_v<decltype(idx_or_pos), long>)                           // Case 1: we got a long, hence access a element
          return _storage[idx_or_pos];                                                      //
        else                                                                                // Case 2: we got a slice
          return array_view<ValueType, idx_or_pos.rank()>{std::move(idx_or_pos), _storage}; //
      }
    }

    ///
    template <typename... T> decltype(auto) operator()(T const &... x) && {
      if constexpr (sizeof...(T) == 0)
        return view_t{*this};
      else {

        static_assert((Rank == -1) or (sizeof...(T) == Rank) or (ellipsis_is_present<T...> and (sizeof...(T) <= Rank)),
                      "Incorrect number of parameters in call");
        //if constexpr (clef::is_any_lazy_v<T...>) return clef::make_expr_call(std::move(*this), std::forward<T>(x)...);

        auto idx_or_pos = _idx_m(x...);                           // we call the index map
        if constexpr (std::is_same_v<decltype(idx_or_pos), long>) // Case 1: we got a long, hence access a element
          return ValueType{_storage[idx_or_pos]};                 // We return a VALUE here, the array is about be destroyed.
        else                                                      // Case 2: we got a slice
          return array_view<ValueType, idx_or_pos.rank()>{std::move(idx_or_pos), _storage}; //
      }
    }

    // ------------------------------- data access --------------------------------------------

    // The Index Map object
    idx_map<Rank> const &indexmap() const { return _idx_m; }

    // The storage handle
    mem::handle<ValueType, 'R'> const &storage() const { return _storage; }
    mem::handle<ValueType, 'R'> &storage() { return _storage; }

    // Memory layout
    auto layout() const { return _idx_m.layout(); }

    /// Starting point of the data. NB : this is NOT the beginning of the memory block for a view in general
    ValueType const *data_start() const { return _storage.data + _idx_m.offset(); }

    /// Starting point of the data. NB : this is NOT the beginning of the memory block for a view in general
    ValueType *data_start() { return _storage.data() + _idx_m.offset(); }

    /// FIXME : auto : type is not good ...
    shape_t<Rank> const &shape() const { return _idx_m.lengths(); }

    /// FIXME same as shape()[i] : redondant
    //size_t shape(size_t i) const { return _idx_m.lengths()[i]; }

    /// Number of elements in the array
    long size() const { return _idx_m.size(); }

    /// FIXME : REMOVE size ? TRIVIAL
    bool is_empty() const { return size() == 0; }

    // ------------------------------- Iterators --------------------------------------------

    //using const_iterator = iterator_adapter<true, idx_map<Rank>::iterator, storage_t>;
    //using iterator       = iterator_adapter<false, idx_map<Rank>::iterator, storage_t>;
    //const_iterator begin() const { return const_iterator(indexmap(), storage(), false); }
    //const_iterator end() const { return const_iterator(indexmap(), storage(), true); }
    //const_iterator cbegin() const { return const_iterator(indexmap(), storage(), false); }
    //const_iterator cend() const { return const_iterator(indexmap(), storage(), true); }
    //iterator begin() { return iterator(indexmap(), storage(), false); }
    //iterator end() { return iterator(indexmap(), storage(), true); }

    // ------------------------------- Operations --------------------------------------------

    // TRIQS_DEFINE_COMPOUND_OPERATORS(array);

    // to forbid serialization of views...
    //template<class Archive> void serialize(Archive & ar, const unsigned int version) = delete;
  }; // namespace nda

} // namespace nda
