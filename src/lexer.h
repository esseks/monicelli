#ifndef MONICELLI_LEXER_H
#define MONICELLI_LEXER_H

// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

#include "lexer.def"
#include "location.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace monicelli {

class Token final : public LocationMixin {
public:
  enum TokenType {
#define DECLARE_TOKEN(NAME, _) TOKEN_##NAME,
    LEXER_TOKENS(DECLARE_TOKEN)
#undef DECLARE_TOKEN
  };

  enum BuiltinTypeValue {
#define DECLARE_TYPE(NAME, _1, _2, _3, _4, _5) BUILTIN_TYPE_##NAME,
    BUILTIN_TYPES(DECLARE_TYPE)
#undef DECLARE_TYPE
  };

  ~Token() {
    if (value_type_ == ValueType::STRING) {
      string_value_.std::string::~string();
    }
  }

  TokenType getType() const { return type_; }

  operator TokenType() const { return getType(); }

  bool isOperator() const;

  uint64_t getIntValue() const {
    assert(getValueTypeForToken(type_) == ValueType::INTEGER);
    return int_value_;
  }

  double getFloatValue() const {
    assert(getValueTypeForToken(type_) == ValueType::FLOAT);
    return fp_value_;
  }

  BuiltinTypeValue getBuiltinTypeValue() const {
    assert(getValueTypeForToken(type_) == ValueType::BUILTIN_TYPE);
    return builtin_type_value_;
  }

  const std::string& getStringValue() const {
    assert(getValueTypeForToken(type_) == ValueType::STRING);
    return string_value_;
  }

  void print(std::ostream& stream);

private:
  enum class ValueType { VOID, STRING, FLOAT, INTEGER, BUILTIN_TYPE };

  Token(TokenType type, Location first_location, Location last_location)
      : LocationMixin(first_location, last_location), type_(type), value_type_(ValueType::VOID) {}

  Token(TokenType type, Location location)
      : LocationMixin(location, location), type_(type), value_type_(ValueType::VOID) {}

  void setIntValue(uint64_t value) {
    assert(getValueTypeForToken(type_) == ValueType::INTEGER);
    int_value_ = value;
  }

  void setFloatValue(double value) {
    assert(getValueTypeForToken(type_) == ValueType::FLOAT);
    fp_value_ = value;
  }

  void setBuiltinTypeValue(BuiltinTypeValue value) {
    assert(getValueTypeForToken(type_) == ValueType::BUILTIN_TYPE);
    builtin_type_value_ = value;
  }

  void setStringValue(std::string&& value) {
    assert(getValueTypeForToken(type_) == ValueType::STRING);
    new (&string_value_) std::string(value);
  }

  static ValueType getValueTypeForToken(TokenType type);

  TokenType type_;

  ValueType value_type_;
  union {
    uint64_t int_value_;
    double fp_value_;
    BuiltinTypeValue builtin_type_value_;
    std::string string_value_;
  };

  friend class Lexer;
};

class Buffer final {
public:
  static const int DEFAULT_CAPACITY = 1 * 1024 * 1024;

  Buffer(int base_capacity = DEFAULT_CAPACITY) : size_(0), capacity_(base_capacity) {
    data_.reset(new char[base_capacity]);
    cursor_ = data_.get();
  }

  void shift(int amount) {
    assert(amount <= size_ && "Cannot shift buffer more than its size.");
    size_ -= amount;
    memmove(data_.get(), data_.get() + amount, size_);
  }
  void imbue(std::istream& input);
  void clear() { size_ = 0; }

  bool isExhausted() const { return cursor_ == data_.get() + size_; }

  char* getData() { return data_.get(); }
  char* getDataEnd() { return data_.get() + size_; }
  int getSize() const { return size_; }

  char* getCursor() { return cursor_; }
  void setCursor(char* value) {
    assert(data_.get() <= value && value <= data_.get() + size_ && "Cursor out of bounds.");
    cursor_ = value;
  }

private:
  int size_;
  int capacity_;
  std::unique_ptr<char[]> data_;
  char* cursor_;
};

class Lexer final {
public:
  explicit Lexer(std::istream& input) : input_(input), trace_enabled_(false) { resetState(); }

  std::unique_ptr<Token> getNextToken();

  bool isTraceEnabled() const { return trace_enabled_; }
  void setTraceEnabled(bool enable) { trace_enabled_ = enable; }
  Location getCurrentLocation() const { return current_location_; }

private:
  void advanceColumn() {
    assert(state_.ts != nullptr && state_.te >= state_.ts);
    current_location_.advanceColumn(state_.te - state_.ts);
  }
  void newLine() { current_location_.newLine(); }

  void resetState();
  void advanceBuffer();

  std::istream& input_;
  Location current_location_;

  struct {
    // State of the lexer FSA. DO NOT MODIFY.
    int cs;
    int act;

    // Start and end of the current token.
    char* ts;
    char* te;
  } state_;

  Buffer buffer_;

  bool trace_enabled_;
};

} // namespace monicelli

#endif
