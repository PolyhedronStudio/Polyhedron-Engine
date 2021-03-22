// LICENSE HERE.

//
// shared/math/endian.h
//
// Endian swapping related code.
//
#ifndef __INC_SHARED_ENDIAN_H__
#define __INC_SHARED_ENDIAN_H__

static inline uint16_t ShortSwap(uint16_t s)
{
    s = (s >> 8) | (s << 8);
    return s;
}

static inline uint32_t LongSwap(uint32_t l)
{
    l = ((l >> 8) & 0x00ff00ff) | ((l << 8) & 0xff00ff00);
    l = (l >> 16) | (l << 16);
    return l;
}

static inline float FloatSwap(float f)
{
    union {
        float f;
        uint32_t l;
    } dat1, dat2;

    dat1.f = f;
    dat2.l = LongSwap(dat1.l);
    return dat2.f;
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define BigShort    ShortSwap
#define BigLong     LongSwap
#define BigFloat    FloatSwap
#define LittleShort(x)    ((uint16_t)(x))
#define LittleLong(x)     ((uint32_t)(x))
#define LittleFloat(x)    ((float)(x))
#define MakeRawLong(b1,b2,b3,b4) (((b4)<<24)|((b3)<<16)|((b2)<<8)|(b1))
#define MakeRawShort(b1,b2) (((b2)<<8)|(b1))
#elif __BYTE_ORDER == __BIG_ENDIAN
#define BigShort(x)     ((uint16_t)(x))
#define BigLong(x)      ((uint32_t)(x))
#define BigFloat(x)     ((float)(x))
#define LittleShort ShortSwap
#define LittleLong  LongSwap
#define LittleFloat FloatSwap
#define MakeRawLong(b1,b2,b3,b4) (((b1)<<24)|((b2)<<16)|((b3)<<8)|(b4))
#define MakeRawShort(b1,b2) (((b1)<<8)|(b2))
#else
#error Unknown byte order
#endif

#define LittleLongMem(p) (((p)[3]<<24)|((p)[2]<<16)|((p)[1]<<8)|(p)[0])
#define LittleShortMem(p) (((p)[1]<<8)|(p)[0])

#define RawLongMem(p) MakeRawLong((p)[0],(p)[1],(p)[2],(p)[3])
#define RawShortMem(p) MakeRawShort((p)[0],(p)[1])

#define LittleVector(a,b) \
    ((b)[0]=LittleFloat((a)[0]),\
     (b)[1]=LittleFloat((a)[1]),\
     (b)[2]=LittleFloat((a)[2]))

#endif // __INC_SHARED_ENDIAN_H__