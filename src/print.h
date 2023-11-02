#pragma once

#include <stdio.h>

#ifdef DEBUG_PRINT
#define DPrint(...) \
                printf(__VA_ARGS__);

#define FPrint() \
                printf("%s()\n", __FUNCTION__);



#define DPrint_A(__value__, __prefix__) \
    printf("%s%s: %s\n", __prefix__, #__value__, __value__);

#define DPrint_D(__value__, __prefix__) \
    printf("%s%s: %llu\n", __prefix__, #__value__, (UINT64)__value__);

#define DPrint_H(__value__, __prefix__) \
    printf("%s%s: 0x%llx\n", __prefix__, #__value__, (UINT64)__value__);

#define DPrint_HD(__value__, __prefix__) \
    printf("%s%s: 0x%llx (%llu)\n", __prefix__, #__value__, (UINT64)__value__, (UINT64)__value__);

#define DPrint_P(__value__, __prefix__) \
    printf("%s%s: %p\n", __prefix__, #__value__, (PVOID)__value__);



#define DPrintMemCol8(_b_, _s_, _o_) \
{ \
    for ( SIZE_T _i_ = 0; _i_ < _s_; _i_+=0x10 ) \
    { \
        SIZE_T _end_ = (_i_+0x10<_s_)?(_i_+0x10):(_s_); \
        SIZE_T _gap_ = (_i_+0x10<=_s_) ? 0 : ((0x10+_i_-_s_)*3); \
        printf("%p  ", (((PUINT8)_o_)+_i_)); \
         \
        for ( SIZE_T _j_ = _i_, _k_=0; _j_ < _end_; _j_++, _k_++ ) \
        { \
            printf("%02x", ((PUINT8)_b_)[_j_]); \
            printf("%c", (_k_==7?'-':' ')); \
        } \
        for ( SIZE_T _j_ = 0; _j_ < _gap_; _j_++ ) \
        { \
            printf(" "); \
        } \
        printf("  "); \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_++ ) \
        { \
            if ( ((PUINT8)_b_)[_j_] < 0x20 || ((PUINT8)_b_)[_j_] > 0x7E || ((PUINT8)_b_)[_j_] == 0x25 ) \
            { \
                printf("."); \
            }  \
            else \
            { \
                printf("%c", ((PUINT8)_b_)[_j_]); \
            } \
        } \
        printf("\n"); \
    } \
}

#define DPrintMemCol16(_b_, _s_) \
{ \
    for ( SIZE_T _i_ = 0; _i_ < _s_; _i_+=0x10 ) \
    { \
        SIZE_T _end_ = (_i_+0x10<_s_)?(_i_+0x10):(_s_); \
        SIZE_T _gap_ = (_i_+0x10<=_s_) ? 0 : ((0x10+_i_-_s_)/2*5); \
        printf("%p  ", (((PUINT8)_b_)+_i_)); \
         \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_+=2 ) \
        { \
            printf("%04x ", *(PUINT16)&(((PUINT8)_b_)[_j_])); \
        } \
        for ( SIZE_T _j_ = 0; _j_ < _gap_; _j_++ ) \
        { \
            printf(" "); \
        } \
        printf("  "); \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_+=2 ) \
        { \
            printf("%wc", *(PUINT16)&(((PUINT8)_b_)[_j_])); \
        } \
        printf("\n"); \
    } \
}

#define DPrintMemCol32(_b_, _s_) \
{ \
    for ( SIZE_T _i_ = 0; _i_ < _s_; _i_+=0x10 ) \
    { \
        SIZE_T _end_ = (_i_+0x10<_s_)?(_i_+0x10):(_s_); \
        printf("%p  ", (((PUINT8)_b_)+_i_)); \
         \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_+=4 ) \
        { \
            printf("%08x ", *(PUINT32)&(((PUINT8)_b_)[_j_])); \
        } \
        printf("\n"); \
    } \
}

#define DPrintMemCol64(_b_, _s_) \
{ \
    for ( SIZE_T _i_ = 0; _i_ < _s_; _i_+=0x10 ) \
    { \
        SIZE_T _end_ = (_i_+0x10<_s_)?(_i_+0x10):(_s_); \
        printf("%p  ", (((PUINT8)_b_)+_i_)); \
         \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_+=8 ) \
        { \
            printf("%016llx ", *(PUINT64)&(((PUINT8)_b_)[_j_])); \
        } \
        printf("\n"); \
    } \
}

