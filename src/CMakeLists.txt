set(CLICHARM_SRCS "main.c"
                  "db.c"
                  "task.c")

include_directories(AFTER SYSTEM ${SQLITE_INCLUDE_DIR})

find_library(M_LIB m)

add_executable(ccharm ${CLICHARM_SRCS})

target_link_libraries(ccharm ${SQLITE_LIBRARIES} ${M_LIB})

