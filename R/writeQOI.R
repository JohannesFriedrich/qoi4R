#' Write an QOI image from an RGB(A) raster array or matrix
#' @param image [matrix] (**required**): Image represented by a integer matrix or array with values in the range of 0 to 255.
#' @param target [character] or [connections] or [raw]: Either name of the file to write, a binary connection or a raw vector (raw() - the default - is good enough) indicating that the output should be a raw vector.
#' @return The result is either stored in a file (if target is a file name), in a raw vector (if target is a raw vector) or sent to a binary connection.
#' @author Johannes Friedrich
#' @examples
#' ## (1) Write to raw() -> see bytes
#' bin <- writeQOI(Rlogo_RGBA)
#' rawToChar(head(bin)) ## qoif
#'
#' \dontrun{
#' ## (2) Write to a *.qoi file
#' writeQOI(Rlogo_RGBA, "Rlogo_RGBA.qoi")
#' }
#' @md
#' @export
writeQOI <- function(image, target = raw()) {
  if (inherits(target, "connection")) {
    r <- .Call(qoiWrite_, image, raw())
    writeBin(r, target)
    invisible(NULL)
  } else invisible(.Call(qoiWrite_, image, if (is.raw(target)) target else path.expand(target)))
}
