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
    // Returns the mouse pos mapped to ndc coordinates
    Vec3d get_pos() const {
        return pos;
    }
    // Returns the mouse velocity
    Vec3d get_vel() const {
        return vel;
    }
    // Adjust mouse mapping variables to window size
    void set_to_window_size(int width, int height) {
        window_width = width;
        window_height = height;
    }
    // Returns wether the left button is being pressed
    bool get_left_button() const {
        return left_button;
    }
    // Returns wether the right button is being pressed
    bool get_right_button() const {
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

    static void activate_cursor_interaction(GLFWwindow* window, int a) {
        Camera::is_cursor_in_window = true;
    }

    void update(GLFWwindow* window, unsigned int shaderProgram, float dt, Vec3d mouse_pos) {
        glm::vec2 mouse = glm::vec2(mouse_pos.get_x(), mouse_pos.get_y());

        yaw_vel += mouse.x * MOUSE_SENS * dt * is_cursor_in_window;
        pitch_vel += mouse.y * MOUSE_SENS * dt * is_cursor_in_window;
        // TODO: limit rotation velocities
        yaw += yaw_vel * dt;
        pitch += pitch_vel * dt;

        // Constraining vertical camera rotation
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // Calculating direction based on yaw and pitch angles
        last_direction = glm::vec3(direction);
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);

        yaw_vel *= (1 - ROTATION_FRICTION);
        pitch_vel *= (1 - ROTATION_FRICTION);

        // Calculating camera right and up axes
        glm::vec3 right = glm::normalize(glm::cross(WORLD_UP, direction));
        glm::vec3 up = glm::cross(direction, right);

        glm::vec3 acc = glm::vec3();
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
        if (glm::length2(acc) > 0)
            acc = glm::normalize(acc) * MAX_ACCELERATION;

        vel = vel + acc * dt;
        // Limiting velocity magnitude
        if (glm::length(vel) > MAX_VELOCITY)
            vel = glm::normalize(vel) * MAX_VELOCITY;

        pos = pos - vel * dt;
        vel *= (1 - MOVEMENT_FRICTION);

        view = glm::lookAt(pos, pos + direction, up);
        int modelLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(view));

        projection = glm::perspective(glm::radians(fovy),
            view_width / view_heigth, ZNEAR, ZFAR);
        modelLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projection));

    }

    glm::vec3 get_pos() const {
        return pos;
    }

    glm::vec3 get_direction() const {
        return direction;
    }

    glm::vec3 get_direction_vel() const {
        return direction - last_direction;
    }

    float get_zfar() const {
        return ZFAR;
    }

    static float fovy; // In degrees
    static bool is_cursor_in_window;

    private:
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 vel;
        glm::vec3 direction;
        glm::vec3 last_direction;

        const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

        float view_width = 800;
        float view_heigth = 600;
        const float ZNEAR = 0.1f;
        const float ZFAR = 100.0f;

        float yaw = -90.0f; // Angle used to rotate camera direction around z axis
        float pitch = 0; // Angle used to rotate camera direction around x axis
        float yaw_vel = 0;
        float pitch_vel = 0;

        float MOUSE_SENS = 5000;
        const float MOVEMENT_FRICTION = 0.05;
        const float ROTATION_FRICTION = 0.1;
        const float MAX_ACCELERATION = 5.0;
        const float MAX_VELOCITY = 10.0;
        const float MAX_ROTATION_VELOCITY = 30.0f;

        // Defining matrices
        // object local coordinates matrix
        glm::mat4 model = glm::mat4(1.0f); 
        // view matrix, represents the camera view
        glm::mat4 view; 
        // perspective projection matrix -> to get clip coordinates
        glm::mat4 projection; 
};