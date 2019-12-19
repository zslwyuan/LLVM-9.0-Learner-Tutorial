#include "bin_conv.h"

TwoBit encode_bit(const Bit &b)
{
    return (b == 0) ? TwoBit(1) : TwoBit(-1);
}

// -----------------------------------------------------------------------
// Conv
// -----------------------------------------------------------------------
ConvOut conv3x3b(const TwoBit line_buffer_m[CONV_BANKS][CONV_ROWS][CONV_COLS],
                 const Bit conv_params_m[K][K], const ap_uint<4> bank, const IdxType cc)
{
    ConvOut sum = 0;
    for (ap_uint<2> kr = 0; kr < K; ++kr)
    {
        for (ap_uint<2> kc = 0; kc < K; ++kc)
        {
            TwoBit data = line_buffer_m[bank][kr][cc + kc];
            const Bit &wt = conv_params_m[2 - kr][2 - kc];
            data = ((wt & partSelect(data, 0, 0) ^ partSelect(data, 1, 1)) << 1) |
                   partSelect(data, 0, 0);
            sum += data;
        }
    }
    return sum;
}

// -----------------------------------------------------------------------
// Produce 32 elements of conv results
// -----------------------------------------------------------------------
void conv_word(const TwoBit line_buffer_m[CONV_BANKS][CONV_ROWS][CONV_COLS],
               const Bit conv_params_m[K][K], ConvOut conv_out_buffer_m[WORD_SIZE])
{
    for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank)
    {
        for (ap_uint<4> cc = 0; cc < BANK_WIDTH; ++cc)
        {
            conv_out_buffer_m[bank * BANK_WIDTH + cc] =
                conv3x3b(line_buffer_m, conv_params_m, bank, cc);
        }
    }
}

// -----------------------------------------------------------------------
// Process each line in a word, we need to outline this loop to
// avoid false control dependencies in Vivado HLS
// -----------------------------------------------------------------------
void process_word(const TwoBit word_buffer_m[CONV_BANKS][CONV_COLS],
                  const TwoBit old_word_buffer_m[CONV_BANKS][CONV_COLS], const bool lb[CONV_BANKS],
                  const bool rb[CONV_BANKS], TwoBit line_buffer_m[CONV_BANKS][CONV_ROWS][CONV_COLS],
                  const Bit conv_params_m[K][K], ConvOut conv_out_buffer_m[WORD_SIZE],
                  const ap_uint<3> log_width, const ap_uint<6> words_per_image, const IdxType wrd)
{
    // slices_per_line = width / BANK_WIDTH
    const ap_uint<5> slices_per_line = 1 << (log_width - LOG_BANK_WIDTH);
    const bool first_wrd = (wrd == 0);
    const bool last_wrd = (wrd == words_per_image);

    // Prologue
    // Update bottom row, slices are shifted left. Some slices copied from
    // previous word (middle row)
    for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank)
    {
        ap_int<6> s_idx = bank + slices_per_line - CONV_BANKS;
        if (s_idx < 0)
        {
            // set to zero or copy from old word (middle row)
            for (ap_uint<4> cc = 1; cc < CONV_COLS - 1; ++cc)
            {
                line_buffer_m[bank][CONV_ROWS - 1][cc] = old_word_buffer_m[CONV_BANKS + s_idx][cc];
            }
            line_buffer_m[bank][CONV_ROWS - 1][0] =
                lb[bank] ? TwoBit(0) : old_word_buffer_m[CONV_BANKS + s_idx][0];
            line_buffer_m[bank][CONV_ROWS - 1][CONV_COLS - 1] =
                rb[bank] ? TwoBit(0) : old_word_buffer_m[CONV_BANKS + s_idx][CONV_COLS - 1];
        }
        else
        {
            // fill from new word
            for (ap_uint<4> cc = 1; cc < CONV_COLS - 1; ++cc)
            {
                line_buffer_m[bank][CONV_ROWS - 1][cc] =
                    (last_wrd) ? TwoBit(0) : word_buffer_m[s_idx][cc];
            }
            line_buffer_m[bank][CONV_ROWS - 1][0] =
                (last_wrd || lb[bank]) ? TwoBit(0) : word_buffer_m[s_idx][0];
            line_buffer_m[bank][CONV_ROWS - 1][CONV_COLS - 1] =
                (last_wrd || rb[bank]) ? TwoBit(0) : word_buffer_m[s_idx][CONV_COLS - 1];
        }
    }

    // Convolution
    conv_word(line_buffer_m, conv_params_m, conv_out_buffer_m);

    // Update
    // Fill line buffer with lines from the new word
    for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank)
    {
        // --------------------------------------------------------------
        // Top row, slices are shifted right by slices_per_line
        ap_int<6> s_idx0 = bank - slices_per_line;
        if (s_idx0 >= 0)
        {
            // slice from input word
            for (ap_uint<4> cc = 1; cc < CONV_COLS - 1; ++cc)
            {
                line_buffer_m[bank][0][cc] = word_buffer_m[s_idx0][cc];
            }
            line_buffer_m[bank][0][0] = lb[bank] ? TwoBit(0) : word_buffer_m[s_idx0][0];
            line_buffer_m[bank][0][CONV_COLS - 1] =
                rb[bank] ? TwoBit(0) : word_buffer_m[s_idx0][CONV_COLS - 1];
        }
        else
        {
            // set to zero or copy from old word (middle row)
            for (ap_uint<4> cc = 1; cc < CONV_COLS - 1; ++cc)
            {
                line_buffer_m[bank][0][cc] =
                    (first_wrd) ? TwoBit(0) : old_word_buffer_m[CONV_BANKS + s_idx0][cc];
            }
            line_buffer_m[bank][0][0] =
                (first_wrd || lb[bank]) ? TwoBit(0) : old_word_buffer_m[CONV_BANKS + s_idx0][0];
            line_buffer_m[bank][0][CONV_COLS - 1] =
                (first_wrd || rb[bank]) ? TwoBit(0)
                                        : old_word_buffer_m[CONV_BANKS + s_idx0][CONV_COLS - 1];
        }

        // --------------------------------------------------------------
        // Middle row, simply copy the word into the line buffer
        for (ap_uint<4> cc = 1; cc < CONV_COLS - 1; ++cc)
        {
            line_buffer_m[bank][1][cc] = word_buffer_m[bank][cc];
        }
        // Fill end buffer bits
        line_buffer_m[bank][1][0] = lb[bank] ? TwoBit(0) : word_buffer_m[bank][0];
        line_buffer_m[bank][1][CONV_COLS - 1] =
            rb[bank] ? TwoBit(0) : word_buffer_m[bank][CONV_COLS - 1];
    }
}

