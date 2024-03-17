#include "mycpp/gc_str.h"

#include <ctype.h>  // isalpha(), isdigit()
#include <stdarg.h>

#include <regex>

#include "mycpp/common.h"
#include "mycpp/gc_alloc.h"     // NewStr()
#include "mycpp/gc_builtins.h"  // StringToInt()
#include "mycpp/gc_list.h"      // join(), split() use it

GLOBAL_STR(kEmptyString, "");

static const std::regex gStrFmtRegex("([^%]*)(?:%(-?[0-9]*)(.))?");
static const int kMaxFmtWidth = 256;  // arbitrary...

int BigStr::find(BigStr* needle, int start, int end) {
  if (end == -1) {
    end = len(this);
  }
  int needle_len = len(needle);

  if (needle_len > (end - start)) {
    return -1;  // needle is too long to be found (Python behavior)
  }

  if (needle_len == 1) {
    char c = needle->data_[0];
    // For 'aaa'.find('a', 0, 1)
    // end = 1, needle_len = 1, last_start = 1 which means we go through once
    for (int i = start; i < end; ++i) {
      if (data_[i] == c) {
        return i;
      }
    }
  } else {
    // Note: this works for finding the empty string.  Empty string is found in
    // empty range like [5, 5), but not in [5, 4)

    // For 'aaa'.find('aa', 0, 2)
    // end = 2, needle_len = 2, last_start = 1 which means we go through once

    int last_start = end - needle_len + 1;
    // could use a smarter substring search algorithm
    for (int i = start; i < last_start; ++i) {
      if (memcmp(data_ + i, needle->data_, needle_len) == 0) {
        return i;
      }
    }
  }
  return -1;
}

int BigStr::rfind(BigStr* needle) {
  int length = len(this);
  DCHECK(len(needle) == 1);  // Oils usage
  char c = needle->data_[0];
  for (int i = length - 1; i >= 0; --i) {
    if (data_[i] == c) {
      return i;
    }
  }
  return -1;
}

bool BigStr::isdigit() {
  int n = len(this);
  if (n == 0) {
    return false;  // special case
  }
  for (int i = 0; i < n; ++i) {
    if (!::isdigit(data_[i])) {
      return false;
    }
  }
  return true;
}

bool BigStr::isalpha() {
  int n = len(this);
  if (n == 0) {
    return false;  // special case
  }
  for (int i = 0; i < n; ++i) {
    if (!::isalpha(data_[i])) {
      return false;
    }
  }
  return true;
}

// e.g. for osh/braces.py
bool BigStr::isupper() {
  int n = len(this);
  if (n == 0) {
    return false;  // special case
  }
  for (int i = 0; i < n; ++i) {
    if (!::isupper(data_[i])) {
      return false;
    }
  }
  return true;
}

bool BigStr::startswith(BigStr* s) {
  int n = len(s);
  if (n > len(this)) {
    return false;
  }
  return memcmp(data_, s->data_, n) == 0;
}

bool BigStr::endswith(BigStr* s) {
  int len_s = len(s);
  int len_this = len(this);
  if (len_s > len_this) {
    return false;
  }
  const char* start = data_ + len_this - len_s;
  return memcmp(start, s->data_, len_s) == 0;
}

// Get a string with one character
BigStr* BigStr::at(int i) {
  int length = len(this);
  if (i < 0) {
    i = length + i;
  }
  DCHECK(0 <= i);
  DCHECK(i < length);  // had a problem here!

  BigStr* result = NewStr(1);
  result->data_[0] = data_[i];
  return result;
}

// s[begin:]
BigStr* BigStr::slice(int begin) {
  return slice(begin, len(this));
}

// s[begin:end]
BigStr* BigStr::slice(int begin, int end) {
  int length = len(this);
  SLICE_ADJUST(begin, end, length);

  DCHECK(0 <= begin && begin <= length);
  DCHECK(0 <= end && end <= length);

  int new_len = end - begin;
  DCHECK(0 <= new_len && new_len <= length);

  BigStr* result = NewStr(new_len);
  memcpy(result->data_, data_ + begin, new_len);

  return result;
}

// Used by 'help' builtin and --help, neither of which translate yet.

List<BigStr*>* BigStr::splitlines(bool keep) {
  DCHECK(keep == true);
  FAIL(kNotImplemented);
}

