#include "binder.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>
#include <string>
#include <string_view>

using std::as_const;
using std::invalid_argument;
using std::pair;
using std::make_pair;
using std::string;
using std::string_view;
using std::vector;
using cxx::binder;

int main() {
  static_assert(std::forward_iterator<binder<string, string>::const_iterator>, "const_iterator must satisfy std::forward_iterator");
}
