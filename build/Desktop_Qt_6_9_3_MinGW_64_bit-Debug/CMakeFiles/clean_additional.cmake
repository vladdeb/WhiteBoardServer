# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\WhiteBoardServer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\WhiteBoardServer_autogen.dir\\ParseCache.txt"
  "WhiteBoardServer_autogen"
  )
endif()
