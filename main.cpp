#include <cstdio>
#include <cmath>

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

    void debug_print() {
        // Shows pointer
        printf("x=%f, y=%f, p=%p\n", x, y, this);
    }
    void print() {
        printf("x=%f, y=%f\n", x, y);
    }

    private:
        double x;
        double y;
};

const Vec2d GRAVITY{ 0, 1 };

struct PointMass {
    const float DAMPING = .05;
    const float RESTING_DISTANCE = 5;
    
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

    double get_pos_x() {
        return pos.get_x();
    }

    double get_pos_y() {
        return pos.get_y();
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
                difference = (RESTING_DISTANCE - d) / d;
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

    void print() {
        pos.print();
    }

    private:
        Vec2d pos;
        Vec2d old_pos;
        Vec2d vel{};
        Vec2d acc{};
        bool fixed;
        Vec2d fixed_pos;
        // Pointer to dynamic array containing pointers to PointMass neighbors 
        PointMass** neighbors;
        int n_neighbors;
};

double get_deltat() {
    return 1.0 / 300;
}

int main() {
    PointMass pm1{0, 0, true};
    PointMass pm2{1.5, 4, false};
    PointMass pm3{-1.5, 4, false};

    pm3.add_neighbor(&pm2);
    pm2.add_neighbor(&pm1);
    pm1.add_neighbor(&pm3);

    PointMass* points[]{&pm1, &pm2, &pm3};

    for (int l = 0; l < 3; l++) {
        printf("P%d ", l + 1);
        points[l]->print();
    }

    printf("Simulating 1000 timesteps..\n");

    int i, j;
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 3; j++) {
            points[j]->apply_force(GRAVITY);
            points[j]->constrain();
        }
        for (j = 0; j < 3; j++) {
            points[j]->update(get_deltat());
        }

        printf("P1(%f %f) P2(%f %f) P3(%f %f)\n",
            points[0]->get_pos_x(), points[0]->get_pos_y(),
            points[1]->get_pos_x(), points[1]->get_pos_y(),
            points[2]->get_pos_x(), points[2]->get_pos_y()
        );
    }
    
}

