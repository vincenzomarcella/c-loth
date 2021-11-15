#include <cstdio>
#include <chrono>
#include <thread>

#include "physics.h"
#include "graphics.h"

#include <GLFW/glfw3.h>

double map_in_range(double value, double from, double to, double mapFrom, double mapTo) {
    return  mapFrom + (mapTo - mapFrom) * (value - from) / (to - from);
}

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

    const int clothsize = WIDTH * HEIGHT;

    float vertices[3 * clothsize]{};
    
    // Array that containts the texture vertices data
    float texVertices[7 * clothsize]{};
    for (i = 0; i < HEIGHT; i++){
        for(j = 0; j < WIDTH; j++){
            texVertices[i * (WIDTH * 7) + (j * 7)] = map_in_range(points[i * WIDTH + j]->get_pos_x(), -300, 300, -1, 1);
            texVertices[i * (WIDTH * 7) + (j * 7) + 1] = map_in_range(points[i * WIDTH + j]->get_pos_y(), -300, 300, -1, 1);
            /*printf(
                "Coords for point x: %d, y: %d, x: %f, y:%f\n", 
                i, 
                j,
                texVertices[i * WIDTH + j],
                texVertices[i * WIDTH + j + 1]);*/
            texVertices[i * (WIDTH * 7) + (j * 7) + 2] = 1.0f;
            texVertices[i * (WIDTH * 7) + (j * 7) + 3] = 1.0f;
            texVertices[i * (WIDTH * 7) + (j * 7) + 4] = 1.0f;
            texVertices[i * (WIDTH * 7) + (j * 7) + 5] = map_in_range(points[i * WIDTH + j]->get_pos_x(), -300, 300, -1, 1);
            texVertices[i * (WIDTH * 7) + (j * 7) + 6] = map_in_range(points[i * WIDTH + j]->get_pos_y(), -300, 300, -1, 1);
            /*printf("Vertexes generated:\nindexes: %d %d x:%f y:%f,\nr:%f g:%f b:%f,\nx:%f y:%f\n",
            i * WIDTH + (j * 7),
            i * WIDTH + (j * 7) + 1,
            texVertices[i * (WIDTH * 7) + (j * 7)    ],
            texVertices[i * (WIDTH * 7) + (j * 7) + 1],
            texVertices[i * (WIDTH * 7) + (j * 7) + 2],
            texVertices[i * (WIDTH * 7) + (j * 7) + 3],
            texVertices[i * (WIDTH * 7) + (j * 7) + 4],
            texVertices[i * (WIDTH * 7) + (j * 7) + 5],
            texVertices[i * (WIDTH * 7) + (j * 7) + 6]);*/
        }
    }

    unsigned int indices[clothsize * 2 * 3]{};
    // TODO: Rework the function that generates the indices.
    for (i = 0; i < HEIGHT - 1; i++) {
        for (j = 0; j < WIDTH - 1; j++) {
            indices[6 * (i * (WIDTH - 1) + j)    ] = i * (WIDTH * 7) + (j * 7);
            indices[6 * (i * (WIDTH - 1) + j) + 1] = i * (WIDTH * 7) + (j * 7) + 1;
            indices[6 * (i * (WIDTH - 1) + j) + 2] = (i + 1) * (WIDTH * 7) + (j * 7);

            indices[6 * (i * (WIDTH - 1) + j) + 3] = i * (WIDTH * 7) + (j * 7) + 1;
            indices[6 * (i * (WIDTH - 1) + j) + 4] = (i + 1) * (WIDTH * 7) + (j * 7);
            indices[6 * (i * (WIDTH - 1) + j) + 5] = (i + 1) * (WIDTH * 7) + (j * 7) + 1;
            printf("Objects generated:\nindexes: %d %d x:%f y:%f,\nindexes: %d %d x:%f y:%f,\nindexes: %d %d x:%f y:%f\n",
            indices[6 * (i * (WIDTH - 1) + j)    ],
            indices[6 * (i * (WIDTH - 1) + j) + 1],
            texVertices[indices[6 * (i * (WIDTH - 1) + j)    ]],
            texVertices[indices[6 * (i * (WIDTH - 1) + j) + 1]],
            indices[6 * (i * (WIDTH - 1) + j) + 2],
            indices[6 * (i * (WIDTH - 1) + j) + 3],
            texVertices[indices[6 * (i * (WIDTH - 1) + j) + 2]],
            texVertices[indices[6 * (i * (WIDTH - 1) + j) + 3]],
            indices[6 * (i * (WIDTH - 1) + j) + 4],
            indices[6 * (i * (WIDTH - 1) + j) + 5],
            texVertices[indices[6 * (i * (WIDTH - 1) + j) + 4]],
            texVertices[indices[6 * (i * (WIDTH - 1) + j) + 5]]);
        }
    }

    GLFWwindow* window = createWindow(800, 600);
    if (!window || !loadGlad())
        return -1;

    unsigned int shaderProgram = getShaderProgram();
    
    unsigned int VAO = getVAO();
    unsigned int VBO = getVBO();
    unsigned int EBO = getEBO();

    // Load the vertex data inside the vertex buffer object
    // Load the vertex indices inside of the element buffer object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    setVertexDataInterpretation();
    // Wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Configuring the way that textures are repeated even though it should not happen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // Configuring linear texture mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
    unsigned char *data = stbi_load("jeans.jpeg", &texture_width, &texture_height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture\n");
    }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)

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
            texVertices[j * 7] = map_in_range(points[j]->get_pos_x(), -300, 300, -1, 1);
            texVertices[j * 7 + 1] = map_in_range(points[j]->get_pos_y(), 300, -300, 1, -1);
        }

        // Loading vertices into buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(texVertices), texVertices, GL_DYNAMIC_DRAW);

        drawFrame(window, sizeof(indices) / sizeof(unsigned int), shaderProgram, VAO);
        //getchar();
    }

    collectGarbage(VAO, VBO, shaderProgram);
}