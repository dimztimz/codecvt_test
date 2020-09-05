// Copyright 2020 Dimitrij Mijoski
// SPDX-License-Identifier: GPL-3.0-or-later

#include <codecvt>
#include <cstring>
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

// template <class T, size_t N> size_t len (T (&)[N]) { return N - 1; }

template <typename T>
std::unique_ptr<T>
to_unique_ptr (T *ptr)
{
  return std::unique_ptr<T> (ptr);
}

// 4 byte (1cp) input buf, 1 U32 cp output buf
void
utf8_to_utf32_in_ok_1 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char32_t out[1] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *){};
  auto out_next = (char32_t *){};
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 4, in_next, out, out + 1, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0]);
}

// 8 byte (2cp) input buf, 2 U32 cp output buf
void
utf8_to_utf32_in_ok_2 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char32_t out[2] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *){};
  auto out_next = (char32_t *){};
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 2, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (in_next == in + 8);
  VERIFY (out_next == out + 2);
  VERIFY (out[0] == u32in[0] && out[1] == u32in[1]);
}

// 6 byte (1cp + 1 incomplete cp) input buf, 1 U32 cp output buf
void
utf8_to_utf32_in_partial_1 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char32_t out[1] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *){};
  auto out_next = (char32_t *){};
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 6, in_next, out, out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0]);
}

// 6 byte (1cp + 1 incomplete cp) input buf, 2 U32 cp output buf
void
utf8_to_utf32_in_partial_2 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char32_t out[1] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *){};
  auto out_next = (char32_t *){};
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 6, in_next, out, out + 2, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0]);
}

// 8 byte (2cp) input buf, 1 U32 cp output buf
void
utf8_to_utf32_in_partial_3 (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char32_t out[1] = {};

  auto state = mbstate_t{};
  auto in_next = (const char *){};
  auto out_next = (char32_t *){};
  auto res = codecvt_base::result ();

  res = cvt.in (state, in, in + 8, in_next, out, out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (in_next == in + 4);
  VERIFY (out_next == out + 1);
  VERIFY (out[0] == u32in[0]);
}

void
utf8_to_utf32_in (const codecvt<char32_t, char, mbstate_t> &cvt)
{
  utf8_to_utf32_in_ok_1 (cvt);
  utf8_to_utf32_in_ok_2 (cvt);
  utf8_to_utf32_in_partial_1 (cvt);
  utf8_to_utf32_in_partial_2 (cvt);
  utf8_to_utf32_in_partial_3 (cvt);
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
utf8_to_utf16_in (const codecvt<char16_t, char, mbstate_t> &cvt)
{
  auto in = u8in;
  char16_t u16out[4];

  auto state = mbstate_t{};
  auto in_next = u8in;
  auto out_next = u16out;

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  auto res = cvt.in (state, in, in + 3, in_next, u16out, u16out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out);
  VERIFY (in_next == in);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 3, in_next, u16out, u16out + 2, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out);
  VERIFY (in_next == in);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 4, in_next, u16out, u16out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out);
  VERIFY (in_next == in);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 4, in_next, u16out, u16out + 2, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (out_next == u16out + 2);
  VERIFY (in_next == in + 4);
  cout << hex << u16out[0] << ' ' << u16out[1] << '\n';
  u16out[0] = u16out[1] = 0;

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, u16out, u16out + 1, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out);
  VERIFY (in_next == in);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, u16out, u16out + 2, out_next);
  // actual output
  // VERIFY(res == cvt.ok); // WHY? bug? should be partial
  // VERIFY(out_ptr == u16out + 2);
  // VERIFY(in_ptr == u8in + 4);
  // expected output
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, u16out, u16out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 6, in_next, u16out, u16out + 4, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 8, in_next, u16out, u16out + 2, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 8, in_next, u16out, u16out + 3, out_next);
  VERIFY (res == cvt.partial);
  VERIFY (out_next == u16out + 2);
  VERIFY (in_next == in + 4);

  state = {};
  in_next = nullptr;
  out_next = nullptr;
  res = cvt.in (state, in, in + 8, in_next, u16out, u16out + 4, out_next);
  VERIFY (res == cvt.ok);
  VERIFY (out_next == u16out + 4);
  VERIFY (in_next == in + 8);
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
  const char16_t *u16in = u"\U0010FFFF\U0010AAAA";
  char u8out[8];

  auto state = mbstate_t{};
  auto in_ptr = u16in;
  auto out_ptr = u8out;

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  auto res
    = cvt.out (state, u16in, u16in + 1, in_ptr, u8out, u8out + 3, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in);
  VERIFY (out_ptr == u8out);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 1, in_ptr, u8out, u8out + 4, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in);
  VERIFY (out_ptr == u8out);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 2, in_ptr, u8out, u8out + 3, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in);
  VERIFY (out_ptr == u8out);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 2, in_ptr, u8out, u8out + 4, out_ptr);
  VERIFY (res == cvt.ok);
  VERIFY (in_ptr == u16in + 2);
  VERIFY (out_ptr == u8out + 4);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 3, in_ptr, u8out, u8out + 3, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in);
  VERIFY (out_ptr == u8out);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 3, in_ptr, u8out, u8out + 4, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in + 2);
  VERIFY (out_ptr == u8out + 4);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 3, in_ptr, u8out, u8out + 8, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in + 2);
  VERIFY (out_ptr == u8out + 4);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 4, in_ptr, u8out, u8out + 3, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in);
  VERIFY (out_ptr == u8out);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 4, in_ptr, u8out, u8out + 4, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in + 2);
  VERIFY (out_ptr == u8out + 4);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 4, in_ptr, u8out, u8out + 6, out_ptr);
  VERIFY (res == cvt.partial);
  VERIFY (in_ptr == u16in + 2);
  VERIFY (out_ptr == u8out + 4);

  state = {};
  in_ptr = nullptr;
  out_ptr = nullptr;
  res = cvt.out (state, u16in, u16in + 4, in_ptr, u8out, u8out + 8, out_ptr);
  VERIFY (res == cvt.ok);
  VERIFY (in_ptr == u16in + 4);
  VERIFY (out_ptr == u8out + 8);
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
  VERIFY (res == cvt.partial); // BUG, returns ERROR, should be Partial
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
  utf8_to_utf16_in ();

  utf16_to_utf8_out ();

  utf8_to_ucs2_in ();
  return global_error;
}
