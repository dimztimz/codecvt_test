// Copyright 2020 Dimitrij Mijoski
// SPDX-License-Identifier: GPL-3.0-or-later

#include <codecvt>
#include <cstdio>
#include <iostream>
#include <locale>
#include <string>
//#include <signal.h>

bool global_error = false;

#define VERIFY(X)                                                              \
  do                                                                           \
    {                                                                          \
      if (!(X))                                                                \
	{                                                                      \
	  global_error = true;                                                 \
	  printf (                                                             \
	    "Error in line: %d,\n    Function: %s,\n    Assertion: %s\n",      \
	    __LINE__, __FUNCTION__, #X);                                       \
	  /*raise (SIGTRAP);*/                                                 \
	}                                                                      \
    }                                                                          \
  while (0)

using namespace std;

template <typename T>
std::unique_ptr<T>
to_unique_ptr (T *ptr)
{
  return std::unique_ptr<T> (ptr);
}

struct test_offsets_ok
{
  size_t in_size, out_size;
};
struct test_offsets_partial
{
  size_t in_size, out_size, expected_in_next, expected_out_next;
};

template <class CharT> struct test_offsets_error
{
  size_t in_size, out_size, expected_in_next, expected_out_next;
  CharT replace_char;
  size_t replace_pos;
};

template <class T, size_t N>
auto constexpr array_size (const T (&)[N]) -> size_t
{
  return N;
}

void
utf8_to_utf32_in_ok (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char in[] = "bш\uAAAA\U0010AAAA";
  const char32_t u32exp[] = U"bш\uAAAA\U0010AAAA";

  static_assert ((array_size (in) == 11), "");
  static_assert (array_size (u32exp) == 5, "");
  VERIFY (char_traits<char>::length (in) == 10);
  VERIFY (char_traits<char32_t>::length (u32exp) == 4);

  test_offsets_ok offsets[] = {{0, 0}, {1, 1}, {3, 2}, {6, 3}, {10, 4}};
  for (auto t : offsets)
    {
      char32_t out[4] = {};
      VERIFY (t.out_size <= array_size (out));
      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char32_t *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.in (state, in, in + t.in_size, in_next, out, out + t.out_size,
		    out_next);
      VERIFY (res == cvt.ok);
      VERIFY (in_next == in + t.in_size);
      VERIFY (out_next == out + t.out_size);
      VERIFY (char_traits<char32_t>::compare (out, u32exp, t.out_size) == 0);
      if (t.out_size < array_size (out))
	VERIFY (out[t.out_size] == 0);
    }

  for (auto t : offsets)
    {
      char32_t out[5] = {};
      VERIFY (t.out_size <= array_size (out));
      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char32_t *) nullptr;
      auto res = codecvt_base::result ();

      res
	= cvt.in (state, in, in + t.in_size, in_next, out, end (out), out_next);
      VERIFY (res == cvt.ok);
      VERIFY (in_next == in + t.in_size);
      VERIFY (out_next == out + t.out_size);
      VERIFY (char_traits<char32_t>::compare (out, u32exp, t.out_size) == 0);
      if (t.out_size < array_size (out))
	VERIFY (out[t.out_size] == 0);
    }
}

