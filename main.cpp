#include <GL/glew.h>     
#include <GLFW/glfw3.h>  
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "shape.h"
#include "input.h"
#include "globals.h"
#include "HIERARCHIAL.h"

// Declare Global variables
std::shared_ptr<model_t> currentModel;
std::shared_ptr<model_node_t> currentNode;
int currentShapeIndex = -1;
std::vector<std::shared_ptr<model_node_t>> allShapes;

glm::mat4 projection;
glm::mat4 view;
GLuint shaderProgram = 0;
Mode currentMode = MODELLING;
TransformMode transformMode = NONE;
char activeAxis = 'X';



float cameraDistance = 5.0f;
float cameraAngleX = 0.0f;
float cameraAngleY = 0.0f;
glm::mat4 modelRotation = glm::mat4(1.0f);


// Lighting variables
bool lightingEnabled = true;
glm::vec3 lightPosition = glm::vec3(5.0f, 5.0f, 5.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
float ambientStrength = 0.3f;
float diffuseStrength = 0.7f;
float specularStrength = 0.5f;
float shininess = 32.0f;

// shaders
GLuint createShaderProgram() {
    // Vertex shader with Gouraud shading
    const char* vertexShaderSrc = R"(
    #version 330 core
    layout(location = 0) in vec4 aPos;
    layout(location = 1) in vec4 aColor;
    layout(location = 2) in vec3 aNormal;
    
    uniform mat4 MVP;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    // Lighting uniforms
    uniform bool enableLighting;
    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 viewPos;
    uniform float ambientStrength;
    uniform float diffuseStrength;
    uniform float specularStrength;
    uniform float shininess;
    
    out vec4 fragColor;
    
    void main() {
        gl_Position = MVP * aPos;
        
        if (enableLighting) {
            // Transform position and normal to world space
            vec3 fragPos = vec3(model * aPos);
            vec3 normal = normalize(mat3(transpose(inverse(model))) * aNormal);
            
            // Ambient
            vec3 ambient = ambientStrength * lightColor;
            
            // Diffuse
            vec3 lightDir = normalize(lightPos - fragPos);
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diffuseStrength * diff * lightColor;
            
            // Specular
            vec3 viewDir = normalize(viewPos - fragPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec3 specular = specularStrength * spec * lightColor;
            
            // Combine lighting with color
            vec3 result = (ambient + diffuse + specular) * vec3(aColor);
            fragColor = vec4(result, aColor.a);
        } else {
            // No lighting, use vertex color directly
            fragColor = aColor;
        }
    })";

    // Fragment shader (simpler since lighting is done in vertex shader)
    const char* fragmentShaderSrc = R"(
    #version 330 core
    in vec4 fragColor;
    out vec4 color;
    void main() {
        color = fragColor;
    })";

    // Compile vertex & fragment shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// recursively renders a hierarchical model
void renderNode(std::shared_ptr<model_node_t> node, const glm::mat4& parentTransform) {
    if (!node) return;
    glm::mat4 modelMatrix = parentTransform * node->getTransform();

    if (node->shape) {
        glm::mat4 MVP = projection * view * modelMatrix;

        // Set all uniforms for lighting
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"),
            1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"),
            1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),
            1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));

        // Lighting uniforms
        glUniform1i(glGetUniformLocation(shaderProgram, "enableLighting"), lightingEnabled);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPosition));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

        // Camera position for specular calculation
        glm::vec3 cameraPos = glm::vec3(
            cameraDistance * sin(glm::radians(cameraAngleY)) * cos(glm::radians(cameraAngleX)),
            cameraDistance * sin(glm::radians(cameraAngleX)),
            cameraDistance * cos(glm::radians(cameraAngleY)) * cos(glm::radians(cameraAngleX))
        );
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        // Lighting parameters
        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "diffuseStrength"), diffuseStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), specularStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), shininess);

        node->shape->draw(MVP, shaderProgram);
    }

    for (auto& child : node->children) {
        renderNode(child, modelMatrix);
    }
}

void renderScene() {
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    if (currentMode == INSPECTION) {
        view = glm::lookAt(
            glm::vec3(cameraDistance * sin(glm::radians(cameraAngleY)) * cos(glm::radians(cameraAngleX)),
                cameraDistance * sin(glm::radians(cameraAngleX)),
                cameraDistance * cos(glm::radians(cameraAngleY)) * cos(glm::radians(cameraAngleX))),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        if (currentModel && currentModel->getRoot()) {
            renderNode(currentModel->getRoot(), modelRotation);
        }
    }
    else {
        view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        if (currentModel && currentModel->getRoot()) {
            renderNode(currentModel->getRoot(), glm::mat4(1.0f));
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "24b0020_24b2165", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

    shaderProgram = createShaderProgram();
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program\n";
        return -1;
    }
    std::cout << "Shaders compiled and linked successfully!" << std::endl;
    std::cout << "Press 'N' to toggle lighting ON/OFF" << std::endl;

    currentModel = std::make_shared<model_t>();
    currentNode = currentModel->getRoot();
    glfwSetKeyCallback(window, keyCallback);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "enableLighting"), lightingEnabled ? 1 : 0);
        renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
