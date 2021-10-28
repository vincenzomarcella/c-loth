#include <cstdio>
#include <cmath>

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

    double magnitude() {
        // Returns the magnitude of the vector
        return sqrt(x * x + y * y);
    }

    private:
        double x;
        double y;
};

const Vec2d GRAVITY{ 0, -1 };

struct PointMass {
    const float DAMPING = .05;
    const float RESTING_DISTANCE = 1; // small values make the simulation explode
                                        // (maybe values smaller than the starting position distance?)
    
    PointMass(double x, double y, bool fixed, int n_neighbors=1)
        : pos{ Vec2d{ x, y } }, fixed{ fixed }, n_neighbors{ n_neighbors } {
        old_pos = Vec2d{ &pos };
        fixed_pos = Vec2d{ &pos };
        // Storing pointer to dynamic array
        neighbors = new PointMass*[n_neighbors]{};
        printf("%d\n", n_neighbors);
    }

    ~PointMass() {
        // Deallocates dynamic array
        delete[] neighbors;
    }

    float get_pos_x() {
        return (float)pos.get_x();
    }

    float get_pos_y() {
        return (float)pos.get_y();
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
        // Same as master branch, but using vectors and dealing with multiple neighbors
        static Vec2d diff, translate;
        static double d, difference;
        for (int i = 0; i < n_neighbors; i++) {
            if (neighbors[i]) {
                diff = pos - neighbors[i]->pos;
                d = diff.magnitude();
                if (abs(d) <= 0)
                    d = 0.001;
                difference = (min(d, RESTING_DISTANCE) - d) / d;
                translate = diff * 0.5 * difference;
                pos += translate;
                neighbors[i]->pos -= translate;
            }
        }
    }

    void update(double dt) {
        // Same as master branch, but using vectors
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

void timestep(PointMass** points, int n_points, int j, double dt) {
    for (j = 0; j < n_points; j++) {
        points[j]->apply_force(GRAVITY);
        points[j]->constrain();
    }
    for (j = 0; j < n_points; j++) {
        points[j]->update(dt);
    }
}