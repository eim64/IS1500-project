#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define NORMALIZE(x, y, len) {                                              \
    const float scalar = Q_rsqrt((x)*(x) + (y)*(y)) * (len);                \
    (x) *= scalar;                                                          \
    (y) *= scalar;                                                          \
}

#define APPLY_COLLISION(x,y, radius) {                                      \
        const int tile_x = (int) (x);                                       \
        const int tile_y = (int) (y);                                       \
                                                                            \
        if (map[tile_y + 1][tile_x] && (tile_y + 1.0 - (y)) < (radius))     \
            (y) = tile_y + 1.0 - (radius);                                  \
                                                                            \
        if (map[tile_y - 1][tile_x] && ((y) - tile_y) < (radius))           \
            (y) = tile_y + (radius);                                        \
                                                                            \
        if (map[tile_y][tile_x + 1] && (tile_x + 1.0 - (x)) < (radius))     \
            (x) = tile_x + 1.0 - (radius);                                  \
                                                                            \
        if (map[tile_y][tile_x - 1] && ((x) - tile_x) < (radius))           \
            (x) = tile_x + (radius);                                        \
                                                                            \
        if (map[tile_y + 1][tile_x + 1])                                    \
            push(&(x), &(y), tile_x + 1.0f, tile_y + 1.0f, (radius));       \
                                                                            \
        if (map[tile_y + 1][tile_x - 1])                                    \
            push(&(x), &(y), tile_x + 0.0f, tile_y + 1.0f, (radius));       \
                                                                            \
        if (map[tile_y - 1][tile_x + 1])                                    \
            push(&(x), &(y), tile_x + 1.0f, tile_y + 0.0f, (radius));       \
                                                                            \
        if (map[tile_y - 1][tile_x - 1])                                    \
            push(&(x), &(y), tile_x + 0.0f, tile_y + 0.0f, (radius));       \
}


float taylor_sin(float val);
float taylor_cos(float val);
float Q_rsqrt( float number );
void push(float* target_x, float* target_y, float source_x, float source_y, float distance);
bool can_hit(const float t_x, const float t_y);