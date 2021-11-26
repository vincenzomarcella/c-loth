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
    "out vec3 Normal;\n"
    "out vec3 FragPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "   TexCoord = aTexCoord;\n"
    "   Normal = normalize(vec3(0.0, 0.0, 3.0));\n"
    "   FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "}\0";

const char* fragmentShaderSource = ""
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "in vec2 TexCoord;\n"
    "in vec3 Normal;\n"
    "in vec3 FragPos;\n"
    "uniform sampler2D ourTexture;\n"
    "vec3 lightPos = vec3(0.0, 0.0, 3.0);\n"
    "void main()\n"
    "{\n"
    "   float ambientStrength = 0.1;\n"
    "   vec3 lightColor = vec3(1.0, 1.0, 1.0);\n"
    "   vec3 ambient = lightColor * ambientStrength;\n"
    "   vec3 lightDir = normalize(lightPos - FragPos);\n"
    "   vec3 diffuse = lightColor * max(dot(Normal, lightDir), 0.0);\n"
    "   FragColor = texture(ourTexture, TexCoord) * vec4(ambient + diffuse, 1.0);\n"
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

void drawFrame(GLFWwindow* window, int n_points, int nIndices, int shaderProgram, unsigned int VAO) {
    glfwMakeContextCurrent(window);
    // Clearing the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Binding the VAO
    glBindVertexArray(VAO);
    // Drawing the triangles
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    // Drawing crosshair
    glPointSize(3);
    glDrawArrays(GL_POINTS, n_points, 1);

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
        bool wireframe_enabled = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

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