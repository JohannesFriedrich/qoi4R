test_that("writeQOI works as expected", {
  bin <- writeQOI(Rlogo_RGBA)

  # check output type and length
  expect_type(bin, "raw")
  expect_length(bin, 78635)
  expect_equal(rawToChar(head(bin, n = 6)), "qoif")

  # check if wrong input is given
  expect_error(writeQOI())
  expect_no_error(writeQOI(Rlogo_RGBA))

  # check string/path output
  writeQOI(Rlogo_RGBA, "Rlogo.qoi")
  expect_true(file.exists("Rlogo.qoi"))
  file.remove("Rlogo.qoi")

  file <- file("Rlogo.qoi", "wb")
  writeQOI(Rlogo_RGBA, file)
  close(file)
  expect_true(file.exists("Rlogo.qoi"))
  file.remove("Rlogo.qoi")
})
