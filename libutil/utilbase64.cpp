#include <stdlib.h>
#include <string.h>
#include <stdexcept>

static const char b64enc[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char b64dec[256] =
{
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0-7 */
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 8-15 */
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 16-23 */
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 24-31 */
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 32-39 */
  0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f, /* 40-47 */
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, /* 48-55 */
  0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 56-63 */
  0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /* 64-71 */
  0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, /* 72-79 */
  0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, /* 80-87 */
  0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff, /* 88-95 */
  0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, /* 96-103 */
  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, /* 104-111 */
  0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, /* 112-119 */
  0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, /* 120-127 */

  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 128-255 */
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

void base64_encode(const void *src, size_t srclen, char *dst, size_t *dstlen)
{
  const unsigned char *src_c = (const unsigned char *)src;
  unsigned char tmp[4];
  size_t dstpos = 0;

  for (size_t i = 0; i < srclen; ) {
    tmp[0] = b64enc[src_c[i] >> 2];
    switch (srclen - i) {
      case 1:
        tmp[1] = b64enc[(src_c[i] & 0x03) << 4];
        tmp[2] = '=';
        tmp[3] = '=';
        i++;
        break;
      case 2:
        tmp[1] = b64enc[((src_c[i] & 0x03) << 4) | (src_c[i+1] >> 4)];
        tmp[2] = b64enc[((src_c[i+1] & 0x0f) << 2)];
        tmp[3] = '=';
        i += 2;
        break;
      default:
        tmp[1] = b64enc[((src_c[i] & 0x03) << 4) | (src_c[i+1] >> 4)];
        tmp[2] = b64enc[((src_c[i+1] & 0x0f) << 2) | ((src_c[i+2] & 0xc0) >> 6)];
        tmp[3] = b64enc[src_c[i+2] & 0x3f];
        i += 3;
        break;
    }
    if (dstpos + 4 > *dstlen)
      throw std::logic_error("base64 encoding failure, buffer overflow");
    memcpy(dst+dstpos, tmp, 4);
    dstpos += 4;
  }
}


#define IS_EMPTY(c) \
((c) == '\n' || (c) == '\r' || (c) == ' ' || (c) == '\t')

int base64_decode(const void *src, size_t srclen, size_t *srcpos, buffer_t *dest)
{
  const unsigned char *src_c = (const unsigned char *)src;

  unsigned char input[4], output[3];
  int ret = 1;

  for (size_t i = 0; i+3 < srclen; ) {
    input[0] = b64dec[src_c[i]];
    if (input[0] == 0xff) {
      if (!IS_EMPTY(src_c[i])) {
        ret = -1;
        break;
      }
      i++;
      continue;
    }

    input[1] = b64dec[src_c[i+1]];
    if (input[1] == 0xff) {
      ret = -1;
      break;
    }
    output[0] = (input[0] << 2) | (input[1] >> 4);

    input[2] = b64dec[src_c[i+2]];
    if (input[2] == 0xff) {
      if (src_c[i+2] != '=' || src_c[i+3] != '=') {
        ret = -1;
        break;
      }
      buffer_append(dest, output, 1);
      ret = 0;
      i += 4;
      break;
    }

    output[1] = (input[1] << 4) | (input[2] >> 2);
    input[3] = b64dec[src_c[i+3]];
    if (input[3] == 0xff) {
      if (src_c[i+3] != '=') {
        ret = -1;
        break;
      }
      buffer_append(dest, output, 2);
      ret = 0;
      i += 4;
      break;
    }

    output[2] = ((input[2] << 6) & 0xc0) | input[3];
    buffer_append(dest, output, 3);
    i += 4;
  }

  for (; i < srclen; i++) {
    if (!IS_EMPTY(src_c[i]))
      break;
  }

  if (srcpos != NULL)
    *srcpos = i;

  return ret;
}
