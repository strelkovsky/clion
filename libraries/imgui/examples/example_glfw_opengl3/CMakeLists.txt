project(glfw_opengl3_imgui)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

target_sources(${PROJECT_NAME} PRIVATE main.cpp ../../imgui_demo.cpp)