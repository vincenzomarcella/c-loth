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

    Vec2d get_pos() {
        return pos;
    }

    Vec2d get_vel() {
        return vel;
    }

    void set_to_window_size(int width, int height) {
        window_width = width;
        window_height = height;
    }

    bool get_left_button() {
        return left_button;
    }

    bool get_right_button() {
        return right_button;
    }

    void update(GLFWwindow* window, int xmin, int xmax, int ymin, int ymax) {
        glfwGetCursorPos(window, &window_x, &window_y);

        left_button = get_button_state(window, GLFW_MOUSE_BUTTON_LEFT);
        right_button = get_button_state(window, GLFW_MOUSE_BUTTON_RIGHT);

        // Mapping window mouse pos to simulation coordinates 
        pos = Vec2d{ map(window_x, 0, window_width, xmin, xmax),
                     map(window_y, 0, window_height, ymax, ymin)}; // y axis is flipped

        vel = pos - old_pos;
        old_pos = pos;
    }

    private:
        Vec2d old_pos{};
        Vec2d pos{};
        Vec2d vel{};
        int window_width;
        int window_height;
        int xmin, xmax;
        int ymin, ymax;
        bool left_button = false;
        bool right_button = false;

        bool get_button_state(GLFWwindow* window, int button_code) {
            switch (glfwGetMouseButton(window, button_code)) {
                case GLFW_PRESS:
                    return true;
                default:
                    return false;
            };
        }
        
};
