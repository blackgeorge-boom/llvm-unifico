##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Common parts which can be used by all plugins
#
##===----------------------------------------------------------------------===##

add_subdirectory(OMPT)
add_subdirectory(PluginInterface)
add_subdirectory(MemoryManager)
add_subdirectory(elf_common)
