#ifdef USE_GLES2
    #include <GLES2/gl2.h>
#else
    #include <GL/glew.h>  // Changed back to GL/glew.h
#endif

#include <GLFW/glfw3.h>
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "../external/glm/glm/glm.hpp"
#include "../external/glm/glm/gtc/matrix_transform.hpp"
#include <iostream>
#include "entity.h"
#include "scene.h"

Camera camera(glm::vec3(0.0f, 0.2f, 5.0f));
float lastFrameTime = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
float deltaTime = 0.0f;

void drawDebugAxes(const glm::mat4& projection, const glm::mat4& view) {
    static GLuint axisVAO = 0;
    static GLuint axisVBO = 0;
    static Shader* axisShader = nullptr;

    if (axisVAO == 0) {
        // Create axis shader
        const char* axisVertexShader = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aColor;
            uniform mat4 projection;
            uniform mat4 view;
            out vec3 Color;
            void main() {
                Color = aColor;
                gl_Position = projection * view * vec4(aPos, 1.0);
            }
        )";

        const char* axisFragmentShader = R"(
            #version 330 core
            in vec3 Color;
            out vec4 FragColor;
            void main() {
                FragColor = vec4(Color, 1.0);
            }
        )";

        // Axis vertices: position and color
        float axisVertices[] = {
            // positions      // colors
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // X axis start (red)
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // X axis end
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Y axis start (green)
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Y axis end
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Z axis start (blue)
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f   // Z axis end
        };

        // Create and setup axis buffers
        glGenVertexArrays(1, &axisVAO);
        glGenBuffers(1, &axisVBO);
        
        glBindVertexArray(axisVAO);
        glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Create shader program
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &axisVertexShader, NULL);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &axisFragmentShader, NULL);
        glCompileShader(fragmentShader);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        axisShader = new Shader(shaderProgram);
    }

    // Draw axes
    axisShader->use();
    axisShader->setMat4("projection", projection);
    axisShader->setMat4("view", view);

    glBindVertexArray(axisVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard('W', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard('S', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard('A', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard('D', deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

#ifdef USE_GLES2
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
#endif

    // Enable multisampling
    glfwWindowHint(GLFW_SAMPLES, 4);  // 4x MSAA
    
    GLFWwindow* window = glfwCreateWindow(1200, 1200, "GLB Viewer", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#ifndef USE_GLES2
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
#endif

    // Set viewport size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Disable face culling to render both sides
    glDisable(GL_CULL_FACE);
    // Or if you want to explicitly set the culling mode:
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_NONE);

    // Enable wireframe mode for debugging
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // After enabling depth test
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable multisampling
    glEnable(GL_MULTISAMPLE);
    
    // Enable line/edge antialiasing
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    try {
        Scene scene;
        
        // Add shaders first
        std::cout << "\nLoading Shaders:" << std::endl;
        scene.AddShader("background", "shaders/gltf.vert", "shaders/gltf.frag");
        scene.AddShader("standard", "shaders/vertex.glsl", "shaders/fragment.glsl");
        
        // Add models
        std::cout << "\nLoading Models:" << std::endl;
        scene.AddModel("background", "assets/models/bz_background.glb");
        scene.AddModel("tank", "assets/models/tank.glb");
        
        std::cout << "\nCreating Entities:" << std::endl;
        
        // Create background first
        Entity* background = scene.CreateEntity("background", "background",
            glm::vec3(0.0f),           // Position at origin
            glm::vec3(0.0f, 0.0f, 0.0f),  // Rotate to face up
            glm::vec3(0.25f));         // Scale to 0.25
        
        if (!background) {
            throw std::runtime_error("Failed to create background entity");
        }
        
        // Print debug info
        std::cout << "Background entity created with:" << std::endl;
        std::cout << "Position: 0, 0, 0" << std::endl;
        std::cout << "Rotation: -90, 0, 0" << std::endl;
        std::cout << "Scale: 0.25" << std::endl;
        
        // Then create tank
        Entity* tank = scene.CreateEntity("tank", "standard",
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(-90.0f, 0.0f, 0.0f),
            glm::vec3(0.1f));
        
        if (!tank) {
            throw std::runtime_error("Failed to create tank entity");
        }

        while (!glfwWindowShouldClose(window)) {
            // Calculate delta time
            float currentTime = glfwGetTime();
            deltaTime = currentTime - lastFrameTime;
            lastFrameTime = currentTime;

            processInput(window);
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            scene.Update(deltaTime);
            scene.Draw(camera);
            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    glfwTerminate();
    return 0;
} 