void
utf8_to_utf32_in_partial (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char in[] = "bш\uAAAA\U0010AAAA";
  const char32_t u32exp[] = U"bш\uAAAA\U0010AAAA";

  static_assert (array_size (in) == 11, "");
  static_assert (array_size (u32exp) == 5, "");
  VERIFY (char_traits<char>::length (in) == 10);
  VERIFY (char_traits<char32_t>::length (u32exp) == 4);

  test_offsets_partial offsets[] = {
    {1, 0, 0, 0}, // no space for first CP

    {3, 1, 1, 1}, // no space for second CP
    {2, 2, 1, 1}, // incomplete second CP
    {2, 1, 1, 1}, // incomplete second CP, and no space for it

    {6, 2, 3, 2}, // no space for third CP
    {4, 3, 3, 2}, // incomplete third CP
    {5, 3, 3, 2}, // incomplete third CP
    {4, 2, 3, 2}, // incomplete third CP, and no space for it
    {5, 2, 3, 2}, // incomplete third CP, and no space for it

    {10, 3, 6, 3}, // no space for fourth CP
    {7, 4, 6, 3},  // incomplete fourth CP
    {8, 4, 6, 3},  // incomplete fourth CP
    {9, 4, 6, 3},  // incomplete fourth CP
    {7, 3, 6, 3},  // incomplete fourth CP, and no space for it
    {8, 3, 6, 3},  // incomplete fourth CP, and no space for it
    {9, 3, 6, 3},  // incomplete fourth CP, and no space for it
  };

  for (auto t : offsets)
    {
      char32_t out[4] = {};
      VERIFY (t.out_size <= array_size (out));
      VERIFY (t.expected_in_next <= t.in_size);
      VERIFY (t.expected_out_next <= t.out_size);
      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char32_t *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.in (state, in, in + t.in_size, in_next, out, out + t.out_size,
		    out_next);
      VERIFY (res == cvt.partial);
      VERIFY (in_next == in + t.expected_in_next);
      VERIFY (out_next == out + t.expected_out_next);
      VERIFY (char_traits<char32_t>::compare (out, u32exp, t.expected_out_next)
	      == 0);
      if (t.expected_out_next < array_size (out))
	VERIFY (out[t.expected_out_next] == 0);
    }
}

void
utf8_to_utf32_in_error (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  const char valid_in[] = "bш\uAAAA\U0010AAAA";
  const char32_t u32exp[] = U"bш\uAAAA\U0010AAAA";

  static_assert (array_size (valid_in) == 11, "");
  static_assert (array_size (u32exp) == 5, "");
  VERIFY (char_traits<char>::length (valid_in) == 10);
  VERIFY (char_traits<char32_t>::length (u32exp) == 4);

  test_offsets_error<char> offsets[] = {

    // replace leading byte with invalid byte
    {1, 4, 0, 0, '\xFF', 0},
    {3, 4, 1, 1, '\xFF', 1},
    {6, 4, 3, 2, '\xFF', 3},
    {10, 4, 6, 3, '\xFF', 6},

    // replace first trailing byte with ASCII byte
    {3, 4, 1, 1, 'z', 2},
    {6, 4, 3, 2, 'z', 4},
    {10, 4, 6, 3, 'z', 7},

    // replace first trailing byte with invalid byte
    {3, 4, 1, 1, '\xFF', 2},
    {6, 4, 3, 2, '\xFF', 4},
    {10, 4, 6, 3, '\xFF', 7},

    // replace first trailing byte with ASCII byte, also incomplete at end
    {5, 4, 3, 2, 'z', 4},
    {8, 4, 6, 3, 'z', 7},
    {9, 4, 6, 3, 'z', 7},

    // replace first trailing byte with invalid byte, also incomplete at end
    {5, 4, 3, 2, '\xFF', 4},
    {8, 4, 6, 3, '\xFF', 7},
    {9, 4, 6, 3, '\xFF', 7},

    // replace second trailing byte with ASCII byte
    {6, 4, 3, 2, 'z', 5},
    {10, 4, 6, 3, 'z', 8},

    // replace second trailing byte with invalid byte
    {6, 4, 3, 2, '\xFF', 5},
    {10, 4, 6, 3, '\xFF', 8},

    // replace second trailing byte with ASCII byte, also incomplete at end
    {9, 4, 6, 3, 'z', 8},

    // replace second trailing byte with invalid byte, also incomplete at end
    {9, 4, 6, 3, '\xFF', 8},

    // replace third trailing byte
    {10, 4, 6, 3, 'z', 9},
    {10, 4, 6, 3, '\xFF', 9}

  };
  for (auto t : offsets)
    {
      char in[10] = {};
      char32_t out[4] = {};
      VERIFY (t.out_size <= array_size (out));
      VERIFY (t.expected_in_next <= t.in_size);
      VERIFY (t.expected_out_next <= t.out_size);
      char_traits<char>::copy (in, valid_in, t.in_size);
      in[t.replace_pos] = t.replace_char;

      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char32_t *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.in (state, in, in + t.in_size, in_next, out, out + t.out_size,
		    out_next);
      VERIFY (res == cvt.error);
      VERIFY (in_next == in + t.expected_in_next);
      VERIFY (out_next == out + t.expected_out_next);
      VERIFY (char_traits<char32_t>::compare (out, u32exp, t.expected_out_next)
	      == 0);
      if (t.expected_out_next < array_size (out))
	VERIFY (out[t.expected_out_next] == 0);
    }
}

