project(glad)

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_sources(${PROJECT_NAME} PUBLIC src/glad.c)