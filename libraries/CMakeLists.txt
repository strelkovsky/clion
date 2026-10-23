add_subdirectory(glfw)
add_subdirectory(glad)

add_subdirectory(assimp)
add_subdirectory(stb)

add_subdirectory(bullet)
add_subdirectory(glm)

add_subdirectory(imgui)
add_subdirectory(imgui/examples)

add_subdirectory(imguizmo)

add_subdirectory(recastnavigation)

set(ozz_build_msvc_rt_dll ON CACHE BOOL "Use MSVC DLL runtime" FORCE)
add_subdirectory(ozz-animation)

add_subdirectory(cereal)

add_subdirectory(behaviortree-cpp)

add_subdirectory(SDL)