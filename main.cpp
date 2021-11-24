#include <cstdio>

#include "physics.h"

const int TARGET_FPS = 60;
const double SECONDSPERFRAME = 1.0 / TARGET_FPS;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

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

bool cursorEnabled = false;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    mouse.set_to_window_size(width, height);
    camera.set_to_window_size(width, height);
}

void switchCursorMode(GLFWwindow* window) {
    cursorEnabled = !cursorEnabled;
    if(cursorEnabled == true)  {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}


void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
        switchCursorMode(window);
}


// Unpin all points except corners
void unpinAll(PointMass* points[]) {
    // Unfixing all points
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++) {
            PointMass* point = points[i * COLS + j];
            point->unfix_position();
        }   

    // Refixing corners
    points[0]->fix_position();
    points[COLS - 1]->fix_position();
    points[COLS * (ROWS - 1)]->fix_position();
    points[COLS * ROWS - 1]->fix_position();
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
    float tex_vertices[8 * n_points]{};
    for (i = 0; i < ROWS; i++){
        for(j = 0; j < COLS; j++){
            int start_index = 8 * to1d_index(i, j, COLS);
            tex_vertices[start_index + 3] = 1.0f;
            tex_vertices[start_index + 4] = 1.0f;
            tex_vertices[start_index + 5] = 1.0f;
            tex_vertices[start_index + 6] = map(points[i * COLS + j]->get_pos_x(), points[0]->get_pos_x(), points[COLS - 1]->get_pos_x(), 0, 1);
            tex_vertices[start_index + 7] = map(points[i * COLS + j]->get_pos_y(), points[0]->get_pos_y(), points[COLS * ROWS - 1]->get_pos_y(), 0, 1);
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
    unsigned int texture = setTexture("jeans.jpeg");
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);

    // Activating shader program
    glUseProgram(shaderProgram);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    camera.load_matrices(shaderProgram);

    int frame = 0;
    double current_time, elapsed, last_time = 0;

    mouse.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Setting the inputmode so that a key state cannot change more than once per buffer swap
    //glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    // Capturing mouse inside window and hiding cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, Camera::zoom);

    glfwSetKeyCallback(window, key_callback);

    // Initialize the state for the GUI
    ImGuiState* GUIState = new ImGuiState();


    float gravity = -10.0f;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        frame = frame % 1500 + 1;
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time;

        printf("%f fps %f ms \r", 1 / elapsed, elapsed);

        //processInput(window);

        // Handling mouse
        //mouse.update(window, 0, XMAX, 0, YMAX);
        //camera.update(window, shaderProgram, mouse.get_pos());

        mouse.update(window, 0, XMAX, 0, YMAX);

        if(!cursorEnabled) {
            glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
            // Handling mouse
            camera.update(window, shaderProgram, mouse.get_pos());
        }
        
        for (i = 0; i < N_PHYSICS_UPDATE; i++)
            timestep(
                points,
                COLS,
                ROWS,
                n_points,
                N_CONSTRAIN_SOLVE,
                SECONDSPERFRAME / N_PHYSICS_UPDATE,
                &mouse
            );

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


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        if (GUIState->show_helper_window) {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("C-LOTH HELPER");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Graphical parameters.");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &GUIState->show_helper_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Wireframe", &GUIState->wireframe_enabled);
            //ImGui::Checkbox("Another Window", &GUIState->show_another_window);

            ImGui::SliderFloat("Gravity", &gravity, -20.0f, 20.0f);
            GRAVITY = Vec3d{0.0f, gravity, 0.0f};

            ImGui::SliderFloat("Wind Strength", &WIND_STRENGTH_MULTIPLIER, 0.0f, 2.0f);
            
            ImGui::SliderInt("Mouse Sensitivity", &camera.MOUSE_SENS, 1000, 20000);

            if (ImGui::Button("Unpin all")) // Buttons return true when clicked (most widgets return true when edited/activated)
                unpinAll(points);
            /*ImGui::SameLine();
            ImGui::Text("counter = %d", counter);*/

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        glfwMakeContextCurrent(window);
        if(GUIState->wireframe_enabled) {
            printf("Wireframe mode enabled");
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            printf("Wireframe mode disabled");
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
    }

    collectGarbage(VAO, VBO, shaderProgram);
}