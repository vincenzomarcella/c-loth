#include <stdexcept>

#include "graphics.h"
#include "vec.h"

// Returns the minimum double between the two given
double min(double a, double b) {
    if (a < b)
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

        // Mapping window mouse pos to simulation coordinates 
        pos = Vec3d{ map(window_x, 0, window_width, xmin, xmax),
                     map(window_y, 0, window_height, ymax, ymin), // y axis is flipped
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
    Camera (double x, double y, double z) : pos{ x, y, z } {}

    Vec3d perspective_projection(Vec3d point) {
        double near = VIEW_WIDTH * tan((M_PI - FOVX) / 2) / 2;
        double far = near + DEPTH;

        double x = point.get_x() - pos.get_x();
        double y = point.get_y() - pos.get_y();
        double z = point.get_z() - pos.get_z();


        return Vec3d{ ((x * near / -z) / VIEW_WIDTH),
                      ((y * near / -z) / VIEW_HEIGHT),
                      map(z, pos.get_z() - near, pos.get_z() - far, 1, -1)
                    };
    }

    void update(GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_LEFT))
            pos += Vec3d{ -10, 0, 0};
        if (glfwGetKey(window, GLFW_KEY_RIGHT))
            pos += Vec3d{ 10, 0, 0};
        if (glfwGetKey(window, GLFW_KEY_UP))
            pos += Vec3d{ 0, 10, 0 };
        if (glfwGetKey(window, GLFW_KEY_DOWN))
            pos += Vec3d{ 0, -10, 0 };
        if (glfwGetKey(window, GLFW_KEY_S))
            pos += Vec3d{ 0, 0, 10 };
        if (glfwGetKey(window, GLFW_KEY_W))
            pos += Vec3d{ 0, 0, -10 };
    }

    Vec3d get_pos() {
        return pos;
    }

    private:
        Vec3d pos;
        const double FOVX = M_PI / 1.5;
        // Camera view plane resolution
        const int VIEW_WIDTH = 800;
        const int VIEW_HEIGHT = 600;
        const int DEPTH = 3000;
};