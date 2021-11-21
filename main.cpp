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

    const int clothsize = ROWS * COLS;
    
    // Array that containts the texture vertices data
    float texVertices[7 * clothsize]{};
    for (i = 0; i < ROWS; i++){
        for(j = 0; j < COLS; j++){
            int start_index = 7 * to1d_index(i, j, COLS);
            texVertices[start_index] = map(points[i * COLS + j]->get_pos_x(), -300, 300, -1, 1);
            texVertices[start_index + 1] = map(points[i * COLS + j]->get_pos_y(), -300, 300, -1, 1);
            texVertices[start_index + 2] = 1.0f;
            texVertices[start_index + 3] = 1.0f;
            texVertices[start_index + 4] = 1.0f;
            texVertices[start_index + 5] = map(points[i * COLS + j]->get_pos_x(), -300, 300, 0, 1);
            texVertices[start_index + 6] = map(points[i * COLS + j]->get_pos_y(), 300, -300, 0, 1);
        }

    }

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

    // Load the vertex data inside the vertex buffer object
    // Load the vertex indices inside of the element buffer object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    setVertexDataInterpretation();
    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Configuring the way that textures are repeated even though it should not happen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // Configuring linear texture mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Configuring bilinear texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Configuring the vertex array striding
    // The first two values are the vertex location
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                       7*sizeof(float), 0);
    // The next three attributes are the color
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                       7*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(2);  
    // The last two attributes are the texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
    // Loading the texture
    int texture_width, texture_height, nrChannels;
    unsigned char *data = stbi_load("flag.jpg", &texture_width, &texture_height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
    }
    //stbi_write_jpg("export.jpg", texture_width, texture_height, 3, data, 90);
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)

    int frame = 0;
    double current_time, elapsed, last_time = 0;
    double avg_fps = 0, avg_ms = 0;

    mouse.set_to_window_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        frame = frame % 500 + 1;
        current_time = glfwGetTime();
        elapsed = current_time - last_time;
        last_time = current_time; 

        printf("%f %f\r", 1 / elapsed, elapsed);

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
        for (j = 0; j < clothsize; j++) {
            texVertices[j * 7    ] = map(points[j]->get_pos_x(), XMIN, XMAX, -1, 1);
            texVertices[j * 7 + 1] = map(points[j]->get_pos_y(), YMIN, YMAX, -1, 1);
            // vertices[j * 3 + 2] = z coordinate

        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(texVertices), texVertices, GL_DYNAMIC_DRAW);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
    }

    collectGarbage(VAO, VBO, shaderProgram);
}