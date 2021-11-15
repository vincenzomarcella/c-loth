#include <cstdio>

#include "physics.h"

const int TARGET_FPS = 60;
const double SECONDSPERFRAME = 1.0 / TARGET_FPS;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

const int N_PHYSICS_UPDATE = 3;
const int N_CONSTRAIN_SOLVE = 10;

const int ROWS = 30; // Number of cloth rows
const int COLS = 50; // Number of points for each cloth row
// Simulation space constrains
const int XMAX = 1000; 
const int YMAX = 1000;
const int ZMAX = 1000;

static Mouse mouse;
static Camera camera{ 0.0, 0.0, 1000.0 };

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    mouse.set_to_window_size(width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

Vec3d rotate_x(double x, double y, double z, double angle) {
    return Vec3d{ x, cos(angle) * y - sin(angle) * z, sin(angle) * y + cos(angle) * z };
}

Vec3d rotate_y(double x, double y, double z, double angle) {
    return Vec3d{ cos(angle) * x + sin(angle) * z, y, -sin(angle) * x + cos(angle) * z };
}

Vec3d rotate_z(double x, double y, double z, double angle) {
    return Vec3d{ cos(angle) * x - sin(angle) * y, sin(angle) * x + cos(angle) * y, z };
}

int main() {
    PointMass* points[COLS * ROWS]{};

    int i, j;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++) {
            PointMass* pm = new PointMass{
                j * 10.0 - 100,
                i * -10.0 + 400,
                200,
                false,
                1 + 1 * (int)(i > 0 && j > 0)
            };
            if (i > 0)
                // Linking to above point
                pm->add_neighbor(points[to1d_index(i - 1, j, COLS)]);
            if (j > 0)
                // Linking to left point
                pm->add_neighbor(points[to1d_index(i, j - 1, COLS)]);
            
            points[i * COLS + j] = pm;
        }   

    // Fixing corners
    // points[0]->fix_position();
    // points[COLS - 1]->fix_position();
    // points[COLS * (ROWS - 1)]->fix_position();
    // points[COLS * ROWS - 1]->fix_position();

    // Fixing yop row
    for (j = 0; j < COLS; j++)
        points[j]->fix_position();

    int n_points = sizeof(points) / sizeof(PointMass*);

    float vertices[3 * n_points]{}; // (x, y, z) vertex data for each point

    // 3 indices (each referring to a (x, y, z) vertex in vertices[])
    // for each of the 2 triangles needed to draw a rectangle using 4 points
    unsigned int indices[3 * 2 * (COLS - 1) * (ROWS - 1)]{}; 
    
    for (i = 0; i < ROWS - 1; i++)
        for (j = 0; j < COLS - 1; j++) {

            /*
            Cloth will be rendered using triangles following this pattern,
            points are stored in a flattened version of this grid matrix (points[])
            
            +-+-+-+-+
            |/|/|/|/|
            +-+-+-+-+
            |/|/|/|/|
            +-+-+-+-+
            |/|/|/|/|
            +-+-+-+-+

            Each rectangle 

            p0 +----+ p1
               |  / |
               | /  |
               |/   |
            p2 +----+ p3

            */

            int start_index = 6 * to1d_index(i, j, COLS - 1);
            // Triangle (p0, p1, p2)
            indices[start_index    ] = to1d_index(i    , j    , COLS);
            indices[start_index + 1] = to1d_index(i    , j + 1, COLS);
            indices[start_index + 2] = to1d_index(i + 1, j    , COLS);
            // Triangle (p1, p2, p3)
            indices[start_index + 3] = to1d_index(i    , j + 1, COLS);
            indices[start_index + 4] = to1d_index(i + 1, j    , COLS);
            indices[start_index + 5] = to1d_index(i + 1, j + 1, COLS);
        }

    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!window || !loadGlad())
        return -1;

    // Binding a callback to allow resizing
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);

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

    mouse.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        frame = frame % 500 + 1;
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time; 

        avg_fps = ((avg_fps * (frame - 1)) + (1 / elapsed)) / frame;
        avg_ms = ((avg_ms * (frame - 1)) + elapsed) / frame; 

        processInput(window);

        // Handling mouse
        mouse.update(window, 0, XMAX, 0, YMAX);
        camera.update(window);
        
        for (i = 0; i < N_PHYSICS_UPDATE; i++)
            timestep(
                points,
                n_points,
                N_CONSTRAIN_SOLVE,
                SECONDSPERFRAME / N_PHYSICS_UPDATE,
                &mouse
            );

        double angle = map(frame, 0, 500, 0, 2 * M_PI);

        // Mapping PointMass positions
        for (j = 0; j < n_points; j++) {
            double x = points[j]->get_pos_x();
            double y = points[j]->get_pos_y();
            double z = points[j]->get_pos_z();

            // Vec3d rotated = rotate_y(x, y, z, angle);
            Vec3d rotated = Vec3d{x, y, z};
            Vec3d projected = camera.perspective_projection(rotated);

            vertices[j * 3    ] = projected.get_x();
            vertices[j * 3 + 1] = projected.get_y();
            vertices[j * 3 + 2] = projected.get_z();

            // projected.print();

        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
    }

    collectGarbage(VAO, VBO, shaderProgram);
}