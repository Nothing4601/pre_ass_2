#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

#include "shape.h"
#include "HIERARCHIAL.h"
#include "globals.h"

// Helper function to create and position a shape
std::shared_ptr<model_node_t> createShape(std::unique_ptr<shape_t> shape,
    glm::vec3 position,
    glm::vec3 scale,
    glm::vec4 color) {

    // Generate geometry FIRST before setting color
    if (shape) {
        shape->generateGeometry();
        shape->setColor(color);  // Now vertices exist, so this works correctly
    }

    currentModel->addShape(std::move(shape));
    auto node = currentModel->getLastNode();

    // Set position
    node->translation = glm::translate(glm::mat4(1.0f), position);

    // Set scale
    node->scale = glm::scale(glm::mat4(1.0f), scale);

    return node;
}

// Build a complete indoor scene
void buildIndoorScene() {
    std::cout << "Building indoor scene..." << std::endl;

    // Clear existing model
    currentModel->clear();
    currentNode = currentModel->getRoot();

    // Colors
    glm::vec4 wallColor(0.9f, 0.9f, 0.85f, 1.0f);      // Light beige
    glm::vec4 floorColor(0.6f, 0.4f, 0.2f, 1.0f);      // Wood brown
    glm::vec4 ceilingColor(0.95f, 0.95f, 0.95f, 1.0f); // Off-white
    glm::vec4 tableColor(0.4f, 0.25f, 0.1f, 1.0f);     // Dark wood
    glm::vec4 chairColor(0.5f, 0.3f, 0.15f, 1.0f);     // Medium wood
    glm::vec4 lampColor(0.9f, 0.9f, 0.7f, 1.0f);       // Light yellow
    glm::vec4 redColor(0.8f, 0.2f, 0.2f, 1.0f);        // Red
    glm::vec4 blueColor(0.2f, 0.4f, 0.8f, 1.0f);       // Blue
    glm::vec4 greenColor(0.2f, 0.7f, 0.3f, 1.0f);      // Green

    // === ROOM STRUCTURE ===

    // Floor (large flat box)
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, -1.50f, 0.0f),
        glm::vec3(10.0f, 0.1f, 12.0f),
        floorColor);

    // Ceiling
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, 4.0f, 1.0f),
        glm::vec3(10.0f, 0.1f, 12.0f),
        ceilingColor);

    // Back Wall
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, 0.0f, -6.0f),
        glm::vec3(10.0f, 4.0f, 0.1f),
        wallColor);

    // Left Wall
    createShape(std::make_unique<box_t>(1),
        glm::vec3(-10.0f, 1.0f, 0.0f),
        glm::vec3(0.1f, 3.0f, 12.0f),
        wallColor);

    // Right Wall
    createShape(std::make_unique<box_t>(1),
        glm::vec3(10.0f, 1.0f, 0.0f),
        glm::vec3(0.1f, 3.0f, 12.0f),
        wallColor);

    // === TABLE (in center) ===

    // Table top
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.5f, 0.1f, 1.0f),
        tableColor);

    // Table legs (4 cylinders)
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(-0.6f, -0.8f, -0.4f),
        glm::vec3(0.05f, 0.8f, 0.05f),
        tableColor);

    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.6f, -0.8f, -0.4f),
        glm::vec3(0.05f, 0.8f, 0.05f),
        tableColor);

    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(-0.6f, -0.8f, 0.4f),
        glm::vec3(0.05f, 0.8f, 0.05f),
        tableColor);

    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.6f, -0.8f, 0.4f),
        glm::vec3(0.05f, 0.8f, 0.05f),
        tableColor);

    // === CHAIRS (around table) ===

    // Chair 1 (front)
    // Seat
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, -0.5f, 1.2f),
        glm::vec3(0.4f, 0.05f, 0.4f),
        chairColor);
    // Backrest
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, -0.1f, 1.4f),
        glm::vec3(0.4f, 0.4f, 0.05f),
        chairColor);
    // Legs
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(-0.15f, -1.0f, 1.05f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.15f, -1.0f, 1.05f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(-0.15f, -1.0f, 1.35f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.15f, -1.0f, 1.35f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);

    // Chair 2 (back)
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, -0.5f, -1.2f),
        glm::vec3(0.4f, 0.05f, 0.4f),
        chairColor);
    createShape(std::make_unique<box_t>(1),
        glm::vec3(0.0f, -0.1f, -1.4f),
        glm::vec3(0.4f, 0.4f, 0.05f),
        chairColor);
    // Legs
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(-0.15f, -1.0f, 1.05f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.15f, -1.0f, 1.05f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(-0.15f, -1.0f, -1.45f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.15f, -1.0f, -1.45f),
        glm::vec3(0.03f, 0.5f, 0.03f),
        chairColor);

    // === LAMP (on table) ===

    // Lamp base (cylinder)
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.5f, 0.2f, 0.0f),
        glm::vec3(0.2f, 0.15f, 0.2f),
        tableColor);

    // Lamp stem
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.5f, 0.6f, 0.0f),
        glm::vec3(0.02f, 0.4f, 0.02f),
        tableColor);

    // Lamp shade (cone upside down)
    createShape(std::make_unique<cone_t>(2),
        glm::vec3(0.5f, 1.0f, 0.0f),
        glm::vec3(0.25f, 0.3f, 0.25f),
        lampColor);

    // === DECORATIVE ITEMS ===

    // Bowl on table (sphere)
    createShape(std::make_unique<sphere_t>(2),
        glm::vec3(-0.5f, 0.15f, 0.0f),
        glm::vec3(0.15f, 0.1f, 0.15f),
        redColor);

    // Ball decoration
    createShape(std::make_unique<sphere_t>(2),
        glm::vec3(-0.5f, 0.3f, 0.0f),
        glm::vec3(0.08f, 0.08f, 0.08f),
        blueColor);

    // === SHELF ON WALL ===

    // Shelf
    createShape(std::make_unique<box_t>(1),
        glm::vec3(-2.0f, 0.5f, -5.45f),
        glm::vec3(0.8f, 0.05f, 0.25f),
        tableColor);

    // Objects on shelf
    createShape(std::make_unique<sphere_t>(2),
        glm::vec3(-2.3f, 0.65f, -5.45f),
        glm::vec3(0.2f, 0.1f, 0.1f),
        redColor);

    createShape(std::make_unique<cylinder_t>(2),
        glm::vec3(-2.0f, 0.75f, -5.45f),
        glm::vec3(0.08f, 0.2f, 0.08f),
        greenColor);

    createShape(std::make_unique<cone_t>(2),
        glm::vec3(-1.7f, 0.65f, -5.45f),
        glm::vec3(0.08f, 0.15f, 0.08f),
        blueColor);

    // === CORNER PLANT ===

    // Pot (cylinder)
    createShape(std::make_unique<cylinder_t>(2),
        glm::vec3(-2.5f, -1.1f, -2.5f),
        glm::vec3(0.2f, 0.3f, 0.2f),
        redColor);

    // Plant stem
    createShape(std::make_unique<cylinder_t>(2),
        glm::vec3(-2.5f, -0.5f, -2.5f),
        glm::vec3(0.03f, 0.6f, 0.03f),
        greenColor);

    // Plant leaves (cones)
    createShape(std::make_unique<cone_t>(2),
        glm::vec3(-2.5f, -0.01f, -2.5f),
        glm::vec3(0.3f, 0.4f, 0.3f),
        greenColor);

    // === CEILING LIGHT ===

    // Hanging cord
    createShape(std::make_unique<cylinder_t>(1),
        glm::vec3(0.0f, 3.5f, 0.0f),
        glm::vec3(0.01f, 0.5f, 0.01f),
        glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

    // Light bulb (sphere)
    createShape(std::make_unique<sphere_t>(2),
        glm::vec3(0.0f, 3.0f, 0.0f),
        glm::vec3(0.15f, 0.2f, 0.15f),
        lampColor);

    std::cout << "Indoor scene created with " << currentModel->getShapeCount() << " objects!" << std::endl;
    std::cout << "Press 'I' for inspection mode to view the scene" << std::endl;
    std::cout << "Use arrow keys to rotate and +/- to zoom" << std::endl;

    // Set current node to root
    currentNode = currentModel->getRoot();
}