#include <cstdio>

#include "physics.h"
#include "graphics.h"

#include <GLFW/glfw3.h>

double get_deltat() {
    return 1.0 / 300;
}

int main() {
    GLFWwindow* window = createWindow(400, 300);
    if (!window || !loadGlad())
        return -1;

    unsigned int shaderProgram = getShaderProgram();

    PointMass pm1{-0.5, 0.5, true};
    PointMass pm2{0.5, 0.5, true};
    PointMass pm3{0.5, -0.4, false, 2};
    PointMass pm4{-0.4, -0.5, false};

    pm3.add_neighbor(&pm2);
    pm3.add_neighbor(&pm4);
    pm2.add_neighbor(&pm1);
    pm4.add_neighbor(&pm1);

    PointMass* points[]{&pm1, &pm2, &pm3, &pm4};

    // Defining rectangle vertices
    float vertices[] = {
        pm1.get_pos_x(), pm1.get_pos_y(), 0.0f, // x, y, z
        pm2.get_pos_x(), pm2.get_pos_y(), 0.0f,
        pm3.get_pos_x(), pm3.get_pos_y(), 0.0f,
        pm4.get_pos_x(), pm4.get_pos_y(), 0.0f,
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 3, 2,
    };
    
    unsigned int VAO = getVAO();
    unsigned int VBO = getVBO();
    unsigned int EBO = getEBO();

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    setVertexDataInterpretation();
    
    int j, n_points = sizeof(points) / sizeof(PointMass*);
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        timestep(points, n_points, j, get_deltat());

        // Mapping PointMass positions
        for (j = 0; j < n_points; j++) {
            vertices[j * 3] = points[j]->get_pos_x();
            vertices[j * 3 + 1] = points[j]->get_pos_y();
        }
        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        drawFrame(window, shaderProgram, VAO);

        printf("P1(%f %f) P2(%f %f) P3(%f %f) P4(%f %f)\n",
            points[0]->get_pos_x(), points[0]->get_pos_y(),
            points[1]->get_pos_x(), points[1]->get_pos_y(),
            points[2]->get_pos_x(), points[2]->get_pos_y(),
            points[3]->get_pos_x(), points[3]->get_pos_y()

        );
    }

    collectGarbage(VAO, VBO, shaderProgram);
}