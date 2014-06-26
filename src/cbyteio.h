/**
 * byte i/o routines for C.
 */

#ifndef CBYTEIO_H
#define CBYTEIO_H

#include <stdio.h>
#include <stdlib.h>

#if !defined(bool) && !defined(__cplusplus)
  typedef int bool;
  #define true    1
  #define false   0
#endif /* !bool */

#ifndef byte
  typedef unsigned char byte;
#endif /* !byte */
#ifndef sbyte
  typedef signed char sbyte;
#endif /* !sbyte */

#ifndef int8
typedef signed   __int8  int8;
#endif
#ifndef int16
typedef signed   __int16 int16;
#endif
#ifndef int32
typedef signed   __int32 int32;
#endif
#ifndef int64
typedef signed   __int64 int64;
#endif
#ifndef uint8
typedef unsigned __int8  uint8;
#endif
#ifndef uint16
typedef unsigned __int16 uint16;
#endif
#ifndef uint32
typedef unsigned __int32 uint32;
#endif
#ifndef uint64
typedef unsigned __int64 uint64;
#endif

#ifndef countof
  #define countof(a)    (sizeof(a) / sizeof(a[0]))
#endif

#ifndef min
  #define min(a, b)     ((a) < (b) ? (a) : (b))
#endif /* !min */

#ifndef max
  #define max(a, b)     ((a) > (b) ? (a) : (b))
#endif /* !max */

#ifndef clip
  #define clip(l, x, h) (min(max((l), (x)), (h)))
#endif /* !clip */

#ifndef PATH_MAX
  #ifdef _MAX_PATH
    #define PATH_MAX  _MAX_PATH
  #else
    #define PATH_MAX  260
  #endif
#endif

#ifndef INLINE
#ifdef inline
#define INLINE  inline
#elif defined(__inline)
#define INLINE  __inline
#else
#define INLINE
#endif
#endif // !INLINE

/** unsigned byte to signed */
static INLINE int utos1(unsigned int value)
{
  return (value & 0x80) ? -(signed)(value ^ 0xff)-1 : value;
}

/** unsigned 2 bytes to signed */
static INLINE int utos2(unsigned int value)
{
  return (value & 0x8000) ? -(signed)(value ^ 0xffff)-1 : value;
}

/** unsigned 3 bytes to signed */
static INLINE int utos3(unsigned int value)
{
  return (value & 0x800000) ? -(signed)(value ^ 0xffffff)-1 : value;
}

/** unsigned 4 bytes to signed */
static INLINE int utos4(unsigned int value)
{
  return (value & 0x80000000) ? -(signed)(value ^ 0xffffffff)-1 : value;
}

/** get 1 byte */
static INLINE int mget1(const byte* data)
{
  return data[0];
}

/** get 2 bytes (little-endian) */
static INLINE int mget2l(const byte* data)
{
  return data[0] | (data[1] * 0x0100);
}

/** get 3 bytes (little-endian) */
static INLINE int mget3l(const byte* data)
{
  return data[0] | (data[1] * 0x0100) | (data[2] * 0x010000);
}

/** get 4 bytes (little-endian) */
static INLINE int mget4l(const byte* data)
{
  return data[0] | (data[1] * 0x0100) | (data[2] * 0x010000) | (data[3] * 0x01000000);
}

/** get 2 bytes (big-endian) */
static INLINE int mget2b(const byte* data)
{
  return data[1] | (data[0] * 0x0100);
}

/** get 3 bytes (big-endian) */
static INLINE int mget3b(const byte* data)
{
  return data[2] | (data[1] * 0x0100) | (data[0] * 0x010000);
}

/** get 4 bytes (big-endian) */
static INLINE int mget4b(const byte* data)
{
  return data[3] | (data[2] * 0x0100) | (data[1] * 0x010000) | (data[0] * 0x01000000);
}

