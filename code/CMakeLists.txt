
add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${BULLET_PHYSICS_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm stb assimp imgui BulletDynamics BulletCollision LinearMath)

target_sources(${PROJECT_NAME} PRIVATE main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 CXX_STANDART_REQUIRED ON CXX_EXTENSIONS OFF)