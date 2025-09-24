#' Read an QOI image into a RGB(A) raster array
#' @param qoi_image_path [character] (**required**): Path to a stored qoi-image
#' @return A matrix with integer (0-255) RGB(A) values with dimensions height x
#' width x channels. Until now 3 (RGB) and 4 (RGBA) channels are integrated in
#' the specification.
#' If the decoding went wrong the returned value is NULL.
#' @author Johannes Friedrich
#' @examples
#' ## (1) Read RGBA values from file
#' path <- system.file("extdata", "Rlogo.qoi", package="qoi")
#' rlogo_qoi <- readQOI(path)
#' dim(rlogo_qoi)
#'
#' ## (2) plot them
#' plot.new()
#  rasterImage(rlogo_qoi/255, xleft = 0, xright = 1,
#              ytop = 0, ybottom = 1, interpolate = FALSE)
#' @md
#' @export
readQOI <- function(qoi_image_path) {
  .Call(qoiRead_, if (is.raw(qoi_image_path)) qoi_image_path else path.expand(qoi_image_path))
}