// -----------------------------------------------------------------------
// A single PE reads from all inputs and weights to generate a single
// output feature map.
// * Make sure this function gets inlined by VHLS, or cosim may fail!
// -----------------------------------------------------------------------
void bin_conv(Word wt_mem[CONVOLVERS][C_WT_WORDS], NormComp nc,
              Word dmem[2][CONVOLVERS][C_DMEM_WORDS], ap_uint<1> d_i_idx, ap_uint<1> d_o_idx,
              const unsigned n_inputs, const Address o_index, const ap_uint<1> new_batch,
              const ap_uint<2> width_mode, // 0=8'b, 1=16'b, 2=32'b
              const ap_uint<2> norm_mode   // 0='do nothing', 1='do norm', 2='do pool'
)
{
    const ap_uint<3> log_width = width_mode + LOG_BANK_WIDTH;
    const ap_uint<5> words_per_image = 1 << (2 * width_mode);
    const unsigned n_phases = n_inputs / CONVOLVERS;
    const unsigned images_per_phase = PIX_PER_PHASE >> (2 * log_width);
    const unsigned WORDS_PER_PHASE = PIX_PER_PHASE / WORD_SIZE;

    // ---------------------------------------------------------------------
    // buffers
    // ---------------------------------------------------------------------
    TwoBit line_buffer[CONVOLVERS][CONV_BANKS][CONV_ROWS][CONV_COLS];
    Bit conv_params[CONVOLVERS][K][K];
    ConvSum fixed_buffer[WORDS_PER_PHASE][WORD_SIZE];
    ConvSum fixed_temp[WORD_SIZE];
    // per-convolver buffers
    TwoBit word_buffer[CONVOLVERS][CONV_BANKS][CONV_COLS];
    TwoBit old_word_buffer[CONVOLVERS][CONV_BANKS][CONV_COLS];
    ConvOut conv_out_buffer[CONVOLVERS][WORD_SIZE];
    // edge padding flag bits
    bool lb[CONV_BANKS];
    bool rb[CONV_BANKS];

    static Address wt_addr = 0;      // address of weight word
    static ap_uint<3> wt_offset = 0; // offset 0..6 of param
    if (new_batch != 0)
    {
        wt_addr = 0;
        wt_offset = 0;
    }

    // ---------------------------------------------------------------------
    // Calculate edge padding flag bits
    const ap_uint<4> log_slice = log_width - LOG_BANK_WIDTH;
    const ap_uint<4> w_div_8 = (1 << log_width) >> 3;

    ap_uint<4> mask = ~ap_uint<4>(0); // set mask to all 1s
    mask = mask >> (4 - log_slice);
    for (ap_uint<4> bank = 0; bank < CONV_BANKS; ++bank)
    {

        const ap_uint<4> x = bank & mask;
        lb[bank] = (x == 0);           // (bank % w_div_8) == 0
        rb[bank] = (x + 1 == w_div_8); // (bank % w_div_8) == w_div_8-1
    }

    // ---------------------------------------------------------------------
    // Reset conv buffer
    for (IdxType i = 0; i < WORDS_PER_PHASE; ++i)
    {
        for (IdxType j = 0; j < WORD_SIZE; ++j)
        {

            fixed_buffer[i][j] = 0;
        }
    }

    // ---------------------------------------------------------------------
    // Compute in phases
    // Each phase processes CONVOLVERS * WORDS_PER_PHASE input words
    // ---------------------------------------------------------------------
    for (ap_uint<10> p = 0; p < n_phases; p += images_per_phase)
    {

        // wrd = which word in the current image
        // wrd_phase = which wrd in the current phase
        ap_uint<8> wrd = 0;
        ap_uint<8> wrd_phase = 0;

        // Load a word each iteration, and then process it
        // We load WORDS_PER_PHASE words per phase, however we also need 1 extra
        // "empty" iteration per image in the phase to do the loop epilogue, so the
        // loop bound is WORDS_PER_PHASE + images_per_phase
        for (ap_uint<8> count = 0; count < WORDS_PER_PHASE + images_per_phase; ++count)
        {
            // First word of an image
            if (wrd == 0)
            {
                Word wt_word_buffer[CONVOLVERS];

                // -------------------------------------------------------------------
                // Load param word
                // Each word contains CONV_W_PER_WORD weight filters, after we use
                // them all we should load the next word
                // -------------------------------------------------------------------
                for (IdxType m = 0; m < CONVOLVERS; ++m)
                {
                    /*if (wt_offset == 0)
                      wt_word_buffer[m] = wt_mem[m][wt_addr];
                    else
                      wt_word_buffer[m] = wt_word_buffer[m] >> WT_SIZE;
                    */
                    wt_word_buffer[m] = wt_mem[m][wt_addr] >> ap_uint<6>(WT_SIZE * wt_offset);
                }
                if (wt_offset == CONV_W_PER_WORD - 1)
                {
                    ++wt_addr;
                    wt_offset = 0;
                }
                else
                {
                    ++wt_offset;
                }
                // print_wt_word(wt_word_buffer[0]);

                // -------------------------------------------------------------------
                // Load params
                // Each word contains CONV_W_PER_WORD weight filters packed into the first
                // 63 bits, the last bit is unused. Wts are stored in output-major order.
                // -------------------------------------------------------------------
                for (IdxType m = 0; m < CONVOLVERS; ++m)
                {
                    for (ap_uint<2> kr = 0; kr < K; ++kr)
                    {
                        for (ap_uint<2> kc = 0; kc < K; ++kc)
                            conv_params[m][kr][kc] =
                                partSelect(wt_word_buffer[m], kr * K + kc, kr * K + kc);
                    }
                }
            }

            // -------------------------------------------------------------------
            // Every word in an image
            // -------------------------------------------------------------------
            // Load word
            // (wrd_phase-wrd) is which wrd in the current phase, aligned to img
            // boundary
            if (wrd != words_per_image)
            {
                for (IdxType m = 0; m < CONVOLVERS; ++m)
                {
                    Word word = dmem[d_i_idx][m][p * words_per_image + wrd_phase];
                    for (IdxType bank = 0; bank < CONV_BANKS; ++bank)
                    {
                        for (IdxType cc = 0; cc < CONV_COLS - 2; ++cc)
                        {
                            word_buffer[m][bank][cc + 1] =
                                encode_bit(partSelect(word, ap_uint<6>(bank * BANK_WIDTH + cc),
                                                      ap_uint<6>(bank * BANK_WIDTH + cc)));
                        }
                        word_buffer[m][bank][0] =
                            (bank == 0)
                                ? TwoBit(0)
                                : encode_bit(partSelect(word, ap_uint<6>(bank * BANK_WIDTH - 1),
                                                        ap_uint<6>(bank * BANK_WIDTH - 1)));
                        word_buffer[m][bank][CONV_COLS - 1] =
                            (bank == CONV_BANKS - 1)
                                ? TwoBit(0)
                                : encode_bit(
                                      partSelect(word, ap_uint<6>(bank * BANK_WIDTH + BANK_WIDTH),
                                                 ap_uint<6>(bank * BANK_WIDTH + BANK_WIDTH)));
                    }
                }
            }

            // Compute
            for (IdxType m = 0; m < CONVOLVERS; ++m)
            {
                // Do the following for each word in an image
                process_word(word_buffer[m], old_word_buffer[m], lb, rb, line_buffer[m],
                             conv_params[m], conv_out_buffer[m], log_width, words_per_image, wrd);
            } // CONVOLVERS

            for (IdxType m = 0; m < CONVOLVERS; ++m)
            {
                for (IdxType bank = 0; bank < CONV_BANKS; ++bank)
                {
                    for (IdxType cc = 0; cc < CONV_COLS; ++cc)
                    {
                        old_word_buffer[m][bank][cc] = word_buffer[m][bank][cc];
                    }
                }
            }

            // -------------------------------------------------------------------
            // Sum results across convolvers
            // -------------------------------------------------------------------
            for (IdxType i = 0; i < WORD_SIZE; ++i)
            {
                // Ignore conv results after processing the first word
                if (wrd > 0)
                {
                    ConvSum s = 0;
                    for (IdxType m = 0; m < CONVOLVERS; ++m)
                        s += conv_out_buffer[m][i];
                    fixed_buffer[wrd_phase - 1][i] += s;
                }
            }

            // -------------------------------------------------------------------
            // Increment counters
            // -------------------------------------------------------------------
            if (wrd != words_per_image)
            {
                wrd++;
                wrd_phase++;
            }
            else
            {
                wrd = 0;
            }
        } // wrd_phase = 0 .. WORDS_PER_PHASE

    } // n_phases

    for (ap_uint<5> w = 0; w < words_per_image; ++w)
    {
        for (IdxType b = 0; b < WORD_SIZE; ++b)
        {

            fixed_temp[b] = fixed_buffer[w][b];
        }

        for (ap_uint<8> i = words_per_image; i < WORDS_PER_PHASE; i += words_per_image)
        {
            for (IdxType b = 0; b < WORD_SIZE; ++b)
            {
                fixed_temp[b] += fixed_buffer[w + i][b];
            }
        }

        for (IdxType b = 0; b < WORD_SIZE; ++b)
        {

            fixed_buffer[w][b] = fixed_temp[b];
        }
    }

    const Address bank_idx = o_index % CONVOLVERS;
    const Address bank_off = o_index / CONVOLVERS;
    const ap_uint<5> pool_width = 1 << (log_width - 1);

    static Word outword;
    Word poolword = 0;

    for (ap_uint<6> w = 0; w < words_per_image; ++w)
    {
        Word binword = 0;
        Address o_bank_idx = bank_idx;
        Address o_bank_offset = bank_off * words_per_image + w;
        const ap_uint<6> out_offset = (w % 4) << 4;

        for (ap_uint<7> i = 0; i < WORD_SIZE; ++i)
        {
            binword = (((fixed_buffer[w][i] >= nc) ? 0 : 1) << i) | binword;
        }

        if (norm_mode == 1)
        {
            outword = binword;
        }
        else if (norm_mode == 2)
        {
            // horizontal pooling first
            ap_int<WORD_SIZE / 2> poolword_h = 0;
            for (ap_uint<6> i = 0; i < WORD_SIZE / 2; ++i)
            {
                poolword_h =
                    ((partSelect(binword, 2 * i, 2 * i) & partSelect(binword, 2 * i + 1, 2 * i + 1))
                     << i) |
                    poolword_h;
            }

            // vertical pooling
            for (ap_uint<6> i = 0; i < WORD_SIZE / 4; ++i)
            {
                // source indices
                ap_uint<5> i0 = i >> (log_width - 1);
                i0 = (i0 << log_width) + partSelect(i, log_width - 2, 0);
                ap_uint<5> i1 = i0 + pool_width;
                // dest index
                ap_uint<6> d0 = out_offset + i;
                poolword =
                    ((partSelect(poolword_h, i0, i0) & partSelect(poolword_h, i1, i1)) << i) |
                    poolword;
            }

            // For log_width > 3 we can just assign the word, but log_width = 3 means
            // width = 8, which means pooled width = 4, which is only 16 bits, which
            // is less than 1 Word. So each time we get here we only have 16 bits,
            // meaning we have to accumulate four of these 16-bit batches before
            // writing a word out.
            if (log_width != LOG_BANK_WIDTH)
            {
                o_bank_offset /= 4;
                outword = poolword;
            }
            else
            {
                outword = (partSelect(poolword, 15, 0) << 48) | (outword >> WORD_SIZE / 4);
                o_bank_idx = (o_index / 4) % CONVOLVERS;
                o_bank_offset = (o_index / 4) / CONVOLVERS;
            }
        }

        dmem[d_o_idx][o_bank_idx][o_bank_offset] = outword;
    }
}