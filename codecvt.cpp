// Copyright 2020 Dimitrij Mijoski
// SPDX-License-Identifier: GPL-3.0-or-later

#include <codecvt>
#include <cstdio>
#include <iostream>
#include <locale>
#include <string>

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
	}                                                                      \
    }                                                                          \
  while (0)

using namespace std;

// 2 code points, both are 4 byte in UTF-8.
// in UTF-16 both are 2 unit i.e. surrogate pairs
const char u8in[] = u8"\U0010FFFF\U0010AAAA";
const char16_t u16in[] = u"\U0010FFFF\U0010AAAA";
const char32_t u32in[] = U"\U0010FFFF\U0010AAAA";

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

template <class T, size_t N>
auto
array_size (const T (&)[N]) -> size_t
{
  return N;
}

void
utf8_to_utf32_in_ok (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  // UTF-8 string of 1-byte CP, 2-byte CP, 3-byte CP and 4-byte CP
  const char in[] = "bш\uAAAA\U0010AAAA";
  const char32_t u32exp[] = U"bш\uAAAA\U0010AAAA";

  VERIFY (array_size (in) == 11);
  VERIFY (char_traits<char>::length (in) == 10);
  VERIFY (array_size (u32exp) == 5);
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

  VERIFY (array_size (in) == 11);
  VERIFY (char_traits<char>::length (in) == 10);
  VERIFY (array_size (u32exp) == 5);
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
utf8_to_utf32_in_error_1 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  char in[8] = {};
  char32_t out[2] = {};
  char_traits<char>::copy (in, u8in, 8);
  in[3] = 'z';

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char32_t *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 2, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 0);
  VERIFY (out_next == out + 0);
  VERIFY (out[0] == 0 && out[1] == 0);
}

void
utf8_to_utf32_in_error_2 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  char in[8] = {};
  char32_t out[2] = {};
  char_traits<char>::copy (in, u8in, 8);
  in[3] = '\xFF';

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char32_t *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 2, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 0);
  VERIFY (out_next == out + 0);
  VERIFY (out[0] == 0 && out[1] == 0);
}

void
utf8_to_utf32_in_error_3 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  char in[8] = {};
  char32_t out[3] = {};
  char_traits<char>::copy (in, u8in, 8);
  in[4] = 'z';

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char32_t *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 5);
  VERIFY (out_next == out + 2);
  VERIFY (out[0] == u32in[0] && out[1] == 'z' && out[2] == 0);
}

void
utf8_to_utf32_in_error_4 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  char in[8] = {};
  char32_t out[3] = {};
  char_traits<char>::copy (in, u8in, 8);
  in[4] = '\xFF';

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char32_t *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0] && out[1] == 0 && out[2] == 0);
}

void
utf8_to_utf32_in_error_5 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  char in[8] = {};
  char32_t out[3] = {};
  char_traits<char>::copy (in, u8in, 8);
  in[7] = 'z';

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char32_t *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0] && out[1] == 0 && out[2] == 0);
}

void
utf8_to_utf32_in_error_6 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  char in[8] = {};
  char32_t out[3] = {};
  char_traits<char>::copy (in, u8in, 8);
  in[7] = '\xFF';

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char32_t *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0] && out[1] == 0 && out[2] == 0);
}

void
utf8_to_utf32_in_error_7 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  char in[7] = {};
  char32_t out[3] = {};
  char_traits<char>::copy (in, u8in, 7);
  in[5] = 'z';
  // last CP has two errors. its second code unit is malformed and it misses its
  // last.

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char32_t *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 7, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0] && out[1] == 0 && out[2] == 0);
}

void
utf8_to_utf32_in (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  utf8_to_utf32_in_ok (cvt);
  utf8_to_utf32_in_partial (cvt);
  utf8_to_utf32_in_error_1 (cvt);
  utf8_to_utf32_in_error_2 (cvt);
  utf8_to_utf32_in_error_3 (cvt);
  utf8_to_utf32_in_error_4 (cvt);
  utf8_to_utf32_in_error_5 (cvt);
  utf8_to_utf32_in_error_6 (cvt);
  utf8_to_utf32_in_error_7 (cvt);
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
utf32_to_utf8_out_ok_1 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u32in;
  char out[4] = {};

  auto state = mbstate_t{};
  auto in_next = (const char32_t *) nullptr;
  auto out_next = (char *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.out (state, in, in + 1, in_next, out, out + 4, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 1);
  VERIFY (out_next == out + 4);
  VERIFY (char_traits<char>::compare (out, u8in, 4) == 0);
}

void
utf32_to_utf8_out_ok_2 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u32in;
  char out[8] = {};

  auto state = mbstate_t{};
  auto in_next = (const char32_t *) nullptr;
  auto out_next = (char *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.out (state, in, in + 2, in_next, out, out + 8, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 2);
  VERIFY (out_next == out + 8);
  VERIFY (char_traits<char>::compare (out, u8in, 8) == 0);
}

void
utf32_to_utf8_out_partial_1 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u32in;
  char out[4] = {};

  auto state = mbstate_t{};
  auto in_next = (const char32_t *) nullptr;
  auto out_next = (char *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.out (state, in, in + 2, in_next, out, out + 4, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 1);
  VERIFY (out_next == out + 4);
  VERIFY (char_traits<char>::compare (out, u8in, 4) == 0);
}

void
utf32_to_utf8_out_partial_2 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u32in;
  char out[6] = {};
  char expected[6] = {};
  char_traits<char>::copy (expected, u8in, 4);

  auto state = mbstate_t{};
  auto in_next = (const char32_t *) nullptr;
  auto out_next = (char *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.out (state, in, in + 2, in_next, out, out + 6, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 1);
  VERIFY (out_next == out + 4);
  VERIFY (char_traits<char>::compare (out, expected, 6) == 0);
}

void
utf32_to_utf8_out_error_1 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  const char32_t in[2] = {0x0010FFFF, 0xFFFFFFFF};
  char out[8] = {};
  char expected[8] = "\U0010FFFF\0\0\0";

  auto state = mbstate_t{};
  auto in_next = (const char32_t *) nullptr;
  auto out_next = (char *) nullptr;
  auto res = codecvt_base::result ();

  res = cvt.out (state, in, in + 2, in_next, out, out + 8, out_next);
  VERIFY (res == cvt.error);
  VERIFY (in_next == in + 1);
  VERIFY (out_next == out + 4);
  VERIFY (char_traits<char>::compare (out, expected, 8) == 0);
}

