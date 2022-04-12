#include <R.h>
#include <Rinternals.h>

#include <stdio.h>
#include "qoi.h"

void *qoi_decode(const void *data, int size, qoi_desc *desc, int channels) {
  if (desc == NULL) Rf_error("desc is NULL");
  const unsigned char *bytes;
  unsigned int header_magic;
  unsigned char *pixels;
  qoi_rgba_t index[64];
  qoi_rgba_t px;
  int px_len, chunks_len, px_pos;
  int p = 0, run = 0;

  if (
      data == NULL || desc == NULL ||
        (channels != 0 && channels != 3 && channels != 4) ||
        size < QOI_HEADER_SIZE + (int)sizeof(qoi_padding)
  ) {
    return NULL;
  }

  bytes = (const unsigned char *)data;

  header_magic = qoi_read_32(bytes, &p);
  desc->width = qoi_read_32(bytes, &p);
  desc->height = qoi_read_32(bytes, &p);
  desc->channels = bytes[p++];
  desc->colorspace = bytes[p++];

  if (
      desc->width == 0 || desc->height == 0 ||
        desc->channels < 3 || desc->channels > 4 ||
        desc->colorspace > 1 ||
        header_magic != QOI_MAGIC ||
        desc->height >= QOI_PIXELS_MAX / desc->width
  ) {
    return NULL;
  }

  if (channels == 0) {
    channels = desc->channels;
  }

  px_len = desc->width * desc->height * channels;
  pixels = (unsigned char *) QOI_MALLOC(px_len);
  if (!pixels) {
    return NULL;
  }

  QOI_ZEROARR(index);
  px.rgba.r = 0;
  px.rgba.g = 0;
  px.rgba.b = 0;
  px.rgba.a = 255;

  chunks_len = size - (int)sizeof(qoi_padding);
  for (px_pos = 0; px_pos < px_len; px_pos += channels) {
    if (run > 0) {
      run--;
    }
    else if (p < chunks_len) {
      int b1 = bytes[p++];

      if (b1 == QOI_OP_RGB) {
        px.rgba.r = bytes[p++];
        px.rgba.g = bytes[p++];
        px.rgba.b = bytes[p++];
      }
      else if (b1 == QOI_OP_RGBA) {
        px.rgba.r = bytes[p++];
        px.rgba.g = bytes[p++];
        px.rgba.b = bytes[p++];
        px.rgba.a = bytes[p++];
      }
      else if ((b1 & QOI_MASK_2) == QOI_OP_INDEX) {
        px = index[b1];
      }
      else if ((b1 & QOI_MASK_2) == QOI_OP_DIFF) {
        px.rgba.r += ((b1 >> 4) & 0x03) - 2;
        px.rgba.g += ((b1 >> 2) & 0x03) - 2;
        px.rgba.b += ( b1       & 0x03) - 2;
      }
      else if ((b1 & QOI_MASK_2) == QOI_OP_LUMA) {
        int b2 = bytes[p++];
        int vg = (b1 & 0x3f) - 32;
        px.rgba.r += vg - 8 + ((b2 >> 4) & 0x0f);
        px.rgba.g += vg;
        px.rgba.b += vg - 8 +  (b2       & 0x0f);
      }
      else if ((b1 & QOI_MASK_2) == QOI_OP_RUN) {
        run = (b1 & 0x3f);
      }

      index[QOI_COLOR_HASH(px) % 64] = px;
    }

    pixels[px_pos + 0] = px.rgba.r;
    pixels[px_pos + 1] = px.rgba.g;
    pixels[px_pos + 2] = px.rgba.b;

    if (channels == 4) {
      pixels[px_pos + 3] = px.rgba.a;
    }
  }

  return pixels;
}

SEXP qoiRead_(SEXP sFilename) {
  const char *fn;
  char *data;
  unsigned char *pixels;
  FILE *f=0;
  qoi_desc desc;

  if (TYPEOF(sFilename) == RAWSXP) {
    data = (char*) RAW(sFilename);
  } else {
    if (TYPEOF(sFilename) != STRSXP || LENGTH(sFilename) < 1) Rf_error("invalid filename");
    fn = CHAR(STRING_ELT(sFilename, 0));
    f = fopen(fn, "rb");
    if (!f) Rf_error("unable to open %s", fn);
  }

  // read size of bin file and open it, buffer result into data
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  if (size <= 0) {
    fclose(f);
    Rf_error("File has size 0");
  }
  fseek(f, 0, SEEK_SET);

  data = QOI_MALLOC(size);
  if (!data) {
    fclose(f);
    Rf_error("Malloc error!");
  }

  int bytes_read = fread(data, 1, size, f);
  fclose(f);

  // check header:
  int p = 0;
  unsigned int header_magic = qoi_read_32((const unsigned char*)data, &p);
  if (header_magic != QOI_MAGIC) Rf_error("Wrong file format!");

  // give the data to qoi_decode
  pixels = qoi_decode(data, bytes_read, &desc, 0);
  QOI_FREE(data);

  if (pixels == NULL) {
    Rf_error("Decoding went wrong!");
    return R_NilValue;
  }

  // convert pixels to vector of pixels
  int height = desc.height;
  int width = desc.width;
  int channels = desc.channels;
  int n_pixels = height * width * channels;

  SEXP res;
  res = PROTECT(allocVector(INTSXP, n_pixels));
  // see: https://github.com/hadley/r-internals/blob/master/vectors.md#get-and-set-values
  int* px = INTEGER(res);

  // convert result of qoi_decode to rgb array
  int counter = 0;
  for(int y = 0; y < height; y++){
    for (int x = 0; x < width; x++){
      for (int c = 0; c < channels; c++){
        px[y + x * height + c * height*width] = pixels[counter];
        counter++;
      }
    }
  }

  // Set dimensions for export to R
  SEXP dim;
  dim = allocVector(INTSXP, 3);
  INTEGER(dim)[0] = height;
  INTEGER(dim)[1] = width;
  INTEGER(dim)[2] = channels;
  setAttrib(res, R_DimSymbol, dim);
  UNPROTECT(1);

  return res;
}
