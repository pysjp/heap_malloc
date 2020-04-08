

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/** Null Definition */
#ifdef NULL
#define NULL_POINTER NULL
#else
#define NULL_POINTER 0x0
#endif

/** Type Definitions */

/** Signed 8-bit */
typedef char s8;

/** Signed 16-bit */
typedef short s16;

/** Signed 32-bit */
typedef int s32;

/** Unsigned 8-bit */
typedef unsigned char u8;

/** Unsigned 16-bit */
typedef unsigned short u16;

/** Unsigned 32-bit */
typedef unsigned int u32;

/** Signed 64-bit */
typedef long long int s64;

/** Unsigned 64-bit */
typedef unsigned long long int u64;

#endif
