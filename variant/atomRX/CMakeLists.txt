add_executable(atomRx
    main.cpp
    board.c
)

target_link_libraries(atomRx PUBLIC proto util log thread driver chibios hal port lld)

