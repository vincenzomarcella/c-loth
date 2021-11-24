#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* vertexShaderSource = ""   
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";

const char* fragmentShaderSource = ""
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D ourTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(ourTexture, TexCoord);\n"
    "}\0";

GLFWwindow* createWindow(int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); for MacOs

    // Creating GLFW window
    GLFWwindow* window = glfwCreateWindow(width, height, "C-loth", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return nullptr;
    }
    // Making the created window the current context
    glfwMakeContextCurrent(window);
    // The number of screen updates to wait before swapping buffers
    glfwSwapInterval(1);
    
    return window;
}

bool loadGlad() {
    // Loading glad, by passing the pointer to the OS specific OpenGL functions
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return false;
    }
    return true;
}

void checkForShaderSuccess(int shader, int statusType, const char* logName) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, statusType, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::%s::COMPILATION_FAILED\n%s", logName, infoLog);
    }
}

unsigned int getVertexShader() {
    // Generating a vertex shader object
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // Attaching the shader source code to the object
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // Compiling the shader
    glCompileShader(vertexShader);
    
    checkForShaderSuccess(vertexShader, GL_COMPILE_STATUS, "VERTEX");

    return vertexShader;
}

unsigned int getFragmentShader() {
    // Generating a fragment shader object
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    checkForShaderSuccess(fragmentShader, GL_COMPILE_STATUS, "FRAGMENT");

    return fragmentShader;
}

unsigned int getShaderProgram() {
    unsigned int vertexShader = getVertexShader();
    unsigned int fragmentShader = getFragmentShader();

    // Creating a shader program object
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    // Attaching previously compiled shaders
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // Linking the shaders
    glLinkProgram(shaderProgram);

    checkForShaderSuccess(shaderProgram, GL_LINK_STATUS, "PROGRAM");

    // Deleting shaders objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

unsigned int getVAO() {
    // Generating a vertex array object
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // Bind the vertex array object
    glBindVertexArray(VAO);

    return VAO;
}

unsigned int getVBO() {
    // Generating a vertex buffer object
    unsigned int VBO; // Unique ID
    // Passing the number of objects to generate and a pointer to
    // an array of uints where to store the names of the objects
    // Since it's one object, it is valid to pass the pointer of a single value
    glGenBuffers(1, &VBO); 
    // Binding the buffer (actually creating the buffer), 
    // specifying the binding target and the name of the buffer object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Copying user-defined vertex data into buffer's memory
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    return VBO;
}

unsigned int getEBO() {
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    return EBO;
}


void setVertexDataInterpretation() {
    // Telling OpenGL how to interpret the vertex data
    // Configuring the vertex array striding
    // The first two values are the vertex location
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    // The next three attributes are the color
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);  
    // The last two attributes are the texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}

unsigned int setTexture(const char* image_filepath) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Configuring the way that textures are repeated even though it should not happen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // Configuring linear texture mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    // Configuring bilinear texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    setVertexDataInterpretation();
    
    // Loading the texture
    int texture_width, texture_height, nrChannels;
    unsigned char *data = stbi_load(image_filepath, &texture_width, &texture_height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else
        printf("Failed to load texture\n");
    stbi_image_free(data);

    return texture;
}

void drawFrame(GLFWwindow* window, int nIndices, int shaderProgram, unsigned int VAO) {
    // Clearing the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Binding the VAO
    glBindVertexArray(VAO);
    // Drawing the triangles
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    glPointSize(3);
    glDrawArrays(GL_POINTS, 40 * 30, 1);
    // Swap buffers 
    glfwSwapBuffers(window);
    // Handles input and calls registered callbacks
    glfwPollEvents();
}


void collectGarbage(unsigned int VAO, unsigned int VBO, unsigned int shaderProgram) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}
