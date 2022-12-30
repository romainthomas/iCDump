/* Copyright 2023 R. Thomas
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ICDUMP_ITERATORS_H_
#define ICDUMP_ITERATORS_H_
#include <iostream>
#include <cmath>
#include <memory>
#include <iterator>
#include <functional>
#include <algorithm>
#include <type_traits>
#include <vector>

namespace iCDump {

// Iterator which return ref on container's values
// ===============================================

template<class T, typename U = typename std::decay_t<T>::value_type, class ITERATOR_T = typename std::decay_t<T>::iterator>
class ref_iterator {
  public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = std::decay_t<U>;
  using difference_type = ptrdiff_t;
  using pointer = typename std::remove_pointer<U>::type*;
  using reference = typename std::remove_pointer<U>::type&;

  using DT        = std::decay_t<T>;
  using DT_VAL    = U;
  using ref_t     = typename ref_iterator::reference;
  using pointer_t = typename ref_iterator::pointer;

  ref_iterator() = delete;

  ref_iterator(T container) :
    container_{std::forward<T>(container)},
    distance_{0}
  {
    this->it_ = std::begin(container_);
  }

  ref_iterator(const ref_iterator& copy) :
    container_{copy.container_},
    it_{std::begin(container_)},
    distance_{copy.distance_}
  {
    std::advance(this->it_, this->distance_);
  }


  ref_iterator& operator=(ref_iterator other) {
    this->swap(other);
    return *this;
  }

  void swap(ref_iterator& other) {
    std::swap(
        const_cast<std::add_lvalue_reference_t<std::remove_const_t<DT>>>(this->container_),
        const_cast<std::add_lvalue_reference_t<std::remove_const_t<DT>>>(other.container_));
    std::swap(this->it_, other.it_);
    std::swap(this->distance_, other.distance_);
  }


  ref_iterator& operator++() {
    this->it_ = std::next(this->it_);
    this->distance_++;
    return *this;
  }

  ref_iterator operator++(int) {
    ref_iterator retval = *this;
    ++(*this);
    return retval;
  }

  ref_iterator& operator--() {
    if (this->it_ != std::begin(container_)) {
      this->it_ = std::prev(this->it_);
      this->distance_--;
    }
    return *this;
  }

  ref_iterator operator--(int) {
    ref_iterator retval = *this;
    --(*this);
    return retval;
  }


  ref_iterator& operator+=(const typename ref_iterator::difference_type& movement) {
    std::advance(this->it_, movement);
    this->distance_ += movement;
    return *this;
  }


  ref_iterator& operator-=(const typename ref_iterator::difference_type& movement) {
    return (*this) += -movement;
  }


  // operator[]

  typename std::enable_if<not std::is_const<ref_t>::value, std::remove_const_t<ref_t>>::type
  operator[](size_t n) {
    return const_cast<std::remove_const_t<ref_t>>(static_cast<const ref_iterator*>(this)->operator[](n));
  }

  std::add_const_t<ref_t> operator[](size_t n) const {
    auto* no_const_this = const_cast<ref_iterator*>(this);

    typename ref_iterator::difference_type saved_dist = std::distance(std::begin(no_const_this->container_), no_const_this->it_);
    no_const_this->it_ = std::begin(no_const_this->container_);
    std::advance(no_const_this->it_, n);

    auto&& v = const_cast<std::add_const_t<ref_t>>(no_const_this->operator*());

    no_const_this->it_ = std::begin(no_const_this->container_);
    std::advance(no_const_this->it_, saved_dist);

    return v;
  }


  ref_iterator operator+(typename ref_iterator::difference_type n) const {
    ref_iterator tmp = *this;
    return tmp += n;
  }


  ref_iterator operator-(typename ref_iterator::difference_type n) const {
    ref_iterator tmp = *this;
    return tmp -= n;
  }


  typename ref_iterator::difference_type operator-(const ref_iterator& rhs) const {
    return this->distance_ - rhs.distance_;
  }

  bool operator<(const ref_iterator& rhs) const {
    return (rhs - *this) > 0;
  }


  bool operator>(const ref_iterator& rhs) const {
    return rhs < *this;
  }


  bool operator>=(const ref_iterator& rhs) const {
    return not (*this < rhs);
  }


  bool operator<=(const ref_iterator& rhs) const {
    return not (*this > rhs);
  }

  ref_iterator begin() const {
    return this->container_;
  }

  ref_iterator cbegin() const {
    return this->begin();
  }

  ref_iterator end()  const {
    ref_iterator it = ref_iterator{this->container_};
    it.it_ = std::end(it.container_);
    it.distance_ = it.size();
    return it;
  }

  ref_iterator cend() const {
    return this->end();
  }

  bool operator==(const ref_iterator& other) const {
    return (this->size() == other.size() and this->distance_ == other.distance_);
  }

  bool operator!=(const ref_iterator& other) const {
    return not (*this == other);
  }

  size_t size() const {
    return this->container_.size();
  }

  decltype(auto) operator*() {
    if constexpr (std::is_pointer_v<DT_VAL>) {
      return const_cast<std::add_const_t<ref_t>>(**it_);
    }

    if constexpr (!std::is_pointer_v<DT_VAL>) {
      return const_cast<std::add_const_t<ref_t>>(*it_);
    }
  }

  typename std::enable_if<not std::is_const<pointer_t>::value, pointer_t>::type
  operator->() {
    return const_cast<std::remove_const_t<pointer_t>>(static_cast<const ref_iterator*>(this)->operator->());
  }

  std::add_const_t<pointer_t> operator->() const {
    return const_cast<std::add_const_t<pointer_t>>(&(this->operator*()));
  }


  protected:
  T container_;
  ITERATOR_T it_;
  typename ref_iterator::difference_type distance_;
};


// Iterator which return const ref on container's values
// =====================================================
template<class T, typename U = typename std::decay_t<T>::value_type, class CT = typename std::add_const<T>::type>
using const_ref_iterator = ref_iterator<CT, U, typename std::decay_t<CT>::const_iterator>;
}

#endif