BigStr* BigStr::upper() {
  int length = len(this);
  BigStr* result = NewStr(length);
  char* buffer = result->data();
  for (int char_index = 0; char_index < length; ++char_index) {
    buffer[char_index] = toupper(data_[char_index]);
  }
  return result;
}

BigStr* BigStr::lower() {
  int length = len(this);
  BigStr* result = NewStr(length);
  char* buffer = result->data();
  for (int char_index = 0; char_index < length; ++char_index) {
    buffer[char_index] = tolower(data_[char_index]);
  }
  return result;
}

BigStr* BigStr::ljust(int width, BigStr* fillchar) {
  DCHECK(len(fillchar) == 1);

  int length = len(this);
  int num_fill = width - length;
  if (num_fill < 0) {
    return this;
  } else {
    BigStr* result = NewStr(width);
    char c = fillchar->data_[0];
    memcpy(result->data_, data_, length);
    for (int i = length; i < width; ++i) {
      result->data_[i] = c;
    }
    return result;
  }
}

BigStr* BigStr::rjust(int width, BigStr* fillchar) {
  DCHECK(len(fillchar) == 1);

  int length = len(this);
  int num_fill = width - length;
  if (num_fill < 0) {
    return this;
  } else {
    BigStr* result = NewStr(width);
    char c = fillchar->data_[0];
    for (int i = 0; i < num_fill; ++i) {
      result->data_[i] = c;
    }
    memcpy(result->data_ + num_fill, data_, length);
    return result;
  }
}

BigStr* BigStr::replace(BigStr* old, BigStr* new_str) {
  // Use -1 as in python2: "aaaa".replace(-1) -> "AAAA"
  return replace(old, new_str, -1);
}

BigStr* BigStr::replace(BigStr* old, BigStr* new_str, int count) {
  // log("replacing %s with %s", old_data, new_str->data_);
  const char* old_data = old->data_;

  int this_len = len(this);
  int old_len = len(old);

  const char* last_possible = data_ + this_len - old_len;

  const char* p_this = data_;  // advances through 'this'

  // First pass: Calculate number of replacements, and hence new length
  int replace_count = 0;
  if (old_len == 0) {
    replace_count = this_len + 1;
    if (count > 0) {
      replace_count = min(replace_count, count);
    }
  } else {
    while (p_this <= last_possible) {
      if (replace_count != count &&  // limit replacements (if count != -1)
          memcmp(p_this, old_data, old_len) == 0) {  // equal
        replace_count++;
        p_this += old_len;
      } else {
        p_this++;
      }
    }
  }

  // log("replacements %d", replace_count);

  if (replace_count == 0) {
    return this;  // Reuse the string if there were no replacements
  }

  int new_str_len = len(new_str);
  int result_len =
      this_len - (replace_count * old_len) + (replace_count * new_str_len);

  BigStr* result = NewStr(result_len);

  const char* new_data = new_str->data_;
  const size_t new_len = new_str_len;

  // Second pass: Copy pieces into 'result'
  p_this = data_;                  // back to beginning
  char* p_result = result->data_;  // advances through 'result'
  replace_count = 0;

  if (old_len == 0) {
    // Should place new_str between each char in this
    while (p_this < last_possible && replace_count != count) {
      replace_count++;
      memcpy(p_result, new_data, new_len);  // Copy from new_str
      p_result += new_len;                  // Move past new_str

      // Write a char from this
      *p_result = *p_this;
      p_this++;
      p_result++;
    }

    if (replace_count != count) {
      // Write a copy of new_str at the end
      assert(p_this == last_possible);
      memcpy(p_result, new_data, new_len);
    } else if (p_this <= last_possible) {
      // Write the last part of string
      memcpy(p_result, p_this, data_ + this_len - p_this);
    }
  } else {
    while (p_this <= last_possible) {
      // Note: would be more efficient if we remembered the match positions
      if (replace_count != count &&  // limit replacements (if count != -1)
          memcmp(p_this, old_data, old_len) == 0) {  // equal
        memcpy(p_result, new_data, new_len);         // Copy from new_str
        replace_count++;
        p_result += new_len;
        p_this += old_len;
      } else {  // copy 1 byte
        *p_result = *p_this;
        p_result++;
        p_this++;
      }
    }
    memcpy(p_result, p_this, data_ + this_len - p_this);  // last part of string
  }

  return result;
}

enum class StripWhere {
  Left,
  Right,
  Both,
};

const int kWhitespace = -1;

