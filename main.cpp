#include <cstdio>

#include "physics.h"

const int TARGET_FPS = 60;
const double SECONDSPERFRAME = 1.0 / TARGET_FPS;

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

const int N_PHYSICS_UPDATE = 3;
const int N_CONSTRAIN_SOLVE = 10;

const int ROWS = 30; // Number of cloth rows
const int COLS = 50; // Number of points for each cloth row
const int WIDTH = 600; // To define a [-WIDTH/2, WIDTH/2] constrained x axis for the simulation
const int HEIGHT = 600; // To define a [-HEIGHT/2, HEIGHT/2] constrained y axis for the simulation
const int XMIN = -WIDTH / 2; 
const int XMAX = WIDTH / 2;
const int YMIN = -HEIGHT / 2; 
const int YMAX = HEIGHT / 2;

static Mouse mouse;

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
                j * 10.0 - 250,
                i * -10.0 + 250,
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
    points[0]->fix_position();
    points[COLS - 1]->fix_position();
    points[COLS * (ROWS - 1)]->fix_position();
    points[COLS * ROWS - 1]->fix_position();

    int n_points = sizeof(points) / sizeof(PointMass*);
    
    // Array that containts the texture vertices data
    float tex_vertices[7 * n_points]{};
    for (i = 0; i < ROWS; i++){
        for(j = 0; j < COLS; j++){
            int start_index = 7 * to1d_index(i, j, COLS);
            // tex_vertices[start_index    ] = map(points[i * COLS + j]->get_pos_x(), -300, 300, -1, 1);
            // tex_vertices[start_index + 1] = map(points[i * COLS + j]->get_pos_y(), -300, 300, -1, 1);
            tex_vertices[start_index + 2] = 1.0f;
            tex_vertices[start_index + 3] = 1.0f;
            tex_vertices[start_index + 4] = 1.0f;
            tex_vertices[start_index + 5] = map(points[i * COLS + j]->get_pos_x(), XMIN, XMAX, 0, 1);
            tex_vertices[start_index + 6] = map(points[i * COLS + j]->get_pos_y(), YMIN, YMAX, 0, 1);
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
            indices[start_index    ] = to1d_index(i   , j   , COLS);
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

    // setVertexDataInterpretation();

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    unsigned int texture = setTexture("flag.jpg");
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);

    int frame = 0;
    double current_time, elapsed, last_time = 0;

    mouse.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        frame = frame % 500 + 1;
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time; 

        printf("%f fps %f ms \r", 1 / elapsed, elapsed);

        processInput(window);

        // Handling mouse
        mouse.update(window, XMIN, XMAX, YMIN, YMAX);
        
        for (i = 0; i < N_PHYSICS_UPDATE; i++)
            timestep(
                points,
                n_points,
                N_CONSTRAIN_SOLVE,
                SECONDSPERFRAME / N_PHYSICS_UPDATE,
                &mouse
            );

        // Mapping PointMass positions
        for (j = 0; j < n_points; j++) {
            tex_vertices[j * 7    ] = map(points[j]->get_pos_x(), XMIN, XMAX, -1, 1);
            tex_vertices[j * 7 + 1] = map(points[j]->get_pos_y(), YMIN, YMAX, -1, 1);

        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(tex_vertices), tex_vertices, GL_DYNAMIC_DRAW);

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
    }

    collectGarbage(VAO, VBO, shaderProgram);
}