void
utf32_to_utf8_out (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  utf32_to_utf8_out_ok_1 (cvt);
  utf32_to_utf8_out_ok_2 (cvt);
  utf32_to_utf8_out_partial_1 (cvt);
  utf32_to_utf8_out_partial_2 (cvt);
  utf32_to_utf8_out_error_1 (cvt);
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
utf8_to_utf16_in_ok_1 (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char16_t u16out[2] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char16_t *) nullptr;
  auto res = codecvt_base::result{};

  res = cvt.in (state, in, in + 4, in_next, u16out, u16out + 2, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == u16out + 2);
  VERIFY (char_traits<char16_t>::compare (u16out, u16in, 2) == 0);
}

void
utf8_to_utf16_in_ok_2 (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char16_t u16out[4] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char16_t *) nullptr;
  auto res = codecvt_base::result{};

  res = cvt.in (state, in, in + 8, in_next, u16out, u16out + 4, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 8);
  VERIFY (out_next == u16out + 4);
  VERIFY (char_traits<char16_t>::compare (u16out, u16in, 4) == 0);
}

void
utf8_to_utf16_in_partial_1 (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char16_t out[1] = {};
  char16_t expected[1] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char16_t *) nullptr;
  auto res = codecvt_base::result{};

  res = cvt.in (state, in, in + 3, in_next, out, out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out);
  VERIFY (in_next == in);
  VERIFY (char_traits<char16_t>::compare (out, expected, 1) == 0);
}

void
utf8_to_utf16_in_partial_2 (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char16_t out[2] = {};
  char16_t expected[2] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char16_t *) nullptr;
  auto res = codecvt_base::result{};

  res = cvt.in (state, in, in + 3, in_next, out, out + 2, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out);
  VERIFY (in_next == in);
  VERIFY (char_traits<char16_t>::compare (out, expected, 2) == 0);
}

void
utf8_to_utf16_in (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char16_t out[4] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *) nullptr;
  auto out_next = (char16_t *) nullptr;
  auto res = codecvt_base::result{};

  utf8_to_utf16_in_ok_1 (cvt);
  utf8_to_utf16_in_ok_2 (cvt);

  utf8_to_utf16_in_partial_1 (cvt);
  utf8_to_utf16_in_partial_2 (cvt);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 4, in_next, out, out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out);
  VERIFY (in_next == in);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, out, out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out);
  VERIFY (in_next == in);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, out, out + 2, out_next);
  // actual output
  // VERIFY(res == cvt.ok); // WHY? bug? should be partial
  // VERIFY(out_ptr == u16out + 2);
  // VERIFY(in_ptr == u8in + 4);
  // expected output
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, out, out + 4, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 8, in_next, out, out + 2, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 8, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == out + 2);
  VERIFY (in_next == in + 4);
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

// tests .out() function of codecvt<char16_t, char, mbstate>
void
utf16_to_utf8_out (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  auto in = u16in;
  char out[8];

  auto state = mbstate_t{};
  auto in_next = in;
  auto out_next = out;

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  auto res = cvt.out (state, in, in + 1, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in);
  VERIFY (out_next == out);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 1, in_next, out, out + 4, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in);
  VERIFY (out_next == out);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 2, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in);
  VERIFY (out_next == out);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 2, in_next, out, out + 4, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 2);
  VERIFY (out_next == out + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 3, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in);
  VERIFY (out_next == out);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 3, in_next, out, out + 4, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 2);
  VERIFY (out_next == out + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 3, in_next, out, out + 8, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 2);
  VERIFY (out_next == out + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 4, in_next, out, out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in);
  VERIFY (out_next == out);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 4, in_next, out, out + 4, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 2);
  VERIFY (out_next == out + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 4, in_next, out, out + 6, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 2);
  VERIFY (out_next == out + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.out (state, in, in + 4, in_next, out, out + 8, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 8);
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
