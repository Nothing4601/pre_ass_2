#ifndef SHAPE_H
#define SHAPE_H
#include <iostream>
#include <vector>
#include <memory>
#include <GL/glew.h>   
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shape Types
enum ShapeType {
    SPHERE_SHAPE,
    CONE_SHAPE,
    BOX_SHAPE,
    CYLINDER_SHAPE
};

// Base Class
class shape_t {
public:
    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec3> normals;  // Added for lighting
    std::vector<unsigned int> indices;

    GLuint VAO = 0, VBO = 0, CBO = 0, NBO = 0, EBO = 0;  // Added NBO for normals
    ShapeType shapetype;
    unsigned int level;

    shape_t() : level(1) {}
    shape_t(unsigned int tesselation_level) : level(tesselation_level) {
        if (level < 1) level = 1;
        if (level > 4) level = 4;
    }

    virtual ~shape_t() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (CBO) glDeleteBuffers(1, &CBO);
        if (NBO) glDeleteBuffers(1, &NBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }

    ShapeType getType() const { return shapetype; }

    virtual void generateGeometry() = 0;

    unsigned int getLevel() const { return level; }

    void setLevel(unsigned int l) {
        if (l < 1) l = 1;
        if (l > 4) l = 4;
        if (level != l) {
            level = l;
            generateGeometry();
            VAO = VBO = CBO = NBO = EBO = 0; // force GPU buffer update
        }
    }

