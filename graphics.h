#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* vertexShaderSource = ""   
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "    ourColor = aColor;\n"
    "    TexCoord = aTexCoord;\n"
    "}\0";
/*
const char* fragmentShaderSource = ""
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";
*/
/*
const char* fragmentShaderSource = ""
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n"
    "void main() {\n"
    "   FragColor = ourColor;\n"
    "}\0";
*/

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


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

GLFWwindow* createWindow(int width, int height) {
    if(!glfwInit()) {
        glfwSetErrorCallback(glfw_error_callback);
    }
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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

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
    glfwMakeContextCurrent(window);
    // Clearing the screen
    // glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    float timeValue = glfwGetTime();

    // Activating shader program
    glUseProgram(shaderProgram);
    // Binding the VAO
    glBindVertexArray(VAO);
    // Drawing the triangles
    //glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.2f, 1.0f);
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    //glLineWidth(2);
    // Drawing the points
    glUniform4f(vertexColorLocation, 0.0f, 0.0f, 0.0f, 1.0f);
    //glDrawElements(GL_POINTS, nIndices, GL_UNSIGNED_INT, 0);
    //glPointSize(2);

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    //glClearColor(GUIState->clear_color.x * GUIState->clear_color.w, GUIState->clear_color.y * GUIState->clear_color.w, GUIState->clear_color.z * GUIState->clear_color.w, GUIState->clear_color.w);
    //glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers 
    glfwSwapBuffers(window);
    // Handles input and calls registered callbacks
    glfwPollEvents();
}

class ImGuiState {
    public: 
        bool show_helper_window = true;
        bool show_another_window = false;
        bool wireframe_enabled = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

void setVertexDataInterpretation() {
    // Telling OpenGL how to interpret the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void collectGarbage(unsigned int VAO, unsigned int VBO, unsigned int shaderProgram) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    // ImGui Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}