#define DPrintBytes(_b_, _s_) \
{ \
    for ( SIZE_T _i_ = 0; _i_ < _s_; _i_+=0x10 ) \
    { \
        SIZE_T _end_ = (_i_+0x10<_s_)?(_i_+0x10):(_s_); \
         \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_++ ) \
        { \
            printf("%02x ", ((PUINT8)_b_)[_j_]); \
        } \
        printf("\n"); \
    } \
}
#else
#define DPrint(...)
#define FPrint()
#define DPrint_A(__value__, __prefix__)
#define DPrint_D(__value__, __prefix__)
#define DPrint_H(__value__, __prefix__)
#define DPrint_HD(__value__, __prefix__)
#define DPrint_P(__value__, __prefix__)
#define DPrintMemCol8(_b_, _s_, _o_)
#define DPrintMemCol16(_b_, _s_)
#define DPrintMemCol32(_b_, _s_)
#define DPrintMemCol64(_b_, _s_)
#define DPrintBytes(_b_, _s_)
#endif

#ifdef ERROR_PRINT
#define EPrint(...) \
{ \
                printf("[e] ");\
                printf(__VA_ARGS__); \
}
#else
#define EPrint(...)
#endif

#define HEX_CHAR_WIDTH(__hcw_v__, __hcw_w__) \
{ \
    UINT8 _hcw_w_ = 0x10; \
    for ( UINT8 _i_ = 0x38; _i_ > 0; _i_-=8 ) \
    { \
        if ( ! ((UINT8)(__hcw_v__ >> _i_)) ) \
            _hcw_w_ -= 2; \
        else \
            break; \
    } \
    __hcw_w__ = _hcw_w_; \
}


#define PrintBytes(_b_, _s_) \
{ \
    for ( SIZE_T _i_ = 0; _i_ < _s_; _i_+=0x10 ) \
    { \
        SIZE_T _end_ = (_i_+0x10<_s_)?(_i_+0x10):(_s_); \
         \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_++ ) \
        { \
            printf("%02x ", ((PUINT8)_b_)[_j_]); \
        } \
        printf("\n"); \
    } \
}

#define PrintMemCol8(_b_, _s_, _o_) \
{ \
    UINT64 _hw_v_ = _o_ + _s_; \
    UINT8 _hw_w_ = 0x10; \
    HEX_CHAR_WIDTH(_hw_v_, _hw_w_); \
    \
    for ( SIZE_T _i_ = 0; _i_ < _s_; _i_+=0x10 ) \
    { \
        SIZE_T _end_ = (_i_+0x10<_s_)?(_i_+0x10):(_s_); \
        SIZE_T _gap_ = (_i_+0x10<=_s_) ? 0 : ((0x10+_i_-_s_)*3); \
        printf("%.*zx  ", _hw_w_, (((SIZE_T)_o_)+_i_)); \
         \
        for ( SIZE_T _j_ = _i_, _k_=0; _j_ < _end_; _j_++, _k_++ ) \
        { \
            printf("%02x", ((PUINT8)_b_)[_j_]); \
            printf("%c", (_k_==7?'-':' ')); \
        } \
        for ( SIZE_T _j_ = 0; _j_ < _gap_; _j_++ ) \
        { \
            printf(" "); \
        } \
        printf("  "); \
        for ( SIZE_T _j_ = _i_; _j_ < _end_; _j_++ ) \
        { \
            if ( ((PUINT8)_b_)[_j_] < 0x20 || ((PUINT8)_b_)[_j_] > 0x7E || ((PUINT8)_b_)[_j_] == 0x25 ) \
            { \
                printf("."); \
            }  \
            else \
            { \
                printf("%c", ((PUINT8)_b_)[_j_]); \
            } \
        } \
        printf("\n"); \
    } \
}
