# Game/CMakeLists.txt

file(GLOB_RECURSE GAME_SOURCES
    src/*.cpp
    src/*.h
)

add_executable(Game
    ${GAME_SOURCES}
)

target_link_libraries(Game PRIVATE Engine)

# ------------------------------------------------------------------
# Automatically copy Engine.dll next to Game.exe after every build
# ------------------------------------------------------------------
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Engine>
        $<TARGET_FILE_DIR:Game>
    COMMENT "Copying Engine.dll to output directory"
)

# ------------------------------------------------------------------
# Optional: copy Assets folder at build time
# ------------------------------------------------------------------
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:Game>/Assets"
    COMMENT "Copying Assets folder"
)