bool OmitChar(uint8_t ch, int what) {
  if (what == kWhitespace) {
    return isspace(ch);
  } else {
    return what == ch;
  }
}

// StripAny is modeled after CPython's do_strip() in stringobject.c, and can
// implement 6 functions:
//
//   strip / lstrip / rstrip
//   strip(char) / lstrip(char) / rstrip(char)
//
// Args:
//   where: which ends to strip from
//   what: kWhitespace, or an ASCII code 0-255

BigStr* StripAny(BigStr* s, StripWhere where, int what) {
  int length = len(s);
  const char* char_data = s->data();

  int i = 0;
  if (where != StripWhere::Right) {
    while (i < length && OmitChar(char_data[i], what)) {
      i++;
    }
  }

  int j = length;
  if (where != StripWhere::Left) {
    do {
      j--;
    } while (j >= i && OmitChar(char_data[j], what));
    j++;
  }

  if (i == j) {  // Optimization to reuse existing object
    return kEmptyString;
  }

  if (i == 0 && j == length) {  // nothing stripped
    return s;
  }

  // Note: makes a copy in leaky version, and will in GC version too
  int new_len = j - i;
  BigStr* result = NewStr(new_len);
  memcpy(result->data(), s->data() + i, new_len);
  return result;
}

BigStr* BigStr::strip() {
  return StripAny(this, StripWhere::Both, kWhitespace);
}

// Used for CommandSub in osh/cmd_exec.py
BigStr* BigStr::rstrip(BigStr* chars) {
  DCHECK(len(chars) == 1);
  int c = chars->data_[0];
  return StripAny(this, StripWhere::Right, c);
}

BigStr* BigStr::rstrip() {
  return StripAny(this, StripWhere::Right, kWhitespace);
}

BigStr* BigStr::lstrip(BigStr* chars) {
  DCHECK(len(chars) == 1);
  int c = chars->data_[0];
  return StripAny(this, StripWhere::Left, c);
}

BigStr* BigStr::lstrip() {
  return StripAny(this, StripWhere::Left, kWhitespace);
}

BigStr* BigStr::join(List<BigStr*>* items) {
  int length = 0;

  int num_parts = len(items);

  // " ".join([]) == ""
  if (num_parts == 0) {
    return kEmptyString;
  }

  // Common case
  // 'anything'.join(["foo"]) == "foo"
  if (num_parts == 1) {
    return items->at(0);
  }

  for (int i = 0; i < num_parts; ++i) {
    length += len(items->at(i));
  }
  // add length of all the separators
  int this_len = len(this);
  length += this_len * (num_parts - 1);

  BigStr* result = NewStr(length);
  char* p_result = result->data_;  // advances through

  for (int i = 0; i < num_parts; ++i) {
    // log("i %d", i);
    if (i != 0 && this_len) {             // optimize common case of ''.join()
      memcpy(p_result, data_, this_len);  // copy the separator
      p_result += this_len;
      // log("this_len %d", this_len);
    }

    int n = len(items->at(i));
    // log("n: %d", n);
    memcpy(p_result, items->at(i)->data_, n);  // copy the list item
    p_result += n;
  }

  return result;
}

static void AppendPart(List<BigStr*>* result, BigStr* s, int left, int right) {
  int new_len = right - left;
  BigStr* part;
  if (new_len == 0) {
    part = kEmptyString;
  } else {
    part = NewStr(new_len);
    memcpy(part->data_, s->data_ + left, new_len);
  }
  result->append(part);
}

// Split BigStr into List<BigStr*> of parts separated by 'sep'.
// The code structure is taken from CPython's Objects/stringlib/split.h.
List<BigStr*>* BigStr::split(BigStr* sep, int max_split) {
  DCHECK(sep != nullptr);
  DCHECK(len(sep) == 1);  // we can only split one char
  char sep_char = sep->data_[0];

  int str_len = len(this);
  if (str_len == 0) {
    // weird case consistent with Python: ''.split(':') == ['']
    return NewList<BigStr*>({kEmptyString});
  }

  List<BigStr*>* result = NewList<BigStr*>({});
  int left = 0;
  int right = 0;
  int num_parts = 0;  // 3 splits results in 4 parts

  while (right < str_len && num_parts < max_split) {
    // search for separator
    for (; right < str_len; right++) {
      if (data_[right] == sep_char) {
        AppendPart(result, this, left, right);
        right++;
        left = right;
        num_parts++;
        break;
      }
    }
  }
  if (num_parts == 0) {  // Optimization when there is no split
    result->append(this);
  } else if (left <= str_len) {  // Last part
    AppendPart(result, this, left, str_len);
  }

  return result;
}

