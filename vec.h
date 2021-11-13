#include <cmath>

struct Vec3d {
    Vec3d(double x, double y, double z) : x{ x }, y{ y }, z{ z } {}
    Vec3d(Vec3d* vec) : x{ vec->x }, y{ vec->y }, z{ vec->z }  {}
    Vec3d() : x{ 0.0 }, y{ 0.0 }, z{ 0.0 } {}

    Vec3d operator+(Vec3d vec) {
        return Vec3d{ x + vec.x, y + vec.y, z + vec.z };
    }
    Vec3d operator-(Vec3d vec) {
        return Vec3d{ x - vec.x, y - vec.y, z - vec.z };
    }
    Vec3d operator*(double c) {
        return Vec3d{ x * c, y * c, z * c};
    }
    Vec3d operator/(double c) {
        return Vec3d{ x / c, y / c, z / c};
    }
    void operator+=(Vec3d vec) {
        x += vec.x;
        y += vec.y;
        z += vec.z;
    }
    void operator-=(Vec3d vec) {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
    }

    double get_x() {
        return x;
    }

    double get_y() {
        return y;
    }

    double get_z() {
        return z;
    }

    // Returns the (squared or not) magnitude of the vector
    double magnitude(bool squared=false) {
        double magnitude_squared = x * x + y * y + z * z;
        if (!squared)
            return sqrt(magnitude_squared);
        return magnitude_squared;
    }
    // Returns the (squared or not) magnitude of the vector considering only xy axes
    double magnitude2d(bool squared=false) {
        double magnitude_squared = x * x + y * y;
        if (!squared)
            return sqrt(magnitude_squared);
        return magnitude_squared;
    }

    void print() {
        printf("(%f, %f, %f)\n", x, y, z);
    }

    private:
        double x;
        double y;
        double z;
};