#include <cmath>

struct Vec2d {
    Vec2d(double x, double y) : x{ x }, y{ y } {}
    Vec2d(Vec2d* vec) : x{ vec->x }, y{ vec->y } {}
    Vec2d() : x{ 0.0 }, y{ 0.0 } {}

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