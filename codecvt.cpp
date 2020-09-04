#include <codecvt>
#include <cstring>
#include <iostream>
#include <locale>
#include <string>

bool global_error = false;

#define VERIFY(X) \
    do { \
        if (!(X)) { \
            global_error = true; \
            printf("Error in line: %d,\n    Function: %s,\n    Assertion: %s\n", \
                   __LINE__, \
                   __FUNCTION__, \
                   #X); \
        } \
    } while (0)

using namespace std;

// 2 code points, both are 4 byte in UTF-8.
// in UTF-16 both are 2 unit i.e. surrogate pairs
const char* u8in = u8"\U0010FFFF\U0010AAAA";

template<typename T>
std::unique_ptr<T> to_unique_ptr(T *ptr)
{
    return std::unique_ptr<T>(ptr);
}

void utf8_to_utf32_in(const codecvt<char32_t, char, mbstate_t> &cvt)
{
    char32_t u32out[2];

    // 4 byte (1cp) input buf, 1 U32 cp output buf
    auto state = mbstate_t{};
    auto in_ptr = u8in;
    auto out_ptr = u32out;
    auto res = cvt.in(state, u8in, u8in + 4, in_ptr, u32out, u32out + 1, out_ptr);
    VERIFY(res == cvt.ok);
    VERIFY(in_ptr == u8in + 4);
    VERIFY(out_ptr == u32out + 1);
    cout << hex << u32out[0] << '\n';

    // 6 byte (1cp + 1 incomplete cp) input buf, 1 U32 cp output buf
    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 6, in_ptr, u32out, u32out + 1, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u8in + 4);
    VERIFY(out_ptr == u32out + 1);

    // 6 byte (1cp + 1 incomplete cp) input buf, 2 U32 cp output buf
    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 6, in_ptr, u32out, u32out + 2, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u8in + 4);
    VERIFY(out_ptr == u32out + 1);

    // 8 byte (2cp) input buf, 1 U32 cp output buf
    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 8, in_ptr, u32out, u32out + 1, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u8in + 4);
    VERIFY(out_ptr == u32out + 1);

    // 8 byte (2cp) input buf, 2 U32 cp output buf
    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 8, in_ptr, u32out, u32out + 2, out_ptr);
    VERIFY(res == cvt.ok);
    VERIFY(in_ptr == u8in + 8);
    VERIFY(out_ptr == u32out + 2);
}

void utf8_to_utf32_in()
{
    auto &cvt = use_facet<codecvt<char32_t, char, mbstate_t>>(locale::classic());
    utf8_to_utf32_in(cvt);

    auto cvt_ptr = to_unique_ptr(new codecvt_utf8<char32_t>());
    utf8_to_utf32_in(*cvt_ptr);
}

void utf8_to_utf16_in(const codecvt<char16_t, char, mbstate_t> &cvt)
{
    char16_t u16out[4];

    auto state = mbstate_t{};
    auto in_ptr = u8in;
    auto out_ptr = u16out;

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    auto res = cvt.in(state, u8in, u8in + 3, in_ptr, u16out, u16out + 1, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out);
    VERIFY(in_ptr == u8in);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 3, in_ptr, u16out, u16out + 2, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out);
    VERIFY(in_ptr == u8in);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 4, in_ptr, u16out, u16out + 1, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out);
    VERIFY(in_ptr == u8in);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 4, in_ptr, u16out, u16out + 2, out_ptr);
    VERIFY(res == cvt.ok);
    VERIFY(out_ptr == u16out + 2);
    VERIFY(in_ptr == u8in + 4);
    cout << hex << u16out[0] << ' ' << u16out[1] << '\n';
    u16out[0] = u16out[1] = 0;

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 1, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out);
    VERIFY(in_ptr == u8in);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 2, out_ptr);
    // actual output
    // VERIFY(res == cvt.ok); // WHY? bug? should be partial
    // VERIFY(out_ptr == u16out + 2);
    // VERIFY(in_ptr == u8in + 4);
    // expected output
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out + 2);
    VERIFY(in_ptr == u8in + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 3, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out + 2);
    VERIFY(in_ptr == u8in + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 4, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out + 2);
    VERIFY(in_ptr == u8in + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 8, in_ptr, u16out, u16out + 2, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out + 2);
    VERIFY(in_ptr == u8in + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 8, in_ptr, u16out, u16out + 3, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(out_ptr == u16out + 2);
    VERIFY(in_ptr == u8in + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, u8in, u8in + 8, in_ptr, u16out, u16out + 4, out_ptr);
    VERIFY(res == cvt.ok);
    VERIFY(out_ptr == u16out + 4);
    VERIFY(in_ptr == u8in + 8);
}

void utf8_to_utf16_in()
{
    auto &cvt = use_facet<codecvt<char16_t, char, mbstate_t>>(locale::classic());
    utf8_to_utf16_in(cvt);

    auto cvt_ptr = to_unique_ptr(new codecvt_utf8_utf16<char16_t>());
    utf8_to_utf16_in(*cvt_ptr);
}

//tests .out() function of codecvt<char16_t, char, mbstate>
void utf16_to_utf8_out(const codecvt<char16_t, char, mbstate_t> &cvt)
{
    const char16_t *u16in = u"\U0010FFFF\U0010AAAA";
    char u8out[8];

    auto state = mbstate_t{};
    auto in_ptr = u16in;
    auto out_ptr = u8out;

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    auto res = cvt.out(state, u16in, u16in + 1, in_ptr, u8out, u8out + 3, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in);
    VERIFY(out_ptr == u8out);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 1, in_ptr, u8out, u8out + 4, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in);
    VERIFY(out_ptr == u8out);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 2, in_ptr, u8out, u8out + 3, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in);
    VERIFY(out_ptr == u8out);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 2, in_ptr, u8out, u8out + 4, out_ptr);
    VERIFY(res == cvt.ok);
    VERIFY(in_ptr == u16in + 2);
    VERIFY(out_ptr == u8out + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 3, in_ptr, u8out, u8out + 3, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in);
    VERIFY(out_ptr == u8out);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 3, in_ptr, u8out, u8out + 4, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in + 2);
    VERIFY(out_ptr == u8out + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 3, in_ptr, u8out, u8out + 8, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in + 2);
    VERIFY(out_ptr == u8out + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 3, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in);
    VERIFY(out_ptr == u8out);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 4, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in + 2);
    VERIFY(out_ptr == u8out + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 6, out_ptr);
    VERIFY(res == cvt.partial);
    VERIFY(in_ptr == u16in + 2);
    VERIFY(out_ptr == u8out + 4);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 8, out_ptr);
    VERIFY(res == cvt.ok);
    VERIFY(in_ptr == u16in + 4);
    VERIFY(out_ptr == u8out + 8);
}

void utf16_to_utf8_out()
{
    auto &cvt = use_facet<codecvt<char16_t, char, mbstate_t>>(locale::classic());
    utf16_to_utf8_out(cvt);

    auto cvt_ptr = to_unique_ptr(new codecvt_utf8_utf16<char16_t>());
    utf16_to_utf8_out(*cvt_ptr);
}

void utf8_to_ucs2_in()
{
    // 2 code points, one is 3 bytes and the other is 4 bytes in UTF-8.
    // in UTF-16 the first is sinlge unit, the second is surrogate pair
    // in UCS2 only the first CP is allowed.
    const char *in = u8"\uAAAA\U0010AAAA";
    char16_t out[2] = {'y', 'y'};

    auto cvt_ptr = to_unique_ptr(new codecvt_utf8<char16_t>());
    auto &cvt = *cvt_ptr;
    auto state = mbstate_t{};
    auto in_ptr = in;
    auto out_ptr = out;

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    auto res = cvt.in(state, in, in + 2, in_ptr, out, out, out_ptr);
    VERIFY(res == cvt.partial); //BUG, returns OK, should be Partial
    VERIFY(out_ptr == out);
    VERIFY(in_ptr == in);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, in, in + 2, in_ptr, out, out + 1, out_ptr);
    VERIFY(res == cvt.partial); // BUG, returns ERROR, should be Partial
    VERIFY(out_ptr == out);
    VERIFY(in_ptr == in);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, in, in + 3, in_ptr, out, out, out_ptr);
    VERIFY(res == cvt.partial); //BUG, return OK, should be Partial
    VERIFY(out_ptr == out);
    VERIFY(in_ptr == in);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, in, in + 3, in_ptr, out, out + 1, out_ptr);
    VERIFY(res == cvt.ok);
    VERIFY(out_ptr == out + 1);
    VERIFY(in_ptr == in + 3);
    cout << "UCS2 sequence: " << hex << out[0] << ' ' << out[1] << '\n';

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, in, in + 6, in_ptr, out, out + 1, out_ptr);
    VERIFY(res == cvt.partial); // BUG, return OK, should be Partial
    VERIFY(out_ptr == out + 1);
    VERIFY(in_ptr == in + 3);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, in, in + 6, in_ptr, out, out + 2, out_ptr);
    VERIFY(res == cvt.partial); // BUG, returns ERROR, should be Partial
    VERIFY(out_ptr == out + 1);
    VERIFY(in_ptr == in + 3);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, in, in + 7, in_ptr, out, out + 1, out_ptr);
    VERIFY(res == cvt.partial); // BUG, returns OK, should be Partial
    VERIFY(out_ptr == out + 1);
    VERIFY(in_ptr == in + 3);

    state = {};
    in_ptr = nullptr;
    out_ptr = nullptr;
    res = cvt.in(state, in, in + 7, in_ptr, out, out + 2, out_ptr);
    VERIFY(res == cvt.error);
    VERIFY(out_ptr == out + 1);
    VERIFY(in_ptr == in + 3);
}

int main()
{
    utf8_to_utf32_in();
    utf8_to_utf16_in();

    utf16_to_utf8_out();

    utf8_to_ucs2_in();
    return global_error;
}
