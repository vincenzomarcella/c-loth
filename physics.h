#include <cstdio>

#include "SimplexNoise.h"
#include "utils.h"

const Vec3d GRAVITY{ 0, -10, 0 };
const float MAX_WIND_STRENGHT = 20;

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
    Vec3d get_pos() const {
        return pos;
    }
    // Returns the x position coordinate casted to float
    float get_pos_x() const {
        return (float)pos.get_x();
    }
    // Returns the y position coordinate casted to float
    float get_pos_y() const {
        return (float)pos.get_y();
    }
    // Returns the z position coordinate casted to float
    float get_pos_z() const {
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
    void apply_force(glm::vec3 force) {
        acc += Vec3d(force.x, force.y, force.z);
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


void timestep(
    PointMass** points,
    int cols, int rows,
    int n_points,
    int iterations,
    double dt,
    Mouse* mouse,
    Camera* camera) {

    static PointMass* dragged_point = nullptr;
    static float dragged_dist; // Distance of dragged point from camera when it was picked

    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < n_points; j++)
            points[j]->constrain();
    }

    double min_distance = INFINITY;
    PointMass* closest_point = nullptr;
    // for (j = 0; j < n_points; j++) {
    float noise_xoff = 0;
    float noise_yoff = 0;
    float min_dist = INFINITY;
    float min_dist_to_camera;
    glm::vec3 camera_pos = camera->get_pos() * 500.0f; // Why does this value work?
    glm::vec3 camera_direction = camera->get_direction() * camera->get_zfar(); 

    for (int i = 0; i < rows; i++) {
        // Resetting noise xoffset
        noise_xoff = 0;
        for (int j = 0; j < cols; j++) {
            int k = j + i * cols; // 1d index
            float time = glfwGetTime();

            // Calculating wind vector
            float wind_strength = map(
                SimplexNoise::noise(noise_xoff, noise_yoff, time),
                -1, 1, 0, MAX_WIND_STRENGHT);
            float wind_phi = map( // Horizontal rotation angle
                SimplexNoise::noise(noise_xoff, noise_yoff, time),
                -1, 1, -M_PI, M_PI); 
            float wind_theta = map( // Vertical rotation angle
                SimplexNoise::noise(noise_xoff, noise_yoff, time),
                -1, 1, -M_PI_2, M_PI_2);
            Vec3d wind = Vec3d{ sin(wind_phi) * cos(wind_theta),
                                sin(wind_phi) * sin(wind_theta),
                                cos(wind_phi) } * wind_strength;

            // Calculating closest point to camera direction
            glm::vec3 dist_to_camera = glm::vec3(
                points[k]->get_pos_x(),
                points[k]->get_pos_y(),
                points[k]->get_pos_z()) - camera_pos;
            float dist_to_direction_squared = glm::length2(dist_to_camera) -
                pow(glm::dot(camera_direction, dist_to_camera) / camera->get_zfar(), 2);

            if (dist_to_direction_squared < min_dist) {
                min_dist = dist_to_direction_squared;
                min_dist_to_camera = glm::length(dist_to_camera);
                closest_point = points[k];
            }

            // Adding forces
            points[k]->apply_force(GRAVITY * points[k]->MASS);
            points[k]->apply_force(wind);
            if (dist_to_direction_squared < 40 && !dragged_point)
                points[k]->apply_force(camera->get_direction_vel() * 60000.0f);
           
            points[k]->update(dt);
            noise_xoff += 0.03;
        }
        noise_yoff += 0.005;
    }

    if (mouse->get_left_button()) {
        if (dragged_point) {
            camera_direction *= dragged_dist;
            dragged_point->fix_position();
            dragged_point->drag_to(Vec3d(camera_pos + camera_direction));

        } else {
            dragged_point = closest_point;
            dragged_dist = min_dist_to_camera / camera->get_zfar();
        }
    } else if (mouse->get_right_button()) {
        if (closest_point)
            closest_point->unfix_position();
    } else
        dragged_point = nullptr;

}