/** put 1 byte */
static INLINE int mput1(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/** put 2 bytes (little-endian) */
static INLINE int mput2l(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  return lastPut & 0xff;
}

/** put 3 bytes (little-endian) */
static INLINE int mput3l(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[2] = lastPut & 0xff;
  return lastPut & 0xff;
}

/** put 4 bytes (little-endian) */
static INLINE int mput4l(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[2] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[3] = lastPut & 0xff;
  return lastPut & 0xff;
}

/** put 2 bytes (big-endian) */
static INLINE int mput2b(int value, byte* data)
{
  int lastPut = value;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/** put 3 bytes (big-endian) */
static INLINE int mput3b(int value, byte* data)
{
  int lastPut = value;
  data[2] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/** put 4 bytes (big-endian) */
static INLINE int mput4b(int value, byte* data)
{
  int lastPut = value;
  data[3] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[2] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/** get 1 byte from file */
static INLINE int fget1(FILE* stream)
{
  return fgetc(stream);
}

/** get 2 bytes from file (little-endian) */
static INLINE int fget2l(FILE* stream)
{
  int b1;
  int b2;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF))
  {
    return b1 | (b2 * 0x0100);
  }
  return EOF;
}

/** get 3 bytes from file (little-endian) */
static INLINE int fget3l(FILE* stream)
{
  int b1;
  int b2;
  int b3;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF))
  {
    return b1 | (b2 * 0x0100) | (b3 * 0x010000);
  }
  return EOF;
}

/** get 4 bytes from file (little-endian) */
static INLINE int fget4l(FILE* stream)
{
  int b1;
  int b2;
  int b3;
  int b4;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  b4 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF) && (b4 != EOF))
  {
    return b1 | (b2 * 0x0100) | (b3 * 0x010000) | (b4 * 0x01000000);
  }
  return EOF;
}

/** get 2 bytes from file (big-endian) */
static INLINE int fget2b(FILE* stream)
{
  int b1;
  int b2;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF))
  {
    return b2 | (b1 * 0x0100);
  }
  return EOF;
}

/** get 3 bytes from file (big-endian) */
static INLINE int fget3b(FILE* stream)
{
  int b1;
  int b2;
  int b3;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF))
  {
    return b3 | (b2 * 0x0100) | (b1 * 0x010000);
  }
  return EOF;
}

/** get 4 bytes from file (big-endian) */
static INLINE int fget4b(FILE* stream)
{
  int b1;
  int b2;
  int b3;
  int b4;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  b4 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF) && (b4 != EOF))
  {
    return b4 | (b3 * 0x0100) | (b2 * 0x010000) | (b1 * 0x01000000);
  }
  return EOF;
}

/** put 1 byte to file */
static INLINE int fput1(int value, FILE* stream)
{
  return fputc(value & 0xff, stream);
}

/** put 2 bytes to file (little-endian) */
static INLINE int fput2l(int value, FILE* stream)
{
  int result;

  result = fputc(value & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
  }
  return result;
}

/** put 3 bytes to file (little-endian) */
static INLINE int fput3l(int value, FILE* stream)
{
  int result;

  result = fputc(value & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc((value >> 16) & 0xff, stream);
    }
  }
  return result;
}

/** put 4 bytes to file (little-endian) */
static INLINE int fput4l(int value, FILE* stream)
{
  int result;

  result = fputc(value & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc((value >> 16) & 0xff, stream);
      if(result != EOF)
      {
        result = fputc((value >> 24) & 0xff, stream);
      }
    }
  }
  return result;
}

/** put 2 bytes to file (big-endian) */
static INLINE int fput2b(int value, FILE* stream)
{
  int result;

  result = fputc((value >> 8) & 0xff, stream);
  if(result != EOF)
  {
    result = fputc(value & 0xff, stream);
  }
  return result;
}

/** put 3 bytes to file (big-endian) */
static INLINE int fput3b(int value, FILE* stream)
{
  int result;

  result = fputc((value >> 16) & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc(value & 0xff, stream);
    }
  }
  return result;
}

/** put 4 bytes to file (big-endian) */
static INLINE int fput4b(int value, FILE* stream)
{
  int result;

  result = fputc((value >> 24) & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 16) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc((value >> 8) & 0xff, stream);
      if(result != EOF)
      {
        result = fputc(value & 0xff, stream);
      }
    }
  }
  return result;
}

#endif /* !CBYTEIO_H */
