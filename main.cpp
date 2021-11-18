#include <cstdio>

#include "physics.h"

const int TARGET_FPS = 60;
const double SECONDSPERFRAME = 1.0 / TARGET_FPS;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const int N_PHYSICS_UPDATE = 3;
const int N_CONSTRAIN_SOLVE = 10;

const int ROWS = 40; // Number of cloth rows
const int COLS = 40; // Number of points for each cloth row
// Simulation space constrains
const int XMAX = 500; 
const int YMAX = 500;
const int ZMAX = 500;

static Mouse mouse;
static Camera camera{ 100.0, 100.0, 500.0 };

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    mouse.set_to_window_size(width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    PointMass* points[COLS * ROWS]{};

    int i, j;
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++) {
            PointMass* pm = new PointMass{
                j * 8.0 - 160,
                i * -8.0 + 160,
                0,
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

    mouse.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -0.8f));
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(100.0f),
        (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);

    // Activating shader program
    glUseProgram(shaderProgram);

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    modelLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        frame = frame % 1500 + 1;
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time; 

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

        view = glm::rotate(view, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Mapping PointMass positions
        for (j = 0; j < n_points; j++) {
            double x = points[j]->get_pos_x();
            double y = points[j]->get_pos_y();
            double z = points[j]->get_pos_z();

            vertices[j * 3    ] = map(x, -XMAX, XMAX, -1, 1);
            vertices[j * 3 + 1] = map(y, -YMAX, YMAX, -1, 1);
            vertices[j * 3 + 2] = map(z, -ZMAX, ZMAX, -1, 1);
        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
    }

    collectGarbage(VAO, VBO, shaderProgram);
}