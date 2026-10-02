#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <btBulletCollisionCommon.h>

#include <print>

int main()
{
    glfwInit();

    GLFWwindow* window_handle = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);

    glfwMakeContextCurrent(window_handle);

    gladLoadGL();

    Assimp::Importer importer;

    if (const aiScene* scene = importer.ReadFile("test_scene.obj", 0); scene != nullptr)
    {
        std::println("Scene loading...");
    }

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    while (glfwWindowShouldClose(window_handle) == GLFW_FALSE)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window_handle);
        glfwPollEvents();
    }

    glfwDestroyWindow(window_handle);
    glfwTerminate();

    return 0;
}
