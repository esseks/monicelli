#ifndef MONICELLI_ITERATORS_H
#define MONICELLI_ITERATORS_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include <memory>
#include <vector>

namespace monicelli {

template<typename T> class PointerVectorConstIter final {
public:
  typedef typename std::vector<std::unique_ptr<T>>::const_iterator ConstIter;

  PointerVectorConstIter(ConstIter iter) : internal_iter_(iter) {}

  const T* operator*() const { return internal_iter_->get(); }
  const T* operator->() const { return internal_iter_->get(); }
  bool operator!=(const PointerVectorConstIter& other) {
    return internal_iter_ != other.internal_iter_;
  }

  PointerVectorConstIter<T>& operator++() {
    ++internal_iter_;
    return *this;
  }
  PointerVectorConstIter<T> operator++(int) { return {internal_iter_++}; }

private:
  ConstIter internal_iter_;
};

template<typename IterT> class ConstRangeWrapper final {
public:
  ConstRangeWrapper(IterT begin, IterT end) : begin_(begin), end_(end) {}

  IterT begin() const { return begin_; }
  IterT end() const { return end_; }

private:
  IterT begin_;
  IterT end_;
};

} // namespace monicelli

#endif
