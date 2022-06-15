#ifndef __CONVERT_FROM_ARGB_H__
#define __CONVERT_FROM_ARGB_H__
//#define SIMD_ALIGNED(var) __declspec(align(16)) var
#define SIMD_ALIGNED(var) var

#define SS(width, shift) (((width) + (1 << (shift)) - 1) >> (shift))
#define IS_ALIGNED(p, a) (!((int)(p) & ((a)-1)))
#define align_buffer_64(var, size)                                           \
  unsigned char* var##_mem = (unsigned char*)(malloc((size) + 63));         /* NOLINT */ \
  unsigned char* var = (unsigned char*)(((int)(var##_mem) + 63) & ~63) /* NOLINT */

#define free_aligned_buffer_64(var) \
  free(var##_mem);                  \
  var = 0



// Any 1 to 1.
#define ANY11(NAMEANY, ANY_SIMD, UVSHIFT, SBPP, BPP, MASK)                \
                        void NAMEANY(const unsigned char* src_ptr, unsigned char* dst_ptr, int width) {     \
                          SIMD_ALIGNED(unsigned char temp[128 * 2]);                                  \
                          memset(temp, 0, 128); /* for YUY2 and msan */                         \
                          int r = width & MASK;                                                 \
                          int n = width & ~MASK;                                                \
                          if (n > 0) {                                                          \
                            ANY_SIMD(src_ptr, dst_ptr, n);                                      \
                          }                                                                     \
                          memcpy(temp, src_ptr + (n >> UVSHIFT) * SBPP, SS(r, UVSHIFT) * SBPP); \
                          ANY_SIMD(temp, temp + 128, MASK + 1);                                 \
                          memcpy(dst_ptr + n * BPP, temp + 128, r * BPP);                       \
                        }

// Any 1 to 2 with source stride (2 rows of source).  Outputs UV planes.
// 128 byte row allows for 32 avx ARGB pixels.
#define ANY12S(NAMEANY, ANY_SIMD, UVSHIFT, BPP, MASK)                        \
                          void NAMEANY(const unsigned char* src_ptr, int src_stride_ptr, unsigned char* dst_u,   \
                                       unsigned char* dst_v, int width) {                                  \
                            SIMD_ALIGNED(unsigned char temp[128 * 4]);                                     \
                            memset(temp, 0, 128 * 2); /* for msan */                                 \
                            int r = width & MASK;                                                    \
                            int n = width & ~MASK;                                                   \
                            if (n > 0) {                                                             \
                              ANY_SIMD(src_ptr, src_stride_ptr, dst_u, dst_v, n);                    \
                            }                                                                        \
                            memcpy(temp, src_ptr + (n >> UVSHIFT) * BPP, SS(r, UVSHIFT) * BPP);      \
                            memcpy(temp + 128, src_ptr + src_stride_ptr + (n >> UVSHIFT) * BPP,      \
                                   SS(r, UVSHIFT) * BPP);                                            \
                            if ((width & 1) && UVSHIFT == 0) { /* repeat last pixel for subsample */ \
                              memcpy(temp + SS(r, UVSHIFT) * BPP, temp + SS(r, UVSHIFT) * BPP - BPP, \
                                     BPP);                                                           \
                              memcpy(temp + 128 + SS(r, UVSHIFT) * BPP,                              \
                                     temp + 128 + SS(r, UVSHIFT) * BPP - BPP, BPP);                  \
                            }                                                                        \
                            ANY_SIMD(temp, 128, temp + 256, temp + 384, MASK + 1);                   \
                            memcpy(dst_u + (n >> 1), temp + 256, SS(r, 1));                          \
                            memcpy(dst_v + (n >> 1), temp + 384, SS(r, 1));                          \
                          }

// Any 2 planes to 1.
#define ANY21(NAMEANY, ANY_SIMD, UVSHIFT, SBPP, SBPP2, BPP, MASK)             \
                                         void NAMEANY(const unsigned char* y_buf, const unsigned char* uv_buf, unsigned char* dst_ptr, \
                                                      int width) {                                                   \
                                           SIMD_ALIGNED(unsigned char temp[64 * 3]);                                       \
                                           memset(temp, 0, 64 * 2); /* for msan */                                   \
                                           int r = width & MASK;                                                     \
                                           int n = width & ~MASK;                                                    \
                                           if (n > 0) {                                                              \
                                             ANY_SIMD(y_buf, uv_buf, dst_ptr, n);                                    \
                                           }                                                                         \
                                           memcpy(temp, y_buf + n * SBPP, r * SBPP);                                 \
                                           memcpy(temp + 64, uv_buf + (n >> UVSHIFT) * SBPP2,                        \
                                                  SS(r, UVSHIFT) * SBPP2);                                           \
                                           ANY_SIMD(temp, temp + 64, temp + 128, MASK + 1);                          \
                                           memcpy(dst_ptr + n * BPP, temp + 128, r * BPP);                           \
                                         }
// clang-format off
// 16x2 pixels -> 8x1.  width is number of argb pixels. e.g. 16.
#define RGBTOUV(QB, QG, QR)                                                 \
                                         "vmul.s16   q8, " #QB ", q10               \n" /* B                    */ \
                                         "vmls.s16   q8, " #QG ", q11               \n" /* G                    */ \
                                         "vmls.s16   q8, " #QR ", q12               \n" /* R                    */ \
                                         "vadd.u16   q8, q8, q15                    \n" /* +128 -> unsigned     */ \
                                         "vmul.s16   q9, " #QR ", q10               \n" /* R                    */ \
                                         "vmls.s16   q9, " #QG ", q14               \n" /* G                    */ \
                                         "vmls.s16   q9, " #QB ", q13               \n" /* B                    */ \
                                         "vadd.u16   q9, q9, q15                    \n" /* +128 -> unsigned     */ \
                                         "vqshrn.u16  d0, q8, #8                    \n" /* 16 bit to 8 bit U    */ \
                                         "vqshrn.u16  d1, q9, #8                    \n" /* 16 bit to 8 bit V    */
// clang-format on


void ARGBToYRow_NEON(const unsigned char *src_argb, unsigned char *dst_y, int width)
{
    asm volatile(
        "vmov.u8    d24, #25                       \n"  // B * 0.1016 coefficient
        "vmov.u8    d25, #129                      \n"  // G * 0.5078 coefficient
        "vmov.u8    d26, #66                       \n"  // R * 0.2578 coefficient
        "vmov.u8    d27, #16                       \n"  // Add 16 constant
        "1:                                        \n"
        "vld4.8     {d0, d1, d2, d3}, [%0]!        \n"  // load 8 ARGB pixels.
        "subs       %2, %2, #8                     \n"  // 8 processed per loop.
        "vmull.u8   q2, d0, d24                    \n"  // B
        "vmlal.u8   q2, d1, d25                    \n"  // G
        "vmlal.u8   q2, d2, d26                    \n"  // R
        "vqrshrn.u16 d0, q2, #8                    \n"  // 16 bit to 8 bit Y
        "vqadd.u8   d0, d27                        \n"
        "vst1.8     {d0}, [%1]!                    \n"  // store 8 pixels Y.
        "bgt        1b                             \n"
        : "+r"(src_argb),  // %0
        "+r"(dst_y),     // %1
        "+r"(width)      // %2
        :
        : "cc", "memory", "q0", "q1", "q2", "q12", "q13");
}
void ARGBToYRow_Any_NEON(const unsigned char *src_ptr, unsigned char *dst_ptr, int width);
ANY11(ARGBToYRow_Any_NEON, ARGBToYRow_NEON, 0, 4, 1, 7);

// TODO(fbarchard): Consider vhadd vertical, then vpaddl horizontal, avoid shr.
void ARGBToUVRow_NEON(const unsigned char *src_argb,
                      int src_stride_argb,
                      unsigned char *dst_u,
                      unsigned char *dst_v,
                      int width)
{
    asm volatile(
        "add        %1, %0, %1                     \n"  // src_stride + src_argb
        "vmov.s16   q10, #112 / 2                  \n"  // UB / VR 0.875 coefficient
        "vmov.s16   q11, #74 / 2                   \n"  // UG -0.5781 coefficient
        "vmov.s16   q12, #38 / 2                   \n"  // UR -0.2969 coefficient
        "vmov.s16   q13, #18 / 2                   \n"  // VB -0.1406 coefficient
        "vmov.s16   q14, #94 / 2                   \n"  // VG -0.7344 coefficient
        "vmov.u16   q15, #0x8080                   \n"  // 128.5
        "1:                                        \n"
        "vld4.8     {d0, d2, d4, d6}, [%0]!        \n"  // load 8 ARGB pixels.
        "vld4.8     {d1, d3, d5, d7}, [%0]!        \n"  // load next 8 ARGB pixels.
        "vpaddl.u8  q0, q0                         \n"  // B 16 bytes -> 8 shorts.
        "vpaddl.u8  q1, q1                         \n"  // G 16 bytes -> 8 shorts.
        "vpaddl.u8  q2, q2                         \n"  // R 16 bytes -> 8 shorts.
        "vld4.8     {d8, d10, d12, d14}, [%1]!     \n"  // load 8 more ARGB pixels.
        "vld4.8     {d9, d11, d13, d15}, [%1]!     \n"  // load last 8 ARGB pixels.
        "vpadal.u8  q0, q4                         \n"  // B 16 bytes -> 8 shorts.
        "vpadal.u8  q1, q5                         \n"  // G 16 bytes -> 8 shorts.
        "vpadal.u8  q2, q6                         \n"  // R 16 bytes -> 8 shorts.

        "vrshr.u16  q0, q0, #1                     \n"  // 2x average
        "vrshr.u16  q1, q1, #1                     \n"
        "vrshr.u16  q2, q2, #1                     \n"

        "subs       %4, %4, #16                    \n"  // 32 processed per loop.
        RGBTOUV(q0, q1, q2)
        "vst1.8     {d0}, [%2]!                    \n"  // store 8 pixels U.
        "vst1.8     {d1}, [%3]!                    \n"  // store 8 pixels V.
        "bgt        1b                             \n"
        : "+r"(src_argb),  // %0
        "+r"(src_stride_argb),  // %1
        "+r"(dst_u),     // %2
        "+r"(dst_v),     // %3
        "+r"(width)        // %4
        :
        : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7",
        "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
    );
}

void ARGBToUVRow_Any_NEON(const unsigned char *src_ptr,
                          int src_stride_ptr,
                          unsigned char *dst_u,
                          unsigned char *dst_v,
                          int width);

ANY12S(ARGBToUVRow_Any_NEON, ARGBToUVRow_NEON, 0, 4, 15);

// Reads 16 U's and V's and writes out 16 pairs of UV.
void MergeUVRow_NEON(const unsigned char *src_u,
                     const unsigned char *src_v,
                     unsigned char *dst_uv,
                     int width)
{
    asm volatile(
        "1:                                        \n"
        "vld1.8     {q0}, [%0]!                    \n"  // load U
        "vld1.8     {q1}, [%1]!                    \n"  // load V
        "subs       %3, %3, #16                    \n"  // 16 processed per loop
        "vst2.8     {q0, q1}, [%2]!                \n"  // store 16 pairs of UV
        "bgt        1b                             \n"
        : "+r"(src_u),                // %0
        "+r"(src_v),                // %1
        "+r"(dst_uv),               // %2
        "+r"(width)                 // %3  // Output registers
        :                             // Input registers
        : "cc", "memory", "q0", "q1"  // Clobber List
    );
}
void MergeUVRow_Any_NEON(const unsigned char *y_buf,
                         const unsigned char *uv_buf,
                         unsigned char *dst_ptr,
                         int width);

ANY21(MergeUVRow_Any_NEON, MergeUVRow_NEON, 0, 1, 1, 2, 15);


int ARGBToNV12(const unsigned char *src_argb,
               int src_stride_argb,
               unsigned char *dst_y,
               int dst_stride_y,
               unsigned char *dst_uv,
               int dst_stride_uv,
               int width,
               int height)
{
    int y;
    int halfwidth = (width + 1) >> 1;

    if(!src_argb || !dst_y || !dst_uv || width <= 0 || height == 0) {
        return -1;
    }

    void (*ARGBToUVRow)(const unsigned char *src_argb0, int src_stride_argb,
                        unsigned char *dst_u, unsigned char *dst_v, int width) =
                            NULL;
    void (*ARGBToYRow)(const unsigned char *src_argb, unsigned char *dst_y, int width) =
        NULL;
    void (*MergeUVRow_)(const unsigned char *src_u, const unsigned char *src_v,
                        unsigned char *dst_uv, int width) = NULL;

    // Negative height means invert the image.
    if(height < 0) {
        height = -height;
        src_argb = src_argb + (height - 1) * src_stride_argb;
        src_stride_argb = -src_stride_argb;
    }

    ARGBToYRow = ARGBToYRow_Any_NEON;

    if(IS_ALIGNED(width, 8)) {
        ARGBToYRow = ARGBToYRow_NEON;
    }

    ARGBToUVRow = ARGBToUVRow_Any_NEON;

    if(IS_ALIGNED(width, 16)) {
        ARGBToUVRow = ARGBToUVRow_NEON;
    }

    MergeUVRow_ = MergeUVRow_Any_NEON;

    if(IS_ALIGNED(halfwidth, 16)) {
        MergeUVRow_ = MergeUVRow_NEON;
    }


    {
        // Allocate a rows of uv.
        align_buffer_64(row_u, ((halfwidth + 31) & ~31) * 2);
        unsigned char *row_v = row_u + ((halfwidth + 31) & ~31);

        for(y = 0; y < height - 1; y += 2) {
            ARGBToUVRow(src_argb, src_stride_argb, row_u, row_v, width);
            MergeUVRow_(row_u, row_v, dst_uv, halfwidth);
            ARGBToYRow(src_argb, dst_y, width);
            ARGBToYRow(src_argb + src_stride_argb, dst_y + dst_stride_y, width);
            src_argb += src_stride_argb * 2;
            dst_y += dst_stride_y * 2;
            dst_uv += dst_stride_uv;
        }

        if(height & 1) {
            ARGBToUVRow(src_argb, 0, row_u, row_v, width);
            MergeUVRow_(row_u, row_v, dst_uv, halfwidth);
            ARGBToYRow(src_argb, dst_y, width);
        }

        free_aligned_buffer_64(row_u);
    }
}
// Shuffle table for converting BGRA to ARGB.
static const unsigned char kShuffleMaskBGRAToARGB[] = {
    3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12
};
// Use first 4 shuffler values to reorder ARGB channels.
void ARGBShuffleRow_C(const unsigned char *src_argb,
                      unsigned char *dst_argb,
                      const unsigned char *shuffler,
                      int width)
{
    int index0 = shuffler[0];
    int index1 = shuffler[1];
    int index2 = shuffler[2];
    int index3 = shuffler[3];
    // Shuffle a row of ARGB.
    int x;

    for(x = 0; x < width; ++x) {
        // To support in-place conversion.
        unsigned char b = src_argb[index0];
        unsigned char g = src_argb[index1];
        unsigned char r = src_argb[index2];
        unsigned char a = src_argb[index3];
        dst_argb[0] = b;
        dst_argb[1] = g;
        dst_argb[2] = r;
        dst_argb[3] = a;
        src_argb += 4;
        dst_argb += 4;
    }
}
// Shuffle ARGB channel order.  e.g. BGRA to ARGB.
int ARGBShuffle(const unsigned char *src_bgra,
                int src_stride_bgra,
                unsigned char *dst_argb,
                int dst_stride_argb,
                const unsigned char *shuffler,
                int width,
                int height)
{
    int y;
    void (*ARGBShuffleRow)(const unsigned char *src_bgra, unsigned char *dst_argb,
                           const unsigned char *shuffler, int width) = ARGBShuffleRow_C;

    if(!src_bgra || !dst_argb || width <= 0 || height == 0) {
        return -1;
    }

    // Negative height means invert the image.
    if(height < 0) {
        height = -height;
        src_bgra = src_bgra + (height - 1) * src_stride_bgra;
        src_stride_bgra = -src_stride_bgra;
    }

    // Coalesce rows.
    if(src_stride_bgra == width * 4 && dst_stride_argb == width * 4) {
        width *= height;
        height = 1;
        src_stride_bgra = dst_stride_argb = 0;
    }

    for(y = 0; y < height; ++y) {
        ARGBShuffleRow(src_bgra, dst_argb, shuffler, width);
        src_bgra += src_stride_bgra;
        dst_argb += dst_stride_argb;
    }

    return 0;
}



#endif