    virtual void setColor(const glm::vec4& c) {
        if (vertices.empty()) {
            colors.assign(1, c);
        }
        else {
            colors.assign(vertices.size(), c);
        }

        // Update GPU buffer if already created
        if (CBO != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, CBO);
            // Re-allocate buffer with correct size
            glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    void setupBuffers() {
        if (VAO != 0) return;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Vertex positions
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(glm::vec4),
            vertices.data(),
            GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        // Ensure colors exist
        if (colors.empty()) {
            colors.assign(vertices.size(), glm::vec4(1.0f));
        }

        // Vertex colors
        glGenBuffers(1, &CBO);
        glBindBuffer(GL_ARRAY_BUFFER, CBO);
        glBufferData(GL_ARRAY_BUFFER,
            colors.size() * sizeof(glm::vec4),
            colors.data(),
            GL_STATIC_DRAW);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);

        // Normals
        if (normals.empty()) {
            normals.assign(vertices.size(), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        glGenBuffers(1, &NBO);
        glBindBuffer(GL_ARRAY_BUFFER, NBO);
        glBufferData(GL_ARRAY_BUFFER,
            normals.size() * sizeof(glm::vec3),
            normals.data(),
            GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);

        // Indices
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int),
            indices.data(),
            GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    virtual void draw(const glm::mat4& MVP, GLuint shaderProgram) {
        if (VAO == 0) {
            generateGeometry();
            setupBuffers();
        }

        // Upload MVP
        GLint mvpLoc = glGetUniformLocation(shaderProgram, "MVP");
        if (mvpLoc != -1) {
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void changeTesselation(int delta) {
        int newLevel = static_cast<int>(level) + delta;
        if (newLevel < 1) newLevel = 1;
        if (newLevel > 4) newLevel = 4;
        setLevel(static_cast<unsigned int>(newLevel));
    }
};

// Sphere
class sphere_t : public shape_t {
public:
    sphere_t(unsigned int tesselation_level = 1) : shape_t(tesselation_level) {
        shapetype = SPHERE_SHAPE;
    }

    void generateGeometry() override {
        vertices.clear();
        colors.clear();
        normals.clear();
        indices.clear();

        unsigned int stacks = 10 * level;
        unsigned int slices = 10 * level;

        for (unsigned int i = 0; i <= stacks; ++i) {
            float phi = glm::pi<float>() * i / stacks;
            for (unsigned int j = 0; j <= slices; ++j) {
                float theta = 2.0f * glm::pi<float>() * j / slices;
                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);

                vertices.emplace_back(x, y, z, 1.0f);
                colors.emplace_back(1, 1, 1, 1);
                // Normal for sphere is just the normalized position
                normals.emplace_back(x, y, z);
            }
        }

        for (unsigned int i = 0; i < stacks; ++i) {
            for (unsigned int j = 0; j < slices; ++j) {
                unsigned int first = i * (slices + 1) + j;
                unsigned int second = first + slices + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }
    }
};

// Cone
class cone_t : public shape_t {
public:
    cone_t(unsigned int tesselation_level = 2) : shape_t(tesselation_level) {
        shapetype = CONE_SHAPE;
    }

    void generateGeometry() override {
        vertices.clear();
        colors.clear();
        normals.clear();
        indices.clear();

        unsigned int slices = 20 * level;

        // Apex
        vertices.emplace_back(0, 1, 0, 1);
        colors.emplace_back(1, 1, 1, 1);
        normals.emplace_back(0, 1, 0);

        // Base center
        vertices.emplace_back(0, -1, 0, 1);
        colors.emplace_back(1, 1, 1, 1);
        normals.emplace_back(0, -1, 0);

        // Base circle vertices
        for (unsigned int i = 0; i <= slices; ++i) {
            float theta = 2.0f * glm::pi<float>() * i / slices;
            float x = cos(theta);
            float z = sin(theta);
            vertices.emplace_back(x, -1, z, 1);
            colors.emplace_back(1, 1, 1, 1);

            // Calculate normal for cone side
            glm::vec3 sideNormal = glm::normalize(glm::vec3(x, 0.5f, z));
            normals.emplace_back(sideNormal);
        }

        // Side triangles
        for (unsigned int i = 0; i < slices; ++i) {
            unsigned int apex = 0;
            unsigned int v1 = 2 + i;
            unsigned int v2 = 2 + (i + 1);

            indices.push_back(apex);
            indices.push_back(v1);
            indices.push_back(v2);
        }

        // Base triangles
        for (unsigned int i = 0; i < slices; ++i) {
            unsigned int center = 1;
            unsigned int v1 = 2 + i;
            unsigned int v2 = 2 + (i + 1);

            indices.push_back(center);
            indices.push_back(v2);
            indices.push_back(v1);
        }
    }
};

// Box
class box_t : public shape_t {
public:
    box_t(unsigned int tesselation_level = 1) : shape_t(tesselation_level) {
        shapetype = BOX_SHAPE;
    }

    void generateGeometry() override {
        vertices.clear();
        colors.clear();
        normals.clear();
        indices.clear();

        unsigned int n = level;
        if (n < 1) n = 1;

        auto addFace = [&](glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec4 v3,
            glm::vec4 color, glm::vec3 normal) {
                unsigned int startIndex = vertices.size();

                for (unsigned int i = 0; i <= n; ++i) {
                    for (unsigned int j = 0; j <= n; ++j) {
                        float u = float(i) / n;
                        float v = float(j) / n;

                        glm::vec4 pos = (1 - u) * (1 - v) * v0 + u * (1 - v) * v1 +
                            u * v * v2 + (1 - u) * v * v3;
                        vertices.push_back(pos);
                        colors.push_back(color);
                        normals.push_back(normal);
                    }
                }

                for (unsigned int i = 0; i < n; ++i) {
                    for (unsigned int j = 0; j < n; ++j) {
                        unsigned int row1 = i * (n + 1) + j + startIndex;
                        unsigned int row2 = (i + 1) * (n + 1) + j + startIndex;

                        indices.push_back(row1);
                        indices.push_back(row2);
                        indices.push_back(row1 + 1);

                        indices.push_back(row2);
                        indices.push_back(row2 + 1);
                        indices.push_back(row1 + 1);
                    }
                }
            };

        glm::vec4 v[] = {
            {-1,-1,-1,1}, {1,-1,-1,1}, {1,1,-1,1}, {-1,1,-1,1},
            {-1,-1, 1,1}, {1,-1, 1,1}, {1,1, 1,1}, {-1,1, 1,1}
        };

        glm::vec4 whiteColor = { 1.0f, 1.0f, 1.0f, 1.0f };

        addFace(v[0], v[1], v[2], v[3], whiteColor, glm::vec3(0, 0, -1));  // back
        addFace(v[5], v[4], v[7], v[6], whiteColor, glm::vec3(0, 0, 1));   // front
        addFace(v[4], v[0], v[3], v[7], whiteColor, glm::vec3(-1, 0, 0));  // left
        addFace(v[1], v[5], v[6], v[2], whiteColor, glm::vec3(1, 0, 0));   // right
        addFace(v[3], v[2], v[6], v[7], whiteColor, glm::vec3(0, 1, 0));   // top
        addFace(v[4], v[5], v[1], v[0], whiteColor, glm::vec3(0, -1, 0));  // bottom
    }
};

// Cylinder
class cylinder_t : public shape_t {
public:
    cylinder_t(unsigned int tesselation_level = 2) : shape_t(tesselation_level) {
        shapetype = CYLINDER_SHAPE;
    }

    void generateGeometry() override {
        vertices.clear();
        colors.clear();
        normals.clear();
        indices.clear();

        unsigned int slices = 20 * level;

        // Generate cylindrical surface vertices
        for (unsigned int i = 0; i <= slices; ++i) {
            float theta = 2.0f * glm::pi<float>() * i / slices;
            float x = cos(theta);
            float z = sin(theta);

            // Top vertex
            vertices.emplace_back(x, 1, z, 1);
            colors.emplace_back(1, 1, 1, 1);
            normals.emplace_back(x, 0, z);  // Normal points radially outward

            // Bottom vertex
            vertices.emplace_back(x, -1, z, 1);
            colors.emplace_back(1, 1, 1, 1);
            normals.emplace_back(x, 0, z);
        }

        // Add center vertices for caps
        unsigned int topCenterIndex = vertices.size();
        vertices.emplace_back(0, 1, 0, 1);
        colors.emplace_back(1, 1, 1, 1);
        normals.emplace_back(0, 1, 0);

        unsigned int bottomCenterIndex = vertices.size();
        vertices.emplace_back(0, -1, 0, 1);
        colors.emplace_back(1, 1, 1, 1);
        normals.emplace_back(0, -1, 0);

        // Cylindrical surface indices
        for (unsigned int i = 0; i < slices; ++i) {
            unsigned int curr = i * 2;
            unsigned int next = ((i + 1) % slices) * 2;

            indices.push_back(curr);
            indices.push_back(curr + 1);
            indices.push_back(next);

            indices.push_back(curr + 1);
            indices.push_back(next + 1);
            indices.push_back(next);
        }

        // Top cap triangles
        for (unsigned int i = 0; i < slices; ++i) {
            unsigned int curr = i * 2;
            unsigned int next = ((i + 1) % slices) * 2;

            indices.push_back(topCenterIndex);
            indices.push_back(curr);
            indices.push_back(next);
        }

        // Bottom cap triangles
        for (unsigned int i = 0; i < slices; ++i) {
            unsigned int curr = i * 2 + 1;
            unsigned int next = ((i + 1) % slices) * 2 + 1;

            indices.push_back(bottomCenterIndex);
            indices.push_back(next);
            indices.push_back(curr);
        }
    }
};

#endif // SHAPE_H