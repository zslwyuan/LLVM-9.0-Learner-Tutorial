#ifndef _APINT
#define _APINT
#define NULL __null

template <int Bits> using ap_uint = __attribute__((__ap_int(Bits))) unsigned;

template <int Bits> using ap_int = __attribute__((__ap_int(Bits))) int;

template <typename F>
inline __attribute__((always_inline)) F partSelect(F input, const int l, const int r)
{
    return ((input) & ((1 << (l + 1)) - 1)) >> r;
}

#endif
