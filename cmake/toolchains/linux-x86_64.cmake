set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(ZIG_TARGET "x86_64-linux-musl")
set(ZIG_SCRIPTS_DIR "${CMAKE_BINARY_DIR}/.zig-wrappers")
file(MAKE_DIRECTORY "${ZIG_SCRIPTS_DIR}")

file(WRITE "${ZIG_SCRIPTS_DIR}/cc"     "#!/bin/sh\nexec zig cc  -target ${ZIG_TARGET} \"$@\"\n")
file(WRITE "${ZIG_SCRIPTS_DIR}/c++"    "#!/bin/sh\nexec zig c++ -target ${ZIG_TARGET} \"$@\"\n")
file(WRITE "${ZIG_SCRIPTS_DIR}/ar"     "#!/bin/sh\nexec zig ar \"$@\"\n")
file(WRITE "${ZIG_SCRIPTS_DIR}/ranlib" "#!/bin/sh\nexec zig ranlib \"$@\"\n")
file(CHMOD "${ZIG_SCRIPTS_DIR}/cc"     PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
file(CHMOD "${ZIG_SCRIPTS_DIR}/c++"    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
file(CHMOD "${ZIG_SCRIPTS_DIR}/ar"     PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
file(CHMOD "${ZIG_SCRIPTS_DIR}/ranlib" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

set(CMAKE_C_COMPILER   "${ZIG_SCRIPTS_DIR}/cc")
set(CMAKE_CXX_COMPILER "${ZIG_SCRIPTS_DIR}/c++")
set(CMAKE_AR           "${ZIG_SCRIPTS_DIR}/ar")
set(CMAKE_RANLIB       "${ZIG_SCRIPTS_DIR}/ranlib")

set(ZIG_CXX "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "" FORCE)
