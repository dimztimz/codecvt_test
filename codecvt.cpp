#include <cassert>
#include <codecvt>
#include <cstring>
#include <iostream>
#include <locale>
#include <string>

using namespace std;

// 2 code points, both are 4 byte in UTF-8.
// in UTF-16 both are 2 unit i.e. surrogate pairs
const char* u8in = u8"\U0010FFFF\U0010AAAA";

//tests .in() function of codecvt<char32_t, char, mbstate>
void test_u32_in()
{
	char32_t u32out[2];

	auto& cvt =
	    use_facet<codecvt<char32_t, char, mbstate_t>>(locale::classic());

	// 4 byte (1cp) input buf, 1 U32 cp output buf
	auto state = mbstate_t{};
	auto in_ptr = u8in;
	auto out_ptr = u32out;
	auto res =
	    cvt.in(state, u8in, u8in + 4, in_ptr, u32out, u32out + 1, out_ptr);
	assert(res == cvt.ok);
	assert(in_ptr == u8in + 4);
	assert(out_ptr == u32out + 1);
	cout << hex << u32out[0] << '\n';

	// 6 byte (1cp + 1 incomplete cp) input buf, 1 U32 cp output buf
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 6, in_ptr, u32out, u32out + 1, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u8in + 4);
	assert(out_ptr == u32out + 1);

	// 6 byte (1cp + 1 incomplete cp) input buf, 2 U32 cp output buf
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 6, in_ptr, u32out, u32out + 2, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u8in + 4);
	assert(out_ptr == u32out + 1);

	// 8 byte (2cp) input buf, 1 U32 cp output buf
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 8, in_ptr, u32out, u32out + 1, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u8in + 4);
	assert(out_ptr == u32out + 1);

	// 8 byte (2cp) input buf, 2 U32 cp output buf
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 8, in_ptr, u32out, u32out + 2, out_ptr);
	assert(res == cvt.ok);
	assert(in_ptr == u8in + 8);
	assert(out_ptr == u32out + 2);
}

//tests .in() function of codecvt<char16_t, char, mbstate>
void test_u16_in()
{

	char16_t u16out[4];

	auto& cvt =
	    use_facet<codecvt<char16_t, char, mbstate_t>>(locale::classic());
	auto state = mbstate_t{};
	auto in_ptr = u8in;
	auto out_ptr = u16out;

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	auto res =
	    cvt.in(state, u8in, u8in + 3, in_ptr, u16out, u16out + 1, out_ptr);
	assert(res == cvt.partial);
	assert(out_ptr == u16out);
	assert(in_ptr == u8in);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 3, in_ptr, u16out, u16out + 2, out_ptr);
	assert(res == cvt.partial);
	assert(out_ptr == u16out);
	assert(in_ptr == u8in);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 4, in_ptr, u16out, u16out + 1, out_ptr);
	assert(res == cvt.partial);
	assert(out_ptr == u16out);
	assert(in_ptr == u8in);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 4, in_ptr, u16out, u16out + 2, out_ptr);
	assert(res == cvt.ok);
	assert(out_ptr == u16out + 2);
	assert(in_ptr == u8in + 4);
	cout << hex << u16out[0] << ' ' << u16out[1] << '\n';
	u16out[0] = u16out[1] = 0;

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 1, out_ptr);
	assert(res == cvt.partial);
	assert(out_ptr == u16out);
	assert(in_ptr == u8in);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 2, out_ptr);
	// actual output
	// assert(res == cvt.ok); // WHY? bug? should be partial
	// assert(out_ptr == u16out + 2);
	// assert(in_ptr == u8in + 4);
	// expected output
	assert(res == cvt.partial);
	assert(out_ptr == u16out + 2);
	assert(in_ptr == u8in + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 3, out_ptr);
	assert(res == cvt.partial);
	assert(out_ptr == u16out + 2);
	assert(in_ptr == u8in + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 6, in_ptr, u16out, u16out + 4, out_ptr);
	assert(res == cvt.partial);
	assert(out_ptr == u16out + 2);
	assert(in_ptr == u8in + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 8, in_ptr, u16out, u16out + 2, out_ptr);
	// actual output
	// assert(res == cvt.ok); // WHY? bug? should be partial
	// assert(out_ptr == u16out + 2);
	// assert(in_ptr == u8in + 4);
	// expected output
	assert(res == cvt.partial);
	assert(out_ptr == u16out + 2);
	assert(in_ptr == u8in + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 8, in_ptr, u16out, u16out + 3, out_ptr);
	assert(res == cvt.partial);
	assert(out_ptr == u16out + 2);
	assert(in_ptr == u8in + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.in(state, u8in, u8in + 8, in_ptr, u16out, u16out + 4, out_ptr);
	assert(res == cvt.ok);
	assert(out_ptr == u16out + 4);
	assert(in_ptr == u8in + 8);
}

