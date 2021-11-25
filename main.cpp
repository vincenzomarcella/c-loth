#include <cstdio>

#include "physics.h"

const int TARGET_FPS = 60;
const double SECONDSPERFRAME = 1.0 / TARGET_FPS;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int N_PHYSICS_UPDATE = 3;
const int N_CONSTRAIN_SOLVE = 10;

const int ROWS = 30; // Number of cloth rows
const int COLS = 40; // Number of points for each cloth row
// Simulation space constrains
const int XMAX = 500; 
const int YMAX = 500;
const int ZMAX = 500;

static Mouse mouse;
static Camera camera;
float Camera::fovy = 45.0f;
bool Camera::is_cursor_in_window = false;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    mouse.set_to_window_size(width, height);
    camera.set_to_window_size(width, height);
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

    // Fixing top row
    for (j = 0; j < COLS; j++)
        points[j]->fix_position();
    // Fixing bottom row
    // for (j = 0; j < COLS; j++)
    //     points[COLS * (ROWS - 1) + j]->fix_position();

    int n_points = sizeof(points) / sizeof(PointMass*);
    
    // Array that containts the texture vertices data
    float tex_vertices[8 * n_points + 3]{}; // +3 to store data for crosshair

    for (i = 0; i < ROWS; i++){
        for(j = 0; j < COLS; j++){
            int start_index = 8 * to1d_index(i, j, COLS);
            tex_vertices[start_index + 3] = 1.0f;
            tex_vertices[start_index + 4] = 1.0f;
            tex_vertices[start_index + 5] = 1.0f;
            tex_vertices[start_index + 6] = map(points[i * COLS + j]->get_pos_x(),
                                                points[0]->get_pos_x(), points[COLS - 1]->get_pos_x(),
                                                0, 1);
            tex_vertices[start_index + 7] = map(points[i * COLS + j]->get_pos_y(),
                                                points[0]->get_pos_y(), points[COLS * ROWS - 1]->get_pos_y(),
                                                0, 1);
        }

    }

    // 3 indices (each referring to a (x, y, z) vertex in vertices[])
    // for each of the 2 triangles needed to draw a rectangle using 4 points
    unsigned int indices[3 * 2 * (COLS - 1) * (ROWS - 1)]{}; 
    
    for (i = 0; i < ROWS - 1; i++)
        for (j = 0; j < COLS - 1; j++) {

            /*
            Cloth will be rendered using triangles following this pattern,
            points are stored in a flattened version of this grid matrix (points[])
            
            +-+-+-+-+       p0 +----+ p1
            |/|/|/|/|          |  / |
            +-+-+-+-+          | /  |
            |/|/|/|/|          |/   |
            +-+-+-+-+       p2 +----+ p3
            |/|/|/|/|
            +-+-+-+-+

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

    // Load the vertex indices inside of the element buffer object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // Wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    unsigned int texture = setTexture("flag.jpg");
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);

    // Activating shader program
    glUseProgram(shaderProgram);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    camera.load_matrices(shaderProgram);

    int frame = 0;
    double current_time, elapsed, last_time = 0;

    mouse.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);
    // Capturing mouse inside window and hiding cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, Camera::zoom);
    glfwSetCursorEnterCallback(window, Camera::activate_cursor_interaction);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        frame = frame % 1500 + 1;
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time;

        // printf("%f fps %f ms \r", 1 / elapsed, elapsed);

        processInput(window);

        // Handling mouse
        mouse.update(window, 0, XMAX, 0, YMAX);
        camera.update(window, shaderProgram, elapsed, mouse.get_pos());

        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        
        for (i = 0; i < N_PHYSICS_UPDATE; i++)
            timestep(
                points,
                COLS,
                ROWS,
                n_points,
                N_CONSTRAIN_SOLVE,
                SECONDSPERFRAME / N_PHYSICS_UPDATE,
                &mouse,
                &camera
            );

        // Updating crosshair position
        tex_vertices[8 * n_points] = camera.get_pos().x + camera.get_direction().x;
        tex_vertices[8 * n_points + 1] = camera.get_pos().y + camera.get_direction().y;
        tex_vertices[8 * n_points + 2] = camera.get_pos().z + camera.get_direction().z;

        // printf("%f %f %f\n", camera.get_pos().x, camera.get_pos().y, camera.get_pos().z);

        // Mapping PointMass positions
        for (j = 0; j < n_points; j++) {
            double x = points[j]->get_pos_x();
            double y = points[j]->get_pos_y();
            double z = points[j]->get_pos_z();

            tex_vertices[j * 8    ] = map(x, -XMAX, XMAX, -1, 1);
            tex_vertices[j * 8 + 1] = map(y, -YMAX, YMAX, -1, 1);
            tex_vertices[j * 8 + 2] = map(z, -ZMAX, ZMAX, -1, 1);
        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(tex_vertices), tex_vertices, GL_DYNAMIC_DRAW);

        drawFrame(window, n_points, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
    }

    collectGarbage(VAO, VBO, shaderProgram);
}