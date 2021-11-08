#include <cstdio>

#include "utils.h"

const Vec3d GRAVITY{ 0, -25, 0};

struct PointMass {
    const float DAMPING = .03;
    const float RESTING_DISTANCE = 8;
    const float STIFFNESS = 0.8; // from 0 to 1
    
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

void timestep(PointMass** points, int n_points, int iterations, double dt, Mouse* mouse) {
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
    for (j = 0; j < n_points; j++) {
        points[j]->apply_force(GRAVITY);
        double distance_to_mouse_squared = (points[j]->get_pos() - mouse_pos).magnitude2d(true);
        if (distance_to_mouse_squared < 200 && !dragged_point)
            points[j]->apply_force(mouse->get_vel() * 50 * 0 + Vec3d(0, 0, -300));
        if (distance_to_mouse_squared < 100 && distance_to_mouse_squared < min_distance) {
            min_distance = distance_to_mouse_squared;
            closest_point = points[j];
        }
        points[j]->update(dt);
    }

    if (mouse->get_left_button()) {
        if (dragged_point) {
            dragged_point->fix_position();
            dragged_point->drag_to(mouse_pos);
        } else
            dragged_point = closest_point;
    } else if (mouse->get_right_button()) {
        if (closest_point)
            closest_point->unfix_position();
    } else
        dragged_point = nullptr;

}