#include <cstdio>

#include "SimplexNoise.h"
#include "utils.h"

const Vec3d GRAVITY{ 0, -10, 0 };

struct PointMass {
    const float DAMPING = .03;
    const float RESTING_DISTANCE = 12;
    const float STIFFNESS = 0.8; // from 0 to 1
    const double MASS = 3.5;
    
    PointMass(double x, double y, double z, bool fixed, int n_neighbors=1)
        : pos{ Vec3d{ x, y, z } }, fixed{ fixed }, n_neighbors{ n_neighbors } {
        old_pos = pos;
        fixed_pos = pos;
        // Storing pointer to dynamic array
        neighbors = new PointMass*[n_neighbors]{};
    }

    ~PointMass() {
        // Deallocates dynamic array
        delete[] neighbors;
    }
    // Returns point pos
    Vec3d get_pos() {
        return pos;
    }
    // Returns the x position coordinate casted to float
    float get_pos_x() {
        return (float)pos.get_x();
    }
    // Returns the y position coordinate casted to float
    float get_pos_y() {
        return (float)pos.get_y();
    }
    // Returns the z position coordinate casted to float
    float get_pos_z() {
        return (float)pos.get_z();
    }
    // Fix the point on its current position
    void fix_position() {
        fixed = true;
        fixed_pos = pos;
    }
    // Unfix the point
    void unfix_position() {
        fixed = false;
    }
    // Links the point to another point
    void add_neighbor(PointMass* neighbor_ptr) {
        // Looping inside neighbors array, placing given neighbor_ptr
        // at the first index where a nullptr is found
        for (int i = 0; i < n_neighbors; i++) {
            if (!neighbors[i]) {
                neighbors[i] = neighbor_ptr;
                return;
            }
        }
        printf("Maximum number of neighbors reached!\n");
    }
    // Adds a given vector to the acceleration
    void apply_force(Vec3d force) {
        acc += force;
    }
    // Moves the point to the given pos
    void drag_to(Vec3d pos) {
        this->pos = pos;
        old_pos = pos;
        fixed_pos = pos;
    }
    // Handles constrain solving for each neighbor
    void constrain() {
        static Vec3d diff, translate;
        static double d, difference;
        for (int i = 0; i < n_neighbors; i++) {
            if (neighbors[i]) {
                diff = pos - neighbors[i]->pos;
                d = diff.magnitude();
                if (d <= 0)
                    d = 0.00001;
                difference = (min(d, RESTING_DISTANCE) - d) / d;
                translate = diff * 0.5 * STIFFNESS * difference;
                if (!fixed)
                    pos += translate;
                if (!(neighbors[i]->fixed))
                    neighbors[i]->pos -= translate;
            }
        }
    }
    // Updates the point position using verlet integration
    void update(double dt) {
        if (fixed) {
            pos = fixed_pos;
        } else {
            vel = pos - old_pos;
            old_pos = pos;
            pos += (vel * (1 - DAMPING) + acc * dt);
        }
        acc = Vec3d{};
    }

    private:
        Vec3d pos;
        Vec3d old_pos;
        Vec3d vel;
        Vec3d acc{};
        bool fixed;
        Vec3d fixed_pos;
        // Pointer to dynamic array containing pointers to PointMass neighbors 
        PointMass** neighbors;
        int n_neighbors;
};

void timestep(PointMass** points, int cols, int rows, int n_points, int iterations, double dt, Mouse* mouse, Camera* camera) {
    Vec3d mouse_pos = mouse->get_pos();
    Vec3d mouse_vel = mouse->get_vel();
    static PointMass* dragged_point = nullptr;

    int i, j;
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < n_points; j++)
            points[j]->constrain();
    }

    double min_distance = INFINITY;
    PointMass* closest_point = nullptr;
    // for (j = 0; j < n_points; j++) {
    float xoff = 0;
    float yoff = 0;
    float min_dist = 1000000;
    glm::vec3 camera_pos = camera->get_pos() * 500.0f;
    glm::vec3 ray = camera->get_direction() * 100.0f;

    for (i = 0; i < rows; i++) {
        xoff = 0;
        for (int k = 0; k < cols; k++) {
            int j = k + i * cols;

            float time = glfwGetTime();

            points[j]->apply_force(GRAVITY * points[j]->MASS);
            float strength = (SimplexNoise::noise(xoff, yoff, time) + 1.0) / 2 * 20;
            float phi = (SimplexNoise::noise(xoff, yoff, time)) * M_PI * 2; // horizontal angle
            float theta = (SimplexNoise::noise(xoff, yoff, time)) * M_PI / 2; // vertical angle
            Vec3d wind = Vec3d{ sin(phi) * cos(theta),
                                sin(phi) * sin(theta),
                                cos(phi) } * strength;
            // points[j]->apply_force(wind);

            glm::vec3 dist_to_camera = glm::vec3(points[j]->get_pos_x(), points[j]->get_pos_y(), points[j]->get_pos_z()) - camera_pos;
            float dist_squared = (dist_to_camera.x * dist_to_camera.x + dist_to_camera.y * dist_to_camera.y + dist_to_camera.z * dist_to_camera.z) -
                glm::dot(ray, dist_to_camera) * glm::dot(ray, dist_to_camera) / 10000.0f;

            if (dist_squared < min_dist) {
                min_dist = dist_squared;
                closest_point = points[j];
            }

            if (dist_squared < 40 && !dragged_point)
                points[j]->apply_force(mouse->get_vel() * 30000);
           
            points[j]->update(dt);
            xoff += 0.03;
        }
        yoff += 0.005;
    }


    if (mouse->get_left_button()) {
        if (dragged_point) {
            dragged_point->fix_position();
            dragged_point->drag_to(Vec3d(camera_pos.x + ray.x * 10, camera_pos.y + ray.y * 10, camera_pos.z + ray.z * 10));
        } else
            dragged_point = closest_point;
    } else if (mouse->get_right_button()) {
        if (closest_point)
            closest_point->unfix_position();
    } else
        dragged_point = nullptr;

}