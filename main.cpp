#include <cstdio>
#include <chrono>
#include <thread>

#include "physics.h"
#include "graphics.h"

#include <GLFW/glfw3.h>

double map_in_range(double value, double from, double to, double mapFrom, double mapTo) {
    return  mapFrom + (mapTo - mapFrom) * (value - from) / (to - from);
}

const int framesPerSecond = 120;
const double secondsPerFrame = 1.0 / framesPerSecond;

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
    // for (j = 0; j < WIDTH / 10; j++)
    //     points[j * 10]->fix_position();
    // points[WIDTH - 1]->fix_position();

    // Fixing corners
    points[0]->fix_position();
    points[WIDTH - 1]->fix_position();
    // points[WIDTH * (HEIGHT - 1)]->fix_position();
    // points[WIDTH * HEIGHT - 1]->fix_position();

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

    double current_time, last_time = 0, elapsed;

    int n_physics_update = 5;
    int n_constrain_solve = 10;

    int width, height;
    double xpos, ypos;
    Vec2d mouse_pos, old_mouse_pos, mouse_vel;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time;

        // printf("%f ms\n", elapsed * 1000);

        if (elapsed < secondsPerFrame) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds((int)((secondsPerFrame - elapsed) * 1000)));
        }

        processInput(window);

        glfwGetCursorPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &width, &height);
        xpos = map_in_range(xpos, 0, width, -300, 300);
        ypos = map_in_range(ypos, 0, height, 300, -300);

        mouse_pos = Vec2d{ xpos, ypos };
        mouse_vel = mouse_pos - old_mouse_pos; 
        old_mouse_pos = mouse_pos;
        // printf("%f %f\n", mouse_vel.get_x(), mouse_vel.get_y());

        for (i = 0; i < n_physics_update; i++)
            timestep(points, n_points, n_constrain_solve, secondsPerFrame / n_physics_update, mouse_pos, mouse_vel);

        // Mapping PointMass positions
        for (j = 0; j < n_points; j++) {
            vertices[j * 3] = map_in_range(points[j]->get_pos_x(), -300, 300, -1, 1);
            vertices[j * 3 + 1] = map_in_range(points[j]->get_pos_y(), 300, -300, 1, -1);
        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);

        // printf("P1(%f %f) P2(%f %f) P3(%f %f) P4(%f %f) P5(%f %f) P6(%f %f)\n",
        //     points[0]->get_pos_x(), points[0]->get_pos_y(),
        //     points[1]->get_pos_x(), points[1]->get_pos_y(),
        //     points[2]->get_pos_x(), points[2]->get_pos_y(),
        //     points[3]->get_pos_x(), points[3]->get_pos_y()
        // );
    }

    collectGarbage(VAO, VBO, shaderProgram);
}