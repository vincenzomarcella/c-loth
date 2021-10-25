#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#define RESTING_DISTANCE 10
#define GRAVITY 10

struct PointMass;

typedef struct PointMass pointMass;

struct PointMass
{
    double x;
    double y;
    double oldX;
    double oldY;
    double velX;
    double velY;
    double accX;
    double accY;
    pointMass * neighbour;
    bool fixed;
    double fixedX;
    double fixedY;
};

void update(struct PointMass* point, double dT){
    if(point->fixed == false){
        point->velX = point->x - point->oldX;
        point->velY = point->y - point->oldY;

        point->oldX = point->x;
        point->oldY = point->y;

        point->x = point->x + point->velX * 0.95 + point->accX * dT;
        point->y = point->y + point->velY * 0.95  + point->accY * dT;

        // point->accX = 0;
        // point->accY = 0;
    } else {
        point->x = point->fixedX;
        point->y = point->fixedY;
    }
}

void constrain(struct PointMass* point) {
    double diffX, diffY, translateX, translateY, d, difference;
    if(point->neighbour){
        diffX = point->x - point->neighbour->x;
        diffY = point->y - point->neighbour->y;
        d = sqrt(diffX * diffX + diffY * diffY);
        difference = (RESTING_DISTANCE - d) / d;
        translateX = diffX * 0.5 * difference;
        translateY = diffY * 0.5 * difference;
        point->x += translateX;
        point->y += translateY;
        point->neighbour->x -= translateX;
        point->neighbour->y -= translateY;
    }
}

int main() {
    struct PointMass punto1 = {
        0,
        100,
        0,
        100,
        0,
        0,
        0,
        GRAVITY,
        NULL,
        true,
        0,
        100
    };
    struct PointMass punto2 = {
        5,
        90,
        5,
        90,
        0,
        0,
        0,
        GRAVITY,
        &punto1,
        false
    };
    struct PointMass punto3 = {
        -5,
        90,
        5,
        90,
        0,
        0,
        0,
        GRAVITY,
        &punto2,
        false
    };
    // Collego il primo punto al punto 3 per fare un triangolo
    punto1.neighbour = &punto3;

    double dt = 1.0 / 300;

    for(int i = 0; i < 1000; i++) {
        
        constrain(&punto1);
        constrain(&punto2);
        constrain(&punto3);

        update(&punto1, dt);
        update(&punto2, dt);
        update(&punto3, dt);
        
        printf("[%d] P1(%f %f) P2(%f %f) P3(%f %f)\n", i,
            punto1.x, punto1.y, punto2.x, punto2.y, punto3.x, punto3.y);

    }
    return 0;
}