//tests .out() function of codecvt<char16_t, char, mbstate>
void test_u16_out()
{
	const char16_t* u16in = u"\U0010FFFF\U0010AAAA";
	char u8out[8];

	auto& cvt =
	    use_facet<codecvt<char16_t, char, mbstate_t>>(locale::classic());
	auto state = mbstate_t{};
	auto in_ptr = u16in;
	auto out_ptr = u8out;

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	auto res =
	    cvt.out(state, u16in, u16in + 1, in_ptr, u8out, u8out + 3, out_ptr);
	// actual output
	// assert(res == cvt.ok); // WHY? Bug, should be partial
	// expected
	assert(res == cvt.partial);
	assert(in_ptr == u16in);
	assert(out_ptr == u8out);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 1, in_ptr, u8out, u8out + 4, out_ptr);
	// assert(res == cvt.ok); // WHY? Bug, should be partial
	// expected
	assert(res == cvt.partial);
	assert(in_ptr == u16in);
	assert(out_ptr == u8out);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 2, in_ptr, u8out, u8out + 3, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u16in);
	assert(out_ptr == u8out);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 2, in_ptr, u8out, u8out + 4, out_ptr);
	assert(res == cvt.ok);
	assert(in_ptr == u16in + 2);
	assert(out_ptr == u8out + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 3, in_ptr, u8out, u8out + 3, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u16in);
	assert(out_ptr == u8out);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 3, in_ptr, u8out, u8out + 4, out_ptr);
	// actual output
	// assert(res == cvt.ok); // WHY? bug, should be partial
	// expected
	assert(res == cvt.partial);
	assert(in_ptr == u16in + 2);
	assert(out_ptr == u8out + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 3, in_ptr, u8out, u8out + 8, out_ptr);
	// actual output
	// assert(res == cvt.ok); // WHY? bug, should be partial
	// expected
	assert(res == cvt.partial);
	assert(in_ptr == u16in + 2);
	assert(out_ptr == u8out + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 3, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u16in);
	assert(out_ptr == u8out);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 4, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u16in + 2);
	assert(out_ptr == u8out + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 6, out_ptr);
	assert(res == cvt.partial);
	assert(in_ptr == u16in + 2);
	assert(out_ptr == u8out + 4);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res =
	    cvt.out(state, u16in, u16in + 4, in_ptr, u8out, u8out + 8, out_ptr);
	assert(res == cvt.ok);
	assert(in_ptr == u16in + 4);
	assert(out_ptr == u8out + 8);
}

template<typename T>
std::unique_ptr<T> to_unique_ptr(T *ptr)
{
    return std::unique_ptr<T>(ptr);
}

void test_u8_ucs2_in()
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
	assert(res == cvt.partial); //BUG, returns OK, should be Partial 
	assert(out_ptr == out);
	assert(in_ptr == in);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res = cvt.in(state, in, in + 2, in_ptr, out, out + 1, out_ptr);
	assert(res == cvt.partial); // BUG, returns ERROR, should be Partial
	assert(out_ptr == out);
	assert(in_ptr == in);

	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res = cvt.in(state, in, in + 3, in_ptr, out, out, out_ptr);
	assert(res == cvt.partial); //BUG, return OK, should be Partial
	assert(out_ptr == out);
	assert(in_ptr == in);
	
	
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res = cvt.in(state, in, in + 3, in_ptr, out, out + 1, out_ptr);
	assert(res == cvt.ok);
	assert(out_ptr == out + 1);
	assert(in_ptr == in + 3);
	cout << "UCS2 sequence: " << hex << out[0] << ' ' << out[1] << '\n';
	
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res = cvt.in(state, in, in + 6, in_ptr, out, out + 1, out_ptr);
	assert(res == cvt.partial); // BUG, return OK, should be Partial
	assert(out_ptr == out + 1);
	assert(in_ptr == in + 3);
	
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res = cvt.in(state, in, in + 6, in_ptr, out, out + 2, out_ptr);
	assert(res == cvt.partial); // BUG, returns ERROR, should be Partial
	assert(out_ptr == out + 1);
	assert(in_ptr == in + 3);
	
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res = cvt.in(state, in, in + 7, in_ptr, out, out + 1, out_ptr);
	assert(res == cvt.partial); // BUG, returns OK, should be Partial
	assert(out_ptr == out + 1);
	assert(in_ptr == in + 3);
	
	state = {};
	in_ptr = nullptr;
	out_ptr = nullptr;
	res = cvt.in(state, in, in + 7, in_ptr, out, out + 2, out_ptr);
	assert(res == cvt.error);
	assert(out_ptr == out + 1);
	assert(in_ptr == in + 3);
}

int main()
{
    test_u32_in();
    test_u16_in();

    test_u16_out();

    test_u8_ucs2_in();
	return 0;
}
