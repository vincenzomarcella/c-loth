#include <cstdio>
#include <chrono>
#include <thread>

#include "physics.h"
#include "graphics.h"

#include <GLFW/glfw3.h>

double map_in_range(double value, double from, double to, double mapFrom, double mapTo) {
    return  mapFrom + (mapTo - mapFrom) * (value - from) / (to - from);
}

// TODO:
//  - texture the cloth
//  - pin/unpin points
//  - switch from wireframe to fill mode
//  - lock framerate
//  - add tearability (segment color based on its length)
//  - drag points
//  - add the z axis
//  - collision with an object (circle or sphere)
//  - shade the cloth
//  - experiment with different types of links between points
//  - add wind (using perlin noise)
//  - glsl (difficile)

//  - hair sim (requires 3d, link points between grid of points)

const int TARGET_FPS = 60;
const double SECONDSPERFRAME = 1.0 / TARGET_FPS;

const int N_PHYSICS_UPDATE = 3;
const int N_CONSTRAIN_SOLVE = 10;

const int WIDTH = 50;
const int HEIGHT = 30;

int main() {
    PointMass* points[WIDTH * HEIGHT]{};

    int i, j;
    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++) {
            PointMass* pm = new PointMass{
                j * 10.0 - 250,
                i * -10.0 + 250,
                false,
                1 + 1 * (int)(i > 0 && j > 0) //+  (int)(i > 0 && j < WIDTH - 1)
            };
            if (i > 0)
                pm->add_neighbor(points[(i - 1) * WIDTH + j]);
            if (j > 0)
                pm->add_neighbor(points[i * WIDTH + j - 1]);
            // if (i > 0 && j > 0)
            //     pm->add_neighbor(points[(i - 1) * WIDTH + j - 1]);

            points[i * WIDTH + j] = pm;
        }   

    // for (i = 0; i < HEIGHT; i++)
    //     for (j = 0; j < WIDTH; j++)
    //         if (i > 0 && j < WIDTH - 1)
    //             points[i * WIDTH + j]->add_neighbor(points[(i - 1) * WIDTH + j + 1]);

    // Fixing once every 10 points on the top row
    // for (j = 0; j < WIDTH; j++)
    //     points[j]->fix_position();
    // points[WIDTH - 1]->fix_position();

    // Fixing corners
    points[0]->fix_position();
    points[WIDTH - 1]->fix_position();
    points[WIDTH * (HEIGHT - 1)]->fix_position();
    points[WIDTH * HEIGHT - 1]->fix_position();

    int n_points = sizeof(points) / sizeof(PointMass*);

    float vertices[3 * n_points]{};
    unsigned int indices[(WIDTH - 1) * (HEIGHT - 1) * 2 * 3]{};
    
    for (i = 0; i < HEIGHT - 1; i++)
        for (j = 0; j < WIDTH - 1; j++) {
            indices[6 * (i * (WIDTH - 1) + j)    ] = i * WIDTH + j;
            indices[6 * (i * (WIDTH - 1) + j) + 1] = i * WIDTH + j + 1;
            indices[6 * (i * (WIDTH - 1) + j) + 2] = (i + 1) * WIDTH + j;

            indices[6 * (i * (WIDTH - 1) + j) + 3] = i * WIDTH + j + 1;
            indices[6 * (i * (WIDTH - 1) + j) + 4] = (i + 1) * WIDTH + j;
            indices[6 * (i * (WIDTH - 1) + j) + 5] = (i + 1) * WIDTH + j + 1;
        }

    GLFWwindow* window = createWindow(800, 800);
    if (!window || !loadGlad())
        return -1;

    unsigned int shaderProgram = getShaderProgram();
    
    unsigned int VAO = getVAO();
    unsigned int VBO = getVBO();
    unsigned int EBO = getEBO();

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    setVertexDataInterpretation();
    // Wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int frame = 0;
    double current_time, elapsed, last_time = 0;
    double avg_fps = 0, avg_ms = 0;

    int width, height;
    double xpos, ypos;
    Vec2d mouse_pos, old_mouse_pos, mouse_vel;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        frame = frame % 500 + 1;
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time;

        avg_fps = ((avg_fps * (frame - 1)) + (1 / elapsed)) / frame;
        avg_ms = ((avg_ms * (frame - 1)) + elapsed) / frame; 
        processInput(window);
        printf("%02d fps %.2f ms \r", (int)avg_fps, avg_ms * 1000);

        glfwGetCursorPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &width, &height);
        xpos = map_in_range(xpos, 0, width, -300, 300);
        ypos = map_in_range(ypos, 0, height, 300, -300);

        mouse_pos = Vec2d{ xpos, ypos };
        mouse_vel = mouse_pos - old_mouse_pos; 
        old_mouse_pos = mouse_pos;

        for (i = 0; i < N_PHYSICS_UPDATE; i++)
            timestep(points, n_points, N_CONSTRAIN_SOLVE, SECONDSPERFRAME / N_PHYSICS_UPDATE, mouse_pos, mouse_vel);

        // Mapping PointMass positions
        for (j = 0; j < n_points; j++) {
            vertices[j * 3] = map_in_range(points[j]->get_pos_x(), -300, 300, -1, 1);
            vertices[j * 3 + 1] = map_in_range(points[j]->get_pos_y(), 300, -300, 1, -1);
        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
    }

    collectGarbage(VAO, VBO, shaderProgram);
}