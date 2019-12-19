#include "ap_int.h"
//-------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------
const unsigned CONVOLVERS = 2;

const unsigned WORD_SIZE = 64;
const unsigned WT_SIZE = 9;
const unsigned CONV_W_PER_WORD = 7;
const unsigned CONV1_W_PER_WORD = 4;
const unsigned KH_PER_WORD = 4;
const unsigned BYTE_SIZE = 8;
const unsigned K = 3;
const unsigned WT_L = 16 * 4 * 512; // parameter to control wt mem size
const unsigned C_WT_WORDS = ((WT_L + CONV_W_PER_WORD - 1) / CONV_W_PER_WORD + CONVOLVERS - 1) /
                            CONVOLVERS; // wt words per convolver
const unsigned WT_WORDS = C_WT_WORDS * CONVOLVERS;
const unsigned KH_WORDS = WT_L / 128 * 16 / WORD_SIZE;

const unsigned DMEM_WORDS = 128 * 32 * 32 / WORD_SIZE;
const unsigned C_DMEM_WORDS = DMEM_WORDS / CONVOLVERS;
const unsigned DMEM_O_WORDS = 512 * 4 * 4 / WORD_SIZE;
const unsigned DB_MEM_WORDS = 32 * 32;

const unsigned PIX_PER_PHASE = 2 * 32 * 32;

const unsigned MAX_WIDTH = WORD_SIZE;
const unsigned BANK_WIDTH = 8;
const unsigned LOG_BANK_WIDTH = 3;

const unsigned CONV_ROWS = 3;
const unsigned CONV_COLS = BANK_WIDTH + 2;
const unsigned CONV_BANKS = WORD_SIZE / BANK_WIDTH;

//-------------------------------------------------------------------
// Typedefs
//-------------------------------------------------------------------
enum LayerTypeEnum
{
    LAYER_CONV1,
    LAYER_CONV,
    LAYER_DENSE,
    LAYER_LAST
};

typedef ap_int<WORD_SIZE> Word;
typedef ap_int<WT_SIZE> WtType;
typedef ap_uint<16> Address;
typedef ap_int<12> ConvSum;
typedef ap_int<5> ConvOut;
typedef ap_uint<10> IdxType;
typedef ap_int<16> NormComp;
typedef ap_int<16> DenseSum;
typedef ap_uint<1> Bit;
typedef ap_uint<2> TwoBit;
