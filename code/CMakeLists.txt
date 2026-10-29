add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${BULLET_PHYSICS_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE glad glm stb assimp imgui BulletDynamics BulletCollision LinearMath platforms-module graphics-module utilities_module)

target_sources(${PROJECT_NAME} PRIVATE main.cpp)