#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char* vertexShaderSource = ""
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = ""
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

GLFWwindow* createWindow(int width, int height) {
    glfwInit();
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
    // Binding a callback to allow resizing
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

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

void drawFrame(GLFWwindow* window, int nIndices, int shaderProgram, unsigned int VAO) {
    // Clearing the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Activating shader program
    glUseProgram(shaderProgram);
    // Binding the VAO
    glBindVertexArray(VAO);
    // Drawing the triangle
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

    // Swap buffers 
    glfwSwapBuffers(window);
    // Handles input and calls registered callbacks
    glfwPollEvents();
}

void setVertexDataInterpretation() {
    // Telling OpenGL how to interpret the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void collectGarbage(unsigned int VAO, unsigned int VBO, unsigned int shaderProgram) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}