List<BigStr*>* BigStr::split(BigStr* sep) {
  return this->split(sep, len(this));
}

unsigned BigStr::hash(HashFunc h) {
  if (!is_hashed_) {
    hash_ = h(data_, len(this)) >> 1;
    is_hashed_ = 1;
  }
  return hash_;
}

static inline BigStr* _StrFormat(const char* fmt, int fmt_len, va_list args) {
  auto beg = std::cregex_iterator(fmt, fmt + fmt_len, gStrFmtRegex);
  auto end = std::cregex_iterator();

  char int_buf[kMaxFmtWidth];
  std::string buf;
  for (std::cregex_iterator it = beg; it != end; ++it) {
    const std::cmatch& match = *it;

    const std::csub_match& lit_m = match[1];
    DCHECK(lit_m.matched);
    const std::string& lit_s = lit_m.str();
    buf.append(lit_s);

    int width = 0;
    bool zero_pad = false;
    bool pad_back = false;
    const std::csub_match& width_m = match[2];
    const std::string& width_s = width_m.str();
    bool ok = false;
    if (width_m.matched && !width_s.empty()) {
      if (width_s[0] == '0') {
        zero_pad = true;
        DCHECK(width_s.size() > 1);
        ok = StringToInt(width_s.c_str() + 1, width_s.size() - 1, 10, &width);
        DCHECK(ok);
        (void)ok;  // silence unused var warning in opt
      } else {
        ok = StringToInt(width_s.c_str(), width_s.size(), 10, &width);
        DCHECK(ok);
      }
      if (width < 0) {
        pad_back = true;
        width *= -1;
      }
      DCHECK(0 <= width && width < kMaxFmtWidth);
    }

    char const* str_to_add = nullptr;
    int add_len = 0;
    const std::csub_match& code_m = match[3];
    const std::string& code_s = code_m.str();
    if (!code_m.matched) {
      DCHECK(!width_m.matched);  // python errors on invalid format operators
      break;
    }
    DCHECK(code_s.size() == 1);
    switch (code_s[0]) {
    case '%': {
      str_to_add = code_s.c_str();
      add_len = 1;
      break;
    }
    case 's': {
      BigStr* s = va_arg(args, BigStr*);
      // Check type unconditionally because mycpp doesn't always check it
      CHECK(ObjHeader::FromObject(s)->type_tag == TypeTag::BigStr);

      str_to_add = s->data();
      add_len = len(s);
      zero_pad = false;  // python ignores the 0 directive for strings
      break;
    }
    case 'r': {
      BigStr* s = va_arg(args, BigStr*);
      // Check type unconditionally because mycpp doesn't always check it
      CHECK(ObjHeader::FromObject(s)->type_tag == TypeTag::BigStr);

      s = repr(s);
      str_to_add = s->data();
      add_len = len(s);
      zero_pad = false;  // python ignores the 0 directive for strings
      break;
    }
    case 'd':  // fallthrough
    case 'o': {
      int d = va_arg(args, int);
      add_len = snprintf(int_buf, kMaxFmtWidth,
                         match.str().c_str() + lit_s.size(), d);
      DCHECK(add_len > 0);
      str_to_add = int_buf;
      break;
    }
    default:
      DCHECK(0);
      break;
    }
    DCHECK(str_to_add != nullptr);

    if (pad_back) {
      buf.append(str_to_add, add_len);
    }
    if (add_len < width) {
      for (int i = 0; i < width - add_len; ++i) {
        buf.push_back(zero_pad ? '0' : ' ');
      }
    }
    if (!pad_back) {
      buf.append(str_to_add, add_len);
    }
  }

  return StrFromC(buf.c_str(), buf.size());
}

BigStr* StrIter::Value() {  // similar to at()
  BigStr* result = NewStr(1);
  result->data_[0] = s_->data_[i_];
  DCHECK(result->data_[1] == '\0');
  return result;
}

BigStr* StrFormat(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  BigStr* ret = _StrFormat(fmt, strlen(fmt), args);
  va_end(args);
  return ret;
}

BigStr* StrFormat(BigStr* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  BigStr* ret = _StrFormat(fmt->data(), len(fmt), args);
  va_end(args);
  return ret;
}
