#include <R.h>
#include <Rinternals.h>

#include <stdio.h>
#include "qoi.h"

void *qoi_encode(const void *data, const qoi_desc *desc, int *out_len) {
  int i, max_size, p, run;
  int px_len, px_end, px_pos, channels;
  unsigned char *bytes;
  const unsigned char *pixels;
  qoi_rgba_t index[64];
  qoi_rgba_t px, px_prev;

  if (
      data == NULL || out_len == NULL || desc == NULL ||
        desc->width == 0 || desc->height == 0 ||
        desc->channels < 3 || desc->channels > 4 ||
        desc->colorspace > 1 ||
        desc->height >= QOI_PIXELS_MAX / desc->width
  ) {
    return NULL;
  }

  max_size =
    desc->width * desc->height * (desc->channels + 1) +
    QOI_HEADER_SIZE + sizeof(qoi_padding);

  p = 0;
  bytes = (unsigned char *) QOI_MALLOC(max_size);
  if (!bytes) {
    return NULL;
  }

  qoi_write_32(bytes, &p, QOI_MAGIC);
  qoi_write_32(bytes, &p, desc->width);
  qoi_write_32(bytes, &p, desc->height);
  bytes[p++] = desc->channels;
  bytes[p++] = desc->colorspace;


  pixels = (const unsigned char *)data;

  QOI_ZEROARR(index);

  run = 0;
  px_prev.rgba.r = 0;
  px_prev.rgba.g = 0;
  px_prev.rgba.b = 0;
  px_prev.rgba.a = 255;
  px = px_prev;

  px_len = desc->width * desc->height * desc->channels;
  px_end = px_len - desc->channels;
  channels = desc->channels;

  for (px_pos = 0; px_pos < px_len; px_pos += channels) {
    px.rgba.r = pixels[px_pos + 0];
    px.rgba.g = pixels[px_pos + 1];
    px.rgba.b = pixels[px_pos + 2];

    if (channels == 4) {
      px.rgba.a = pixels[px_pos + 3];
    }

    if (px.v == px_prev.v) {
      run++;
      if (run == 62 || px_pos == px_end) {
        bytes[p++] = QOI_OP_RUN | (run - 1);
        run = 0;
      }
    }
    else {
      int index_pos;

      if (run > 0) {
        bytes[p++] = QOI_OP_RUN | (run - 1);
        run = 0;
      }

      index_pos = QOI_COLOR_HASH(px) % 64;

      if (index[index_pos].v == px.v) {
        bytes[p++] = QOI_OP_INDEX | index_pos;
      }
      else {
        index[index_pos] = px;

        if (px.rgba.a == px_prev.rgba.a) {
          signed char vr = px.rgba.r - px_prev.rgba.r;
          signed char vg = px.rgba.g - px_prev.rgba.g;
          signed char vb = px.rgba.b - px_prev.rgba.b;

          signed char vg_r = vr - vg;
          signed char vg_b = vb - vg;

          if (
              vr > -3 && vr < 2 &&
                vg > -3 && vg < 2 &&
                vb > -3 && vb < 2
          ) {
            bytes[p++] = QOI_OP_DIFF | (vr + 2) << 4 | (vg + 2) << 2 | (vb + 2);
          }
          else if (
              vg_r >  -9 && vg_r <  8 &&
                vg   > -33 && vg   < 32 &&
                vg_b >  -9 && vg_b <  8
          ) {
            bytes[p++] = QOI_OP_LUMA     | (vg   + 32);
            bytes[p++] = (vg_r + 8) << 4 | (vg_b +  8);
          }
          else {
            bytes[p++] = QOI_OP_RGB;
            bytes[p++] = px.rgba.r;
            bytes[p++] = px.rgba.g;
            bytes[p++] = px.rgba.b;
          }
        }
        else {
          bytes[p++] = QOI_OP_RGBA;
          bytes[p++] = px.rgba.r;
          bytes[p++] = px.rgba.g;
          bytes[p++] = px.rgba.b;
          bytes[p++] = px.rgba.a;
        }
      }
    }
    px_prev = px;
  }

  for (i = 0; i < (int)sizeof(qoi_padding); i++) {
    bytes[p++] = qoi_padding[i];
  }

  *out_len = p;
  return bytes;
}

SEXP qoiWrite_(SEXP image, SEXP sFilename){
  SEXP dims;
  SEXP res = R_NilValue;
  const char *fn;
  int channels = 1, raw_array = 0, width, height;
  FILE *f=0;

  // check type of image-input
  if (TYPEOF(image) == RAWSXP) raw_array = 1;
  if (!raw_array && TYPEOF(image) != INTSXP)
    Rf_error("image must be a matrix or array of raw or integer numbers");

  if (TYPEOF(sFilename) == RAWSXP) {
    f = 0;
  } else {
    if (TYPEOF(sFilename) != STRSXP || LENGTH(sFilename) < 1) Rf_error("invalid filename");
    fn = CHAR(STRING_ELT(sFilename, 0));
    f = fopen(fn, "wb");
    if (!f) Rf_error("unable to create %s", fn);
  }

  dims = Rf_getAttrib(image, R_DimSymbol);
  if (dims == R_NilValue || TYPEOF(dims) != INTSXP || LENGTH(dims) < 2 || LENGTH(dims) > 3)
    Rf_error("image must be a matrix or an array of two or three dimensions");

  if (raw_array && LENGTH(dims) == 3) { /* raw arrays have either bpp, width, height or width, height dimensions */
    channels = INTEGER(dims)[0];
    width = INTEGER(dims)[1];
    height = INTEGER(dims)[2];
  } else { /* others have width, height[, bpp] */
    width = INTEGER(dims)[1];
    height = INTEGER(dims)[0];
    if (LENGTH(dims) == 3)
      channels = INTEGER(dims)[2];
  }

  int DataSize = width * height * channels;

  if (channels < 3 || channels > 4)
    Rf_error("image must have either 3 (RGB) or 4 (RGBA) channels");

  // prepare incoming matrix as RGB(A) stream:
  // see: https://github.com/hadley/r-internals/blob/master/vectors.md#get-and-set-values
  int* dataPtr = INTEGER(image);

  unsigned char rgb_values[DataSize];

  // convert result of qoi_decode to rgb array
  int counter = 0;
  for(int y = 0; y < height; y++){
    for (int x = 0; x < width; x++){
      for (int c = 0; c < channels; c++){
        rgb_values[counter] = dataPtr[y + x * height + c * height*width];
        counter++;
      }
    }
  }

  // write desc from dimensions of incoming array
  qoi_desc desc;
  desc.channels = channels;
  desc.height = height;
  desc.width = width;
  desc.colorspace = 1;

  int size;
  void* encoded;

  encoded = qoi_encode(&rgb_values, &desc, &size);

  if (!encoded) {
    fclose(f);
    return R_NilValue;
  }

  if (f) { /* if it is a file, just return */
    fwrite(encoded, 1, size, f);
    fclose(f);
    return R_NilValue;
  }

  // copy encoded data into SEXP res
  res = PROTECT(allocVector(RAWSXP, size));
  unsigned char *byte = RAW(res);
  for (int i=0; i<size; i++){
    byte[i] = ((unsigned char*) encoded)[i];
  }

  UNPROTECT(1);
  QOI_FREE(encoded);
  return res;
}