void
utf8_to_utf32_in (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  utf8_to_utf32_in_ok (cvt);
  utf8_to_utf32_in_partial (cvt);
  utf8_to_utf32_in_error (cvt);
}

void
utf8_to_utf32_in ()
{
  auto &cvt
    = use_facet<codecvt<char32_t, char, mbstate_t>> (locale::classic ());
  utf8_to_utf32_in (cvt);

  auto cvt_ptr = to_unique_ptr (new codecvt_utf8<char32_t> ());
  utf8_to_utf32_in (*cvt_ptr);
}

void
utf32_to_utf8_out_ok (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char32_t in[] = U"bш\uAAAA\U0010AAAA";
  const char u8exp[] = "bш\uAAAA\U0010AAAA";

  static_assert (array_size (in) == 5, "");
  static_assert (array_size (u8exp) == 11, "");
  VERIFY (char_traits<char32_t>::length (in) == 4);
  VERIFY (char_traits<char>::length (u8exp) == 10);

  const test_offsets_ok offsets[] = {{0, 0}, {1, 1}, {2, 3}, {3, 6}, {4, 10}};
  for (auto t : offsets)
    {
      char out[10] = {};
      VERIFY (t.out_size <= array_size (out));
      auto state = mbstate_t{};
      auto in_next = (const char32_t *) nullptr;
      auto out_next = (char *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.out (state, in, in + t.in_size, in_next, out, out + t.out_size,
		     out_next);
      VERIFY (res == cvt.ok);
      VERIFY (in_next == in + t.in_size);
      VERIFY (out_next == out + t.out_size);
      VERIFY (char_traits<char>::compare (out, u8exp, t.out_size) == 0);
      if (t.out_size < array_size (out))
	VERIFY (out[t.out_size] == 0);
    }
}

void
utf32_to_utf8_out_partial (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char32_t in[] = U"bш\uAAAA\U0010AAAA";
  const char u8exp[] = "bш\uAAAA\U0010AAAA";

  static_assert (array_size (in) == 5, "");
  static_assert (array_size (u8exp) == 11, "");
  VERIFY (char_traits<char32_t>::length (in) == 4);
  VERIFY (char_traits<char>::length (u8exp) == 10);

  const test_offsets_partial offsets[] = {
    {1, 0, 0, 0}, // no space for first CP

    {2, 1, 1, 1}, // no space for second CP
    {2, 2, 1, 1}, // no space for second CP

    {3, 3, 2, 3}, // no space for third CP
    {3, 4, 2, 3}, // no space for third CP
    {3, 5, 2, 3}, // no space for third CP

    {4, 6, 3, 6}, // no space for fourth CP
    {4, 7, 3, 6}, // no space for fourth CP
    {4, 8, 3, 6}, // no space for fourth CP
    {4, 9, 3, 6}, // no space for fourth CP
  };
  for (auto t : offsets)
    {
      char out[10] = {};
      VERIFY (t.out_size <= array_size (out));
      VERIFY (t.expected_out_next <= t.out_size);
      auto state = mbstate_t{};
      auto in_next = (const char32_t *) nullptr;
      auto out_next = (char *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.out (state, in, in + t.in_size, in_next, out, out + t.out_size,
		     out_next);
      VERIFY (res == cvt.partial);
      VERIFY (in_next == in + t.expected_in_next);
      VERIFY (out_next == out + t.expected_out_next);
      VERIFY (char_traits<char>::compare (out, u8exp, t.expected_out_next)
	      == 0);
      if (t.expected_out_next < array_size (out))
	VERIFY (out[t.expected_out_next] == 0);
    }
}

void
utf32_to_utf8_out_error (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  const char32_t valid_in[] = U"bш\uAAAA\U0010AAAA";
  const char u8exp[] = "bш\uAAAA\U0010AAAA";

  static_assert (array_size (valid_in) == 5, "");
  static_assert (array_size (u8exp) == 11, "");
  VERIFY (char_traits<char32_t>::length (valid_in) == 4);
  VERIFY (char_traits<char>::length (u8exp) == 10);

  test_offsets_error<char32_t> offsets[] = {{4, 10, 0, 0, 0x00110000, 0},
					    {4, 10, 1, 1, 0x00110000, 1},
					    {4, 10, 2, 3, 0x00110000, 2},
					    {4, 10, 3, 6, 0x00110000, 3}};

  for (auto t : offsets)
    {
      char32_t in[4] = {};
      char out[10] = {};
      VERIFY (t.out_size <= array_size (out));
      VERIFY (t.expected_in_next <= t.in_size);
      VERIFY (t.expected_out_next <= t.out_size);
      char_traits<char32_t>::copy (in, valid_in, t.in_size);
      in[t.replace_pos] = t.replace_char;

      auto state = mbstate_t{};
      auto in_next = (const char32_t *) nullptr;
      auto out_next = (char *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.out (state, in, in + t.in_size, in_next, out, out + t.out_size,
		     out_next);
      VERIFY (res == cvt.error);
      VERIFY (in_next == in + t.expected_in_next);
      VERIFY (out_next == out + t.expected_out_next);
      VERIFY (char_traits<char>::compare (out, u8exp, t.expected_out_next)
	      == 0);
      if (t.expected_out_next < array_size (out))
	VERIFY (out[t.expected_out_next] == 0);
    }
}

void
utf32_to_utf8_out (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  utf32_to_utf8_out_ok (cvt);
  utf32_to_utf8_out_partial (cvt);
  utf32_to_utf8_out_error (cvt);
}

void
utf32_to_utf8_out ()
{
  auto &cvt
    = use_facet<codecvt<char32_t, char, mbstate_t>> (locale::classic ());
  utf32_to_utf8_out (cvt);

  auto cvt_ptr = to_unique_ptr (new codecvt_utf8<char32_t> ());
  utf32_to_utf8_out (*cvt_ptr);
}

void
utf8_to_utf16_in_ok (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char in[] = "bш\uAAAA\U0010AAAA";
  const char16_t u16exp[] = u"bш\uAAAA\U0010AAAA";

  static_assert ((array_size (in) == 11), "");
  static_assert (array_size (u16exp) == 6, "");
  VERIFY (char_traits<char>::length (in) == 10);
  VERIFY (char_traits<char16_t>::length (u16exp) == 5);

  test_offsets_ok offsets[] = {{0, 0}, {1, 1}, {3, 2}, {6, 3}, {10, 5}};
  for (auto t : offsets)
    {
      char16_t out[5] = {};
      VERIFY (t.out_size <= array_size (out));
      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char16_t *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.in (state, in, in + t.in_size, in_next, out, out + t.out_size,
		    out_next);
      VERIFY (res == cvt.ok);
      VERIFY (in_next == in + t.in_size);
      VERIFY (out_next == out + t.out_size);
      VERIFY (char_traits<char16_t>::compare (out, u16exp, t.out_size) == 0);
      if (t.out_size < array_size (out))
	VERIFY (out[t.out_size] == 0);
    }

  for (auto t : offsets)
    {
      char16_t out[6] = {};
      VERIFY (t.out_size <= array_size (out));
      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char16_t *) nullptr;
      auto res = codecvt_base::result ();

      res
	= cvt.in (state, in, in + t.in_size, in_next, out, end (out), out_next);
      VERIFY (res == cvt.ok);
      VERIFY (in_next == in + t.in_size);
      VERIFY (out_next == out + t.out_size);
      VERIFY (char_traits<char16_t>::compare (out, u16exp, t.out_size) == 0);
      if (t.out_size < array_size (out))
	VERIFY (out[t.out_size] == 0);
    }
}

void
utf8_to_utf16_in_partial (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char in[] = "bш\uAAAA\U0010AAAA";
  const char16_t u16exp[] = u"bш\uAAAA\U0010AAAA";

  static_assert (array_size (in) == 11, "");
  static_assert (array_size (u16exp) == 6, "");
  VERIFY (char_traits<char>::length (in) == 10);
  VERIFY (char_traits<char16_t>::length (u16exp) == 5);

  test_offsets_partial offsets[] = {
    {1, 0, 0, 0}, // no space for first CP

    {3, 1, 1, 1}, // no space for second CP
    {2, 2, 1, 1}, // incomplete second CP
    {2, 1, 1, 1}, // incomplete second CP, and no space for it

    {6, 2, 3, 2}, // no space for third CP
    {4, 3, 3, 2}, // incomplete third CP
    {5, 3, 3, 2}, // incomplete third CP
    {4, 2, 3, 2}, // incomplete third CP, and no space for it
    {5, 2, 3, 2}, // incomplete third CP, and no space for it

    {10, 3, 6, 3}, // no space for fourth CP
    {10, 4, 6, 3}, // no space for fourth CP
    {7, 5, 6, 3},  // incomplete fourth CP
    {8, 5, 6, 3},  // incomplete fourth CP
    {9, 5, 6, 3},  // incomplete fourth CP
    {7, 3, 6, 3},  // incomplete fourth CP, and no space for it
    {8, 3, 6, 3},  // incomplete fourth CP, and no space for it
    {9, 3, 6, 3},  // incomplete fourth CP, and no space for it
    {7, 4, 6, 3},  // incomplete fourth CP, and no space for it
    {8, 4, 6, 3},  // incomplete fourth CP, and no space for it
    {9, 4, 6, 3},  // incomplete fourth CP, and no space for it

  };

  for (auto t : offsets)
    {
      char16_t out[5] = {};
      VERIFY (t.out_size <= array_size (out));
      VERIFY (t.expected_in_next <= t.in_size);
      VERIFY (t.expected_out_next <= t.out_size);
      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char16_t *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.in (state, in, in + t.in_size, in_next, out, out + t.out_size,
		    out_next);
      VERIFY (res == cvt.partial);
      VERIFY (in_next == in + t.expected_in_next);
      VERIFY (out_next == out + t.expected_out_next);
      VERIFY (char_traits<char16_t>::compare (out, u16exp, t.expected_out_next)
	      == 0);
      if (t.expected_out_next < array_size (out))
	VERIFY (out[t.expected_out_next] == 0);
    }
}

void
utf8_to_utf16_in_error (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  const char valid_in[] = "bш\uAAAA\U0010AAAA";
  const char16_t u16exp[] = u"bш\uAAAA\U0010AAAA";

  static_assert (array_size (valid_in) == 11, "");
  static_assert (array_size (u16exp) == 6, "");
  VERIFY (char_traits<char>::length (valid_in) == 10);
  VERIFY (char_traits<char16_t>::length (u16exp) == 5);

  test_offsets_error<char> offsets[] = {

    // replace leading byte with invalid byte
    {1, 5, 0, 0, '\xFF', 0},
    {3, 5, 1, 1, '\xFF', 1},
    {6, 5, 3, 2, '\xFF', 3},
    {10, 5, 6, 3, '\xFF', 6},

    // replace first trailing byte with ASCII byte
    {3, 5, 1, 1, 'z', 2},
    {6, 5, 3, 2, 'z', 4},
    {10, 5, 6, 3, 'z', 7},

    // replace first trailing byte with invalid byte
    {3, 5, 1, 1, '\xFF', 2},
    {6, 5, 3, 2, '\xFF', 4},
    {10, 5, 6, 3, '\xFF', 7},

    // replace first trailing byte with ASCII byte, also incomplete at end
    {5, 5, 3, 2, 'z', 4},
    {8, 5, 6, 3, 'z', 7},
    {9, 5, 6, 3, 'z', 7},

    // replace first trailing byte with invalid byte, also incomplete at end
    {5, 5, 3, 2, '\xFF', 4},
    {8, 5, 6, 3, '\xFF', 7},
    {9, 5, 6, 3, '\xFF', 7},

    // replace second trailing byte with ASCII byte
    {6, 5, 3, 2, 'z', 5},
    {10, 5, 6, 3, 'z', 8},

    // replace second trailing byte with invalid byte
    {6, 5, 3, 2, '\xFF', 5},
    {10, 5, 6, 3, '\xFF', 8},

    // replace second trailing byte with ASCII byte, also incomplete at end
    {9, 5, 6, 3, 'z', 8},

    // replace second trailing byte with invalid byte, also incomplete at end
    {9, 5, 6, 3, '\xFF', 8},

    // replace third trailing byte
    {10, 5, 6, 3, 'z', 9},
    {10, 5, 6, 3, '\xFF', 9}

  };
  for (auto t : offsets)
    {
      char in[10] = {};
      char16_t out[5] = {};
      VERIFY (t.out_size <= array_size (out));
      VERIFY (t.expected_in_next <= t.in_size);
      VERIFY (t.expected_out_next <= t.out_size);
      char_traits<char>::copy (in, valid_in, t.in_size);
      in[t.replace_pos] = t.replace_char;

      auto state = mbstate_t{};
      auto in_next = (const char *) nullptr;
      auto out_next = (char16_t *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.in (state, in, in + t.in_size, in_next, out, out + t.out_size,
		    out_next);
      VERIFY (res == cvt.error);
      VERIFY (in_next == in + t.expected_in_next);
      VERIFY (out_next == out + t.expected_out_next);
      VERIFY (char_traits<char16_t>::compare (out, u16exp, t.expected_out_next)
	      == 0);
      if (t.expected_out_next < array_size (out))
	VERIFY (out[t.expected_out_next] == 0);
    }
}

void
utf8_to_utf16_in (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  utf8_to_utf16_in_ok (cvt);
  utf8_to_utf16_in_partial (cvt);
  utf8_to_utf16_in_error (cvt);
}

void
utf8_to_utf16_in ()
{
  auto &cvt
    = use_facet<codecvt<char16_t, char, mbstate_t>> (locale::classic ());
  utf8_to_utf16_in (cvt);

  auto cvt_ptr = to_unique_ptr (new codecvt_utf8_utf16<char16_t> ());
  utf8_to_utf16_in (*cvt_ptr);
}

void
utf16_to_utf8_out_ok (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char16_t in[] = u"bш\uAAAA\U0010AAAA";
  const char u8exp[] = "bш\uAAAA\U0010AAAA";

  static_assert (array_size (in) == 6, "");
  static_assert (array_size (u8exp) == 11, "");
  VERIFY (char_traits<char16_t>::length (in) == 5);
  VERIFY (char_traits<char>::length (u8exp) == 10);

  const test_offsets_ok offsets[] = {{0, 0}, {1, 1}, {2, 3}, {3, 6}, {5, 10}};
  for (auto t : offsets)
    {
      char out[10] = {};
      VERIFY (t.out_size <= array_size (out));
      auto state = mbstate_t{};
      auto in_next = (const char16_t *) nullptr;
      auto out_next = (char *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.out (state, in, in + t.in_size, in_next, out, out + t.out_size,
		     out_next);
      VERIFY (res == cvt.ok);
      VERIFY (in_next == in + t.in_size);
      VERIFY (out_next == out + t.out_size);
      VERIFY (char_traits<char>::compare (out, u8exp, t.out_size) == 0);
      if (t.out_size < array_size (out))
	VERIFY (out[t.out_size] == 0);
    }
}

void
utf16_to_utf8_out_partial (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char16_t in[] = u"bш\uAAAA\U0010AAAA";
  const char u8exp[] = "bш\uAAAA\U0010AAAA";

  static_assert (array_size (in) == 6, "");
  static_assert (array_size (u8exp) == 11, "");
  VERIFY (char_traits<char16_t>::length (in) == 5);
  VERIFY (char_traits<char>::length (u8exp) == 10);

  const test_offsets_partial offsets[] = {
    {1, 0, 0, 0}, // no space for first CP

    {2, 1, 1, 1}, // no space for second CP
    {2, 2, 1, 1}, // no space for second CP

    {3, 3, 2, 3}, // no space for third CP
    {3, 4, 2, 3}, // no space for third CP
    {3, 5, 2, 3}, // no space for third CP

    {5, 6, 3, 6}, // no space for fourth CP
    {5, 7, 3, 6}, // no space for fourth CP
    {5, 8, 3, 6}, // no space for fourth CP
    {5, 9, 3, 6}, // no space for fourth CP

    {4, 10, 3, 6}, // incomplete fourth CP

    {4, 6, 3, 6}, // incomplete fourth CP, and no space for it
    {4, 7, 3, 6}, // incomplete fourth CP, and no space for it
    {4, 8, 3, 6}, // incomplete fourth CP, and no space for it
    {4, 9, 3, 6}, // incomplete fourth CP, and no space for it
  };
  for (auto t : offsets)
    {
      char out[10] = {};
      VERIFY (t.out_size <= array_size (out));
      VERIFY (t.expected_out_next <= t.out_size);
      auto state = mbstate_t{};
      auto in_next = (const char16_t *) nullptr;
      auto out_next = (char *) nullptr;
      auto res = codecvt_base::result ();

      res = cvt.out (state, in, in + t.in_size, in_next, out, out + t.out_size,
		     out_next);
      VERIFY (res == cvt.partial);
      VERIFY (in_next == in + t.expected_in_next);
      VERIFY (out_next == out + t.expected_out_next);
      VERIFY (char_traits<char>::compare (out, u8exp, t.expected_out_next)
	      == 0);
      if (t.expected_out_next < array_size (out))
	VERIFY (out[t.expected_out_next] == 0);
    }
}

// tests .out() function of codecvt<char16_t, char, mbstate>
void
utf16_to_utf8_out (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  utf16_to_utf8_out_ok (cvt);
  utf16_to_utf8_out_partial (cvt);
}

void
utf16_to_utf8_out ()
{
  auto &cvt
    = use_facet<codecvt<char16_t, char, mbstate_t>> (locale::classic ());
  utf16_to_utf8_out (cvt);

  auto cvt_ptr = to_unique_ptr (new codecvt_utf8_utf16<char16_t> ());
  utf16_to_utf8_out (*cvt_ptr);
}

void
utf8_to_ucs2_in ()
{
  // 2 code points, one is 3 bytes and the other is 4 bytes in UTF-8.
  // in UTF-16 the first is sinlge unit, the second is surrogate pair
  // in UCS2 only the first CP is allowed.
  const char *in = u8"\uAAAA\U0010AAAA";
  char16_t out[2] = {'y', 'y'};

  auto cvt_ptr = to_unique_ptr (new codecvt_utf8<char16_t> ());
  auto &cvt = *cvt_ptr;
  auto state = mbstate_t{};
  auto in_ptr = in;
  auto out_ptr = out;

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  auto res = cvt.in (state, in, in + 2, in_ptr, out, out, out_ptr);
  VERIFY (res == cvt.partial); // BUG, returns OK, should be Partial
  VERIFY (out_ptr == out);
  VERIFY (in_ptr == in);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.in (state, in, in + 2, in_ptr, out, out + 1, out_ptr);
  VERIFY (res == cvt.partial); // BUG, returns ERROR, should be Partial
  VERIFY (out_ptr == out);
  VERIFY (in_ptr == in);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.in (state, in, in + 3, in_ptr, out, out, out_ptr);
  VERIFY (res == cvt.partial); // BUG, return OK, should be Partial
  VERIFY (out_ptr == out);
  VERIFY (in_ptr == in);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.in (state, in, in + 3, in_ptr, out, out + 1, out_ptr);
  VERIFY (res == cvt.ok);
  VERIFY (out_ptr == out + 1);
  VERIFY (in_ptr == in + 3);
  cout << "UCS2 sequence: " << hex << out[0] << ' ' << out[1] << '\n';

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.in (state, in, in + 6, in_ptr, out, out + 1, out_ptr);
  VERIFY (res == cvt.partial); // BUG, return OK, should be Partial
  VERIFY (out_ptr == out + 1);
  VERIFY (in_ptr == in + 3);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.in (state, in, in + 6, in_ptr, out, out + 2, out_ptr);
  VERIFY (res == cvt.partial
	  || res == cvt.error); // XXX which one? Incomplete 4 byte sequence?
  // return partial because it is incomplete or error because 4 byte UTF8
  // sequences are non-BMP and invalid on UCS2
  VERIFY (out_ptr == out + 1);
  VERIFY (in_ptr == in + 3);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.in (state, in, in + 7, in_ptr, out, out + 1, out_ptr);
  VERIFY (res == cvt.partial); // BUG, returns OK, should be Partial
  VERIFY (out_ptr == out + 1);
  VERIFY (in_ptr == in + 3);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.in (state, in, in + 7, in_ptr, out, out + 2, out_ptr);
  VERIFY (res == cvt.error);
  VERIFY (out_ptr == out + 1);
  VERIFY (in_ptr == in + 3);
}

int
main ()
{
  utf8_to_utf32_in ();
  utf32_to_utf8_out ();

  utf8_to_utf16_in ();
  utf16_to_utf8_out ();

  utf8_to_ucs2_in ();
  return global_error;
}
