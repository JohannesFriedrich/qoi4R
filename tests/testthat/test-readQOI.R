test_that("readQOI works as expected", {
  path_qoi <- system.file("extdata", "Rlogo.qoi", package="qoi")
  path_png <- system.file("extdata", "Rlogo.png", package="qoi")
  rlogo_qoi <- readQOI(path_qoi)

  # check output type and dim
  expect_equal(class(rlogo_qoi), "array")
  expect_type(rlogo_qoi, "integer")
  expect_equal(dim(rlogo_qoi), c(561, 724, 4))

  # check if wrong input is given
  expect_error(readQOI())
  expect_error(readQOI(path_png))

})
