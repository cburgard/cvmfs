/**
 * This file is part of the CernVM File System.
 *
 * Implements a string class that stores short strings on the stack and
 * malloc's a std::string on the heap on overflow.  Used for file names and
 * path names that are usually small.
 */

#ifndef SHORTSTRING_H_
#define SHORTSTRING_H_

#include <cstring>

#include <string>
#include <algorithm>

const unsigned char kDefaultMaxName = 25;
const unsigned char kDefaultMaxLink = 25;
const unsigned char kDefaultMaxPath = 200;

template<unsigned char StackSize>
class ShortString {
 public:
  ShortString() : long_string_(NULL), length_(0) { }
  ShortString(const ShortString &other) {
    long_string_ = NULL;
    Assign(other);
  }
  ShortString(const char *chars, const unsigned length) {
    long_string_ = NULL;
    Assign(chars, length);
  }
  ShortString & operator= (const ShortString & other) {
    if (this != &other)
      Assign(other);
    return *this;
  }
  ~ShortString() { delete long_string_; }

  void Assign(const char *chars, const unsigned length) {
    delete long_string_;
    long_string_ = NULL;
    if (length > StackSize) {
      long_string_ = new std::string(chars, length);
    } else {
      if (length)
        memcpy(stack_, chars, length);
      this->length_ = length;
    }
  }

  void Assign(const ShortString &other) {
    if (other.long_string_) {
      Assign(&((*other.long_string_)[0]), other.long_string_->length());
    } else {
      Assign(other.stack_, other.length_);
    }
  }

  void Append(const ShortString &other) {
    Append(other.stack_, other.length_);
  }

  void Append(const char *chars, const unsigned length) {
    if (long_string_) {
      long_string_->append(chars, length);
      return;
    }

    const unsigned new_length = this->length_ + length;
    if (new_length > StackSize) {
      long_string_ = new std::string();
      long_string_->reserve(new_length);
      long_string_->assign(stack_, length_);
      long_string_->append(chars, length);
      return;
    }
    if (length > 0)
      memcpy(&stack_[this->length_], chars, length);
    this->length_ = new_length;
  }

  const char *GetChars() const {
    if (long_string_)
      return long_string_->data();
    else
      return stack_;
  }

  unsigned GetLength() const {
    if (long_string_)
      return long_string_->length();
    return length_;
  }

  bool IsEmpty() const {
    return GetLength() == 0;
  }

  std::string ToString() const {
    return std::string(this->GetChars(), this->GetLength());
  }

  const char *c_str() const {
    if (long_string_)
      return long_string_->c_str();

    char *c = (char *)stack_ + length_;
    *c = '\0';
    return stack_;
  }

  bool operator ==(const ShortString &other) const {
    const unsigned this_length = this->GetLength();
    const unsigned other_length = other.GetLength();
    if (this_length != other_length)
      return false;
    if (this_length == 0)
      return true;

    return memcmp(this->GetChars(), other.GetChars(), this_length) == 0;
  }

  bool operator !=(const ShortString &other) const {
    return !(*this == other);
  }

  bool operator <(const ShortString &other) const {
    const unsigned this_length = this->GetLength();
    const unsigned other_length = other.GetLength();

    if (this_length < other_length)
      return true;
    if (this_length > other_length)
      return false;

    const char *this_chars = this->GetChars();
    const char *other_chars = other.GetChars();
    for (unsigned i = 0; i < this_length; ++i) {
      if (this_chars[i] < other_chars[i])
        return true;
      if (this_chars[i] > other_chars[i])
        return false;
    }
    return false;
  }

  bool StartsWith(const ShortString &other) const {
    const unsigned this_length = this->GetLength();
    const unsigned other_length = other.GetLength();
    if (this_length < other_length)
      return false;

    return memcmp(this->GetChars(), other.GetChars(), other_length) == 0;
  }

  ShortString Suffix(const unsigned start_at) const {
    const unsigned length = this->GetLength();
    if (start_at >= length)
      return ShortString("", 0);

    return ShortString(this->GetChars() + start_at, length-start_at);
  }

 private:
  std::string *long_string_;
  char stack_[StackSize+1];  // +1 to add a final '\0' if necessary
  unsigned char length_;
};  // class ShortString

typedef ShortString<kDefaultMaxPath> PathString;
typedef ShortString<kDefaultMaxName> NameString;
typedef ShortString<kDefaultMaxLink> LinkString;

#endif  // SHORTSTRING_H_