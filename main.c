#include <stdio.h>
#include <stdlib.h>

#include "src/matrix/matrix.h"
#define MATRIX

#include "src/image/binarization.h"

int main()
{
  char* file_name = "lib/testbmp/10x10.bmp";

  struct MatrixUCHAR mat = binarization(file_name);
  printMatrixUCHAR(mat);
}
