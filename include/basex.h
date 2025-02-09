/**
 * basex
 * template <typename InputIterator, typename OutputIterator>
 * OutputIterator bxencode(InputIterator beg, InputIterator end, OutputIterator result, const std::string &digits);
 * template <typename InputIterator, typename OutputIterator>
 * OutputIterator bxdecode(InputIterator beg, InputIterator end, OutputIterator result, const std::string &digits);
 *
 * base64
 * template <typename InputIterator, typename OutputIterator>
 * OutputIterator b64:encode(InputIterator beg, InputIterator end, OutputIterator result);
 * template <typename InputIterator, typename OutputIterator>
 * OutputIterator b64:decode(InputIterator beg, InputIterator end, OutputIterator result);
 *
 * base58
 * template <typename InputIterator, typename OutputIterator>
 * OutputIterator b58:encode(InputIterator beg, InputIterator end, OutputIterator result);
 * template <typename InputIterator, typename OutputIterator>
 * OutputIterator b58:decode(InputIterator beg, InputIterator end, OutputIterator result);
 */

#ifndef _BASEX_H_
#define _BASEX_H_

#include <algorithm>
#include <istream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

namespace basex
{
template <typename InputIterator, typename OutputIterator>
OutputIterator bxencode_base_conversion(InputIterator beg, InputIterator end, OutputIterator result, const std::string &digits)
{
    if (beg == end) { return result; }

    const auto base = digits.size();
    using difference_type = typename std::iterator_traits<InputIterator>::difference_type;
    std::vector<uint8_t> bx;  // NUMBER[beg, end) == \sum (bx[i] \times base^i)
    for (difference_type length = 0; beg != end; ++beg) {
        // https://github.com/bitcoin/bitcoin/blob/v28.1/src/base58.cpp#L102
        uint32_t carry = *beg;
        difference_type new_length = 0;
        // bx = bx * 256 + ch
        for (auto i = 0; carry != 0 || i < length; ++i, ++new_length) {
            if (i < length) {
                carry += bx[i] << 8;
                bx[i] = carry % base;
            } else {
                bx.push_back(carry % base);
            }
            carry /= base;
        }
        length = new_length;
    }

    // bx[0] is the least significant digit of NUMBER[beg, end), so it should be placed at the rightmost of text representation
    for (auto it = bx.rbegin(); it != bx.rend(); ++it) { *result++ = digits[*it]; }
    return result;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator bxdecode_base_conversion(InputIterator beg, InputIterator end, OutputIterator result, const unsigned base, const int8_t (&decmap)[256])
{
    if (beg == end) { return result; }

    using difference_type = typename std::iterator_traits<InputIterator>::difference_type;
    std::vector<uint8_t> bx;
    for (difference_type length = 0; beg != end; ++beg) {
        uint32_t carry = decmap[static_cast<unsigned char>(*beg)];
        difference_type new_length = 0;
        // bx = bx * base + ch
        for (auto i = 0; carry != 0 || i < length; ++i, ++new_length) {
            if (i < length) {
                carry += bx[i] * base;
                bx[i] = carry & 255;
            } else {
                bx.push_back(carry & 255);
            }
            carry >>= 8;
        }
        length = new_length;
    }

    return std::copy(bx.rbegin(), bx.rend(), result);
}

inline size_t log2(size_t i) noexcept
{
    size_t result = 0;
    while (i >>= 1) { ++result; }
    return result;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator bxencode_pow2(InputIterator beg, InputIterator end, OutputIterator result, const std::string &digits)
{
    int idx = log2(digits.size());  // 1 <= idx <= 7
    auto nbits = std::lcm(idx, 8);  // 8 <= lcm <= 56
    auto nbytes = nbits >> 3;
    auto ndigits = nbits / idx;
    uint8_t mask = (1U << idx) - 1;
    while (beg != end) {
        uint64_t bits = 0;
        for (int i = 0; i != nbytes && beg != end; ++i) { bits = bits << 8 | *beg++; }
        char buffer[ndigits];
        for (int i = 0; i != ndigits; ++i) {
            buffer[i] = digits[bits & mask];
            bits >>= idx;
        }
        reverse_copy(buffer, buffer + ndigits, result);
    }
    return result;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator bxdecode_pow2(InputIterator beg, InputIterator end, OutputIterator result, const unsigned base, const int8_t (&decmap)[256])
{
    int idx = log2(base);           // 1 <= idx <= 7
    auto nbits = std::lcm(idx, 8);  // 8 <= lcm <= 56
    auto nbytes = nbits >> 3;
    auto ndigits = nbits / idx;
    while (beg != end) {
        uint64_t bits = 0;
        for (int i = 0; i != ndigits && beg != end; ++i) { bits = bits << idx | decmap[static_cast<unsigned char>(*beg++)]; }
        for (int i = (nbytes - 1) << 3; i >= 0; i -= 8) { *result++ = bits >> i & 255; }
    }
    return result;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator bxencode(InputIterator beg, InputIterator end, OutputIterator result, const std::string &digits)
{
    auto size = digits.size();
    if (size & (size - 1)) {  // not power of 2
        return bxencode_base_conversion(beg, end, result, digits);
    } else {
        return bxencode_pow2(beg, end, result, digits);
    }
}

template <typename InputIterator, typename OutputIterator>
OutputIterator bxdecode(InputIterator beg, InputIterator end, OutputIterator result, const unsigned base, const int8_t (&decmap)[256])
{
    if (base & (base - 1)) {  // not power of 2
        return bxdecode_base_conversion(beg, end, result, base, decmap);
    } else {
        return bxdecode_pow2(beg, end, result, base, decmap);
    }
}

template <typename InputIterator, typename OutputIterator>
OutputIterator bxdecode(InputIterator beg, InputIterator end, OutputIterator result, const std::string &digits)
{
    int8_t decmap[256];
    std::fill(decmap, std::end(decmap), -1);
    for (unsigned i = 0; i != digits.size(); ++i) { decmap[static_cast<unsigned>(digits[i])] = i; }
    return bxdecode(beg, end, result, digits.size(), decmap);
}

namespace b64
{
// To make this library header only, digits and decmap are DEFINED here with internal linkage
const std::string digits = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const int8_t decmap[256] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
                            -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
                            -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

template <typename BidirectionalIterator>
inline BidirectionalIterator lastn(BidirectionalIterator beg, BidirectionalIterator end,
                                   typename std::iterator_traits<BidirectionalIterator>::difference_type n, std::bidirectional_iterator_tag)
{
    std::advance(end, -n);
    return end;
}

template <typename ForwardIterator>
inline ForwardIterator lastn(ForwardIterator beg, ForwardIterator end, typename std::iterator_traits<ForwardIterator>::difference_type n,
                             std::forward_iterator_tag)
{
    auto result = beg;
    while (n--) { ++beg; }
    while (beg != end) { ++result; }
    return result;
}

template <typename ForwardIterator>
inline ForwardIterator lastn(ForwardIterator beg, ForwardIterator end, typename std::iterator_traits<ForwardIterator>::difference_type n)
{
    return lastn(beg, end, n, typename std::iterator_traits<ForwardIterator>::iterator_category());
}

template <typename ForwardIterator, typename OutputIterator>
OutputIterator encode(ForwardIterator beg, ForwardIterator end, OutputIterator result, std::forward_iterator_tag)
{
    auto size = std::distance(beg, end);
    auto nremainder = size % 3;
    if (nremainder) { end = lastn(beg, end, nremainder); }
    result = bxencode(beg, end, result, digits);
    if (nremainder) {
        char buffer[] = "====";
        uint32_t remainder;
        if (nremainder == 1) {
            remainder = *end << 4;
        } else {
            remainder = *end++ << 8;
            remainder |= *end;
            remainder <<= 2;
        }
        for (int i = 0; i != nremainder + 1; ++i) {
            buffer[nremainder - i] = digits[remainder & 63];
            // bitshift is defined in terms of values, not representations
            remainder >>= 6;
        }
        std::copy_n(buffer, 4, result);
    }
    return result;
}

template <typename ForwardIterator, typename OutputIterator>
OutputIterator decode(ForwardIterator beg, ForwardIterator end, OutputIterator result, std::forward_iterator_tag, bool has_padding)
{
    if (has_padding) { end = lastn(beg, end, 4); }
    result = bxdecode(beg, end, result, 64, decmap);
    if (has_padding) {
        uint32_t bits = 0;
        unsigned i = 0;
        for (; i != 4; ++i) {
            uint8_t c = *end++;
            if (c == '=') { break; }
            bits = bits << 6 | decmap[c];
        }
        if (i != 4) { bits >>= (i == 2) ? 4 : 2; }
        --i;
        while (i) { *result++ = (bits >> (--i << 3)) & 255; }
    }
    return result;
}


template <typename InputIterator, typename OutputIterator>
OutputIterator buffered_codec(InputIterator beg, InputIterator end, OutputIterator result, bool is_encode)
{
    std::vector<uint8_t> buffer(3 << 8);
    while (beg != end) {
        unsigned i = 0;
        for (; i != buffer.size() && beg != end; ++i) { buffer[i] = *beg++; }
        if (is_encode) {
            result = encode(buffer.cbegin(), buffer.cbegin() + i, result, std::random_access_iterator_tag());
        } else {
            if (i & 3 && !(i &= ~3U)) {
                break;  // trailing '\n' and other garbages are ignored
            }
            result = decode(buffer.cbegin(), buffer.cbegin() + i, result, std::random_access_iterator_tag(), i != buffer.size());
        }
    }
    return result;
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator encode(InputIterator beg, InputIterator end, OutputIterator result, std::input_iterator_tag)
{
    return buffered_codec(beg, end, result, true);
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator decode(InputIterator beg, InputIterator end, OutputIterator result, std::input_iterator_tag)
{
    return buffered_codec(beg, end, result, false);
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator encode(InputIterator beg, InputIterator end, OutputIterator result)
{
    return encode(beg, end, result, typename std::iterator_traits<InputIterator>::iterator_category());
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator decode(InputIterator beg, InputIterator end, OutputIterator result)
{
    return decode(beg, end, result, typename std::iterator_traits<InputIterator>::iterator_category());
}
}  // namespace b64

namespace b58
{
// see comments of b64::digits/decmap for the reason why they are DEFINED here
const std::string digits = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
const int8_t decmap[256] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  -1, -1, -1, -1, -1, -1,
                            -1, 9,  10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1,
                            -1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

template <typename InputIterator, typename OutputIterator>
inline OutputIterator encode(InputIterator beg, InputIterator end, OutputIterator result)
{
    return bxencode(beg, end, result, digits);
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator decode(InputIterator beg, InputIterator end, OutputIterator result)
{
    return bxdecode(beg, end, result, 58, decmap);
}
}  // namespace b58
}  // namespace basex

#endif
