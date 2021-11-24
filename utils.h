#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "graphics.h"
#include "vec.h"

// Returns the minimum double between the two given
double min(double a, double b) {
    if (a < b)
        return a;
    return b;
}
// Returns the maximum double between the two given
double max(double a, double b) {
    if (a > b)
        return a;
    return b;
}

// Maps value from range [start1, stop1] to [start2, stop2]
double map(double value, double start1, double stop1, double start2, double stop2) {
    if (stop1 == start1)
        throw std::runtime_error{ "Map starting range can't be 0 width!" };
    return  start2 + (stop2 - start2) * (value - start1) / (stop1 - start1);
}

// Converts a (i, j) 2d pair of indexes to 1d  
int to1d_index(int i, int j, int width) {
    return i * width + j;
}

// A class to handle mouse inputs
struct Mouse {
    double window_x;
    double window_y;
    // Returns the mouse pos mapped to simulation space
    Vec3d get_pos() {
        return pos;
    }
    // Returns the mouse velocity
    Vec3d get_vel() {
        return vel;
    }
    // Adjust mouse mapping variables to window size
    void set_to_window_size(int width, int height) {
        window_width = width;
        window_height = height;
    }
    // Returns wether the left button is being pressed
    bool get_left_button() {
        return left_button;
    }
    // Returns wether the right button is being pressed
    bool get_right_button() {
        return right_button;
    }
    // Gets current mouse position relative to window, updates button states,
    // maps mouse position to given coordinates ranges and updates mouse velocity
    void update(GLFWwindow* window, int xmin, int xmax, int ymin, int ymax) {
        glfwGetCursorPos(window, &window_x, &window_y);

        left_button = get_button_state(window, GLFW_MOUSE_BUTTON_LEFT);
        right_button = get_button_state(window, GLFW_MOUSE_BUTTON_RIGHT);

        // Mapping window mouse pos to omogeneus ndc coordinates 
        pos = Vec3d{ map(window_x, 0, window_width, -1, 1),
                     map(window_y, 0, window_height, 1, -1), // y axis is flipped
                     0.0 }; 

        vel = pos - old_pos;
        old_pos = pos;
    }

    private:
        Vec3d old_pos{};
        Vec3d pos{};
        Vec3d vel{};
        int window_width;
        int window_height;
        int xmin, xmax;
        int ymin, ymax;
        bool left_button = false;
        bool right_button = false;

        // Returns wether a button is being pressed given its GLFW code
        bool get_button_state(GLFWwindow* window, int button_code) {
            switch (glfwGetMouseButton(window, button_code)) {
                case GLFW_PRESS:
                    return true;
                default:
                    return false;
            };
        }
        
};

struct Camera {

    void load_matrices(unsigned int shaderProgram) {
        // Loading uniforms matrices 
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }

    void set_to_window_size(int width, int height) {
        view_width = width;
        view_heigth = height;
    }

    static void zoom(GLFWwindow* window, double xoff, double yoff) {
        Camera::fovy = (int)min(max(1, (Camera::fovy + yoff)), 179);
    }

    void update(GLFWwindow* window, unsigned int shaderProgram, Vec3d mouse_pos) {
        float dt = (1.0f / 60);

        mouse = glm::vec3(mouse_pos.get_x(), mouse_pos.get_y(), 0);

        yaw_vel += (mouse.x - last_mouse.x) * MOUSE_SENS * dt;
        pitch_vel += (mouse.y - last_mouse.y) * MOUSE_SENS * dt;

        yaw += yaw_vel * dt;
        pitch += pitch_vel * dt;

        yaw_vel *= 0.9;
        pitch_vel *= 0.9;

        // if (pitch > 89.0f)
        //     pitch = 89.0f;
        // if (pitch < -89.0f)
        //     pitch = -89.0f;

        glm::vec3 last_mouse = glm::vec3(mouse);

        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);

        glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(world_up, direction));
        glm::vec3 up = glm::cross(direction, right);

        glm::vec3 acc = glm::vec3(0.0f, 0.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_A))
            acc -= right;
        if (glfwGetKey(window, GLFW_KEY_D))
            acc += right;
        if (glfwGetKey(window, GLFW_KEY_S))
            acc += direction;
        if (glfwGetKey(window, GLFW_KEY_W))
            acc -= direction;
        if (glfwGetKey(window, GLFW_KEY_UP))
            acc -= up;
        if (glfwGetKey(window, GLFW_KEY_DOWN))
            acc += up;

        // Normalizing acceleration
        if (sqrt(acc.x * acc.x + acc.y * acc.y + acc.z * acc.z) > 0)
            acc = glm::normalize(acc) * 5.0f;

        vel = vel + acc * dt;
        // Limiting velocity magnitude
        if (sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z) > 5.0f)
            vel = glm::normalize(vel) * 5.0f;

        pos = pos - vel * dt;
        vel *= 0.95;
            
        view = glm::lookAt(pos, pos + direction, up);
        int modelLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(view));

        projection = glm::perspective(glm::radians(fovy),
            view_width / view_heigth, ZNEAR, ZFAR);
        modelLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projection));

    }

    glm::vec3 get_pos() {
        return pos;
    }

    glm::vec3 get_direction() {
        return direction;
    }

    static float fovy; // In degrees

    private:
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 vel = glm::vec3();
        glm::vec3 direction;
        glm::vec3 mouse = glm::vec3();
        glm::vec3 last_mouse = glm::vec3(mouse);

        float view_width = 800;
        float view_heigth = 600;
        const float ZNEAR = 0.1f;
        const float ZFAR = 100.0f;
        float yaw = -90.0f; // Angle used to rotate camera direction around z axis
        float pitch = 0; // Angle used to rotate camera direction around x axis
        float yaw_vel = 0;
        float pitch_vel = 0;
        const int MOUSE_SENS = 3000;

      
        // Defining matrices
        // object local coordinates matrix
        glm::mat4 model = glm::mat4(1.0f); 
        // view matrix, represents the camera view
        glm::mat4 view; 
        // perspective projection matrix -> to get clip coordinates
        glm::mat4 projection; 
};