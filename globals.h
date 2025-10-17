#ifndef GLOBALS_H
#define GLOBALS_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <memory>

// Forward declarations
struct model_node_t;
struct model_t;

// Matrices
extern glm::mat4 projection;
extern glm::mat4 view;
extern GLuint shaderProgram;

// Modes
enum Mode { MODELLING, INSPECTION };
enum TransformMode { NONE, ROTATE, TRANSLATE, SCALE };
extern Mode currentMode;
extern TransformMode transformMode;
extern char activeAxis;

// Model and node pointers
extern std::shared_ptr<model_t> currentModel;
extern std::shared_ptr<model_node_t> currentNode;

// Camera
extern float cameraDistance;
extern float cameraAngleX;
extern float cameraAngleY;
extern glm::mat4 modelRotation;


// Lighting variables
extern bool lightingEnabled;
extern glm::vec3 lightPosition;
extern glm::vec3 lightColor;
extern float ambientStrength;
extern float diffuseStrength;
extern float specularStrength;
extern float shininess;

#endif // GLOBALS_H
