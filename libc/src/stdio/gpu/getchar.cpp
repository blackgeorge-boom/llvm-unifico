//===-- GPU implementation of getchar -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "src/stdio/getchar.h"
#include "file.h"

#include <stdio.h>

namespace __llvm_libc {

LLVM_LIBC_FUNCTION(int, getchar, ()) {
  unsigned char c;
  size_t r = file::read(stdin, &c, 1);

  if (r != 1)
    return EOF;
  return c;
}

} // namespace __llvm_libc
