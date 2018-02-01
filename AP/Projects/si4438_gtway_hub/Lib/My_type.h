/**********************************************************************
  My_type.h file
  类型定义
  作者：FISHER
  建立日期：2014-01-29
  修改日期：
**********************************************************************/

#ifndef __My_TYPE_H__
#define __My_TYPE_H__


#define CHAE    char
#define BYTE 	unsigned char
#define WORD 	unsigned int
#define uint8    unsigned char
#define uint8_t    unsigned char
#define bool    unsigned char

#define u8    unsigned char

#define word   unsigned short int
#define uint16   unsigned short int
#define u16   unsigned short int
#define uint16_t   unsigned short int

#define uint32   unsigned long int
#define uint32_t   unsigned long int

#define int8     signed char
#define int8_t     signed char

#define int16    signed short int
#define int32    signed long int
#define uint64   unsigned long long int
#define int64    signed long long int

#define uchar unsigned char
#define uint  unsigned int
#define ulong unsigned long

// used with UU16
# define LSB 1
# define MSB 0

// used with UU32 (b0 is least-significant byte)
# define b0 3
# define b1 2
# define b2 1
# define b3 0

# define true 1
# define false 0


typedef unsigned char U8;
typedef unsigned int U16;
typedef unsigned long U32;
typedef unsigned char U88;

typedef signed char S8;
typedef signed int S16;
typedef signed long S32;


typedef union UU16
{
   U16 U16;
   S16 S16;
   U8 U8[2];
   S8 S8[2];
} UU16;

typedef union UU32
{
   U32 U32;
   S32 S32;
   UU16 UU16[2];
   U16 U16[2];
   S16 S16[2];
   U8 U8[4];
   S8 S8[4];
} UU32;

typedef union UU88
{
   U88 U88[11];
} UU88;

#endif
