
<!-- README.md is generated from README.Rmd. Please edit that file -->
<!-- badges: start -->

[![CRAN](https://www.r-pkg.org/badges/version/qoi)](https://cran.r-project.org/web/packages/qoi/index.html)
[![Downloads](https://cranlogs.r-pkg.org/badges/grand-total/qoi)](https://www.r-pkg.org/pkg/qoi)
[![R-CMD-check](https://github.com/JohannesFriedrich/qoi4R/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/JohannesFriedrich/qoi4R/actions/workflows/R-CMD-check.yaml)
[![Project Status: Active – The project has reached a stable, usable
state and is being actively
developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![codecov](https://codecov.io/github/JohannesFriedrich/qoi4R/graph/badge.svg?token=SHhbrXi8yH)](https://codecov.io/github/JohannesFriedrich/qoi4R)

<!-- badges: end -->

# QOI

QOI is fast. It losslessy compresses images to a similar size of PNG,
while offering 20x-50x faster encoding and 3x-4x faster decoding.

QOI is simple. The reference en-/decoder fits in about 300 lines of C.
The file format specification is a single page PDF.

<https://qoiformat.org/>

This **R**-package is a wrapper to handle QOI files with **R**. The
package is completely written in base R with no dependencies and pure
C-code.

<!-- badges: start -->
<!-- badges: end -->

## Installation

The package is on CRAN. The easiest way to download is via:

``` r
install.packages("qoi")
```

You can install the development version from GitHub with the following
command:

``` r
if (!require("devtools")) install.packages("devtools")
devtools::install_github("JohannesFriedrich/qoi4R")
```

## Usage

There are just two main functions: `readQOI()` and `writeQOI()`.

- `readQOI()`: Takes an qoi-format image and decodes it into its RGB or
  RGBA values. The result is a matrix with dimensions height x width x
  channels.
- `writeQOI()`: Takes an RGB(A) matrix and encodes it into an qoi-image.

### `readQOI()`

Let´s read in an QOI-image delivered with this package:

``` r
library(qoi)
path <- system.file("extdata", "Rlogo.qoi", package="qoi")

Rlogo_pixels <- readQOI(path)
```

The dimension of `Rlogo_pixels` is 561, 724, 4. So the height of the
image is 561 px, the width 724 px and it has 4 channels, so it´s RGBA.
With `3` channels it would be RGB encoded.

With `readQOI()` is it possible to show the image with **R**.

``` r
plot.new()
rasterImage(Rlogo_pixels/255, xleft = 0, xright = 1,
            ytop = 0, ybottom = 1, interpolate = FALSE)
```

<img src="README_figs/README-unnamed-chunk-4-1.png" width="672" style="display: block; margin: auto;" />

The pixels returned from `readQOI()` are integer values between 0 - 255.
To show them with `rasteImage()` it is necessary to divide them by 255
to get values between 0-1.

Let´s compare the result to the wonderful
[PNG-package](https://cran.r-project.org/web/packages/png). The results
from `readPNG()` are numerical values between 0 and 1. To compare the
results multiply with 255 and change the mode to integer.

``` r
qoi_logo_rgb_png <- png::readPNG(system.file("extdata", "qoi_logo.png", package="qoi"))*255L

qoi_logo_rgb_qoi <- qoi::readQOI(system.file("extdata", "qoi_logo.qoi", package="qoi"))

mode(qoi_logo_rgb_png) <- "integer"

identical(qoi_logo_rgb_png, qoi_logo_rgb_qoi)
## [1] TRUE
```

### `writeQOI()`

With this function it is possible to save an RGB(A) matrix to an
QOI-image. The input arguments are:

- image: a matrix with dimensions height x width x channels
- target: Either name of the file to write, a binary connection or a raw
  vector indicating that the output should be a raw vector (so the
  hex-interpretation of the image).

If no second argument is given, the returned value is the
hex-interpretation of the image in the QOI-file format.

``` r
qoi_logo <- writeQOI(qoi_logo_rgb_qoi)
rawToChar(head(qoi_logo))
## [1] "qoif"
```

If an second argument is given as character the image is saved to this
name:

``` r
writeQOI(qoi_logo_rgb_qoi, "qoi_logo_rgb.qoi")
```

If the second argument is of type `connection` the hex interpretation of
the QOI-image will be send to this connection.

``` r
file <- file("file.qoi", "wb")
writeQOI(qoi_logo_rgb_qoi, file)
close(file)
```

## Acknowlegment

This package would not exist without the following
persons/homepages/tutorial/…:

- [Phoboslab - original specification](https://github.com/phoboslab/qoi)
- [PNG R-package](https://github.com/s-u/png)
- [Example C-code
  R-package](https://github.com/coolbutuseless/simplecall)
- [R
  extensions](https://cran.r-project.org/doc/manuals/r-release/R-exts.html)
- [Hadley´s R-Internals](https://github.com/hadley/r-internals)
