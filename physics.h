#include <cstdio>
#include <cmath>
#include <random>

double min(double a, double b) {
    if (a < b)
        return a;
    return b;
}

struct Vec2d {
    // Constructors
    Vec2d(double x, double y) : x{ x }, y{ y } {}
    Vec2d(Vec2d* vec) : x{ vec->x }, y{ vec->y } {}
    Vec2d() : x{ 0.0 }, y{ 0.0 } {}

    // Overloading operators
    Vec2d operator+(Vec2d vec) {
        return Vec2d{ x + vec.x, y + vec.y };
    }
    Vec2d operator-(Vec2d vec) {
        return Vec2d{ x - vec.x, y - vec.y };
    }
    Vec2d operator*(double c) {
        return Vec2d{ x * c, y * c };
    }
    Vec2d operator/(double c) {
        return Vec2d{ x / c, y / c };
    }
    void operator+=(Vec2d vec) {
        x += vec.x;
        y += vec.y;
    }
    void operator-=(Vec2d vec) {
        x -= vec.x;
        y -= vec.y;
    }

    double get_x() {
        return x;
    }

    double get_y() {
        return y;
    }

    double magnitude(bool squared=false) {
        // Returns the magnitude of the vector
        double magnitude_squared = x * x + y * y;
        if (!squared)
            return sqrt(magnitude_squared);
        return magnitude_squared;
    }

    private:
        double x;
        double y;
};

const Vec2d GRAVITY{ 0, -30 };

struct PointMass {
    const float DAMPING = .03;
    const float RESTING_DISTANCE = 10;
    const float STIFFNESS = 0.9; // from 0 to 1
    
    PointMass(double x, double y, bool fixed, int n_neighbors=1)
        : pos{ Vec2d{ x, y } }, fixed{ fixed }, n_neighbors{ n_neighbors } {
        old_pos = Vec2d{ &pos };
        fixed_pos = Vec2d{ &pos };
        // Storing pointer to dynamic array
        neighbors = new PointMass*[n_neighbors]{};
    }

    ~PointMass() {
        // Deallocates dynamic array
        delete[] neighbors;
    }

    Vec2d get_pos() {
        return pos;
    }

    float get_pos_x() {
        return (float)pos.get_x();
    }

    float get_pos_y() {
        return (float)pos.get_y();
    }

    void fix_position() {
        fixed = true;
    }

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

    void apply_force(Vec2d force) {
        acc += force;
    }

    void constrain() {
        static Vec2d diff, translate;
        static double d, difference;
        for (int i = 0; i < n_neighbors; i++) {
            if (neighbors[i]) {
                diff = pos - neighbors[i]->pos;
                d = diff.magnitude();
                if (d <= 0)
                    d = 0.001;
                difference = (min(d, RESTING_DISTANCE) - d) / d;
                translate = diff * 0.5 * STIFFNESS * difference;
                pos += translate;
                neighbors[i]->pos -= translate;
            }
        }
    }

    void update(double dt) {
        if (fixed) {
            pos = fixed_pos;
        } else {
            vel = pos - old_pos;
            old_pos = pos;
            pos += (vel * (1 - DAMPING) + acc * dt);
        }
        acc = Vec2d{};
    }

    private:
        Vec2d pos;
        Vec2d old_pos;
        Vec2d vel;
        Vec2d acc{};
        bool fixed;
        Vec2d fixed_pos;
        // Pointer to dynamic array containing pointers to PointMass neighbors 
        PointMass** neighbors;
        int n_neighbors;
};

void timestep(PointMass** points, int n_points, int iterations, double dt, Vec2d mouse_pos, Vec2d mouse_vel) {
    int i, j;
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < n_points; j++)
            points[j]->constrain();
    }

    for (j = 0; j < n_points; j++) {
        points[j]->apply_force(GRAVITY);
        if ((points[j]->get_pos() - mouse_pos).magnitude(true) < 200)
            points[j]->apply_force(mouse_vel * 100);
        points[j]->update(dt);
    }
}