
#define BINOMIAL_COEFFICIENT_TABLE_MAX_N 12
static const uint16_t binomial_coefficient[BINOMIAL_COEFFICIENT_TABLE_MAX_N + 1][BINOMIAL_COEFFICIENT_TABLE_MAX_N + 1] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 4, 6, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    { 1, 5, 10, 10, 5, 1, 0, 0, 0, 0, 0, 0, 0},
    { 1, 6, 15, 20, 15, 6, 1, 0, 0, 0, 0, 0, 0},
    { 1, 7, 21, 35, 35, 21, 7, 1, 0, 0, 0, 0, 0},
    { 1, 8, 28, 56, 70, 56, 28, 8, 1, 0, 0, 0, 0},
    { 1, 9, 36, 84, 126, 126, 84, 36, 9, 1, 0, 0, 0},
    { 1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1, 0, 0},
    { 1, 11, 55, 165, 330, 462, 462, 330, 165, 55, 11, 1, 0},
    { 1, 12, 66, 220, 495, 792, 924, 792, 495, 220, 66, 12, 1},
};
// The de Casteljau algorithm for Bezier patches evaluates a point by repeated bilinear interpolation, a direct computation
// from the definition. Scratch space is used, and the top-left of the grid is written over each time so only this scratch space is needed.
#define MAX_NUM_BEZIER_PATCH_POINTS 1024
vec3 evaluate_bezier_patch_de_casteljau(int n, int m, vec3 *points, float u, float v)
{
    static vec3 scratch[MAX_NUM_BEZIER_PATCH_POINTS];
    if (n * m > MAX_NUM_BEZIER_PATCH_POINTS) {
        fprintf(stderr, "ERROR: Too many points in Bezier patch.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(scratch, points, sizeof(vec3)*n*m);
    
    int max_degree = n > m ? n : m; 
    for (int k = max_degree-1; k > 0; --k) {
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < k; j++) {
                scratch[j*n + i] = vec3_bilerp(scratch[j*n + i], scratch[j*n + i+1], scratch[(j+1)*n + i], scratch[(j+1)*n + i+1], u, v);
            }
        }
    }
    if (n < max_degree) {
        for (int k = max_degree - n; k > 0; --k) {
            for (int i = 0; i < k; i++) {
                scratch[i*n] = vec3_lerp(scratch[i*n], scratch[(i+1)*n], v);
            }
        }
    } else if (m < max_degree) {
        for (int k = max_degree - m; k > 0; --k) {
            for (int i = 0; i < k; i++) {
                scratch[i] = vec3_lerp(scratch[i], scratch[i+1], u);
            }
        }
    }
    return scratch[0];
}

vec3 evaluate_rational_bezier_patch_de_casteljau(int n, int m, vec4 points[], float u, float v)
{
    static vec4 scratch[MAX_NUM_BEZIER_PATCH_POINTS];
    if (n * m > MAX_NUM_BEZIER_PATCH_POINTS) {
        fprintf(stderr, "ERROR: Too many points in Bezier patch.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(scratch, points, sizeof(vec4)*n*m);
    
    int max_degree = n > m ? n : m; 
    for (int k = max_degree-1; k > 0; --k) {
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < k; j++) {
                scratch[j*n + i] = vec4_bilerp(scratch[j*n + i], scratch[j*n + i+1], scratch[(j+1)*n + i], scratch[(j+1)*n + i+1], u, v);
            }
        }
    }
    if (n < max_degree) {
        for (int k = max_degree - n; k > 0; --k) {
            for (int i = 0; i < k; i++) {
                scratch[i*n] = vec4_lerp(scratch[i*n], scratch[(i+1)*n], v);
            }
        }
    } else if (m < max_degree) {
        for (int k = max_degree - m; k > 0; --k) {
            for (int i = 0; i < k; i++) {
                scratch[i] = vec4_lerp(scratch[i], scratch[i+1], u);
            }
        }
    }
    vec4 p = scratch[0];
    return new_vec3(X(p) / W(p), Y(p) / W(p), Z(p) / W(p));
}

vec3 evaluate_rational_cubic_bezier_patch(vec4 points[], float u, float v)
{
    return evaluate_rational_bezier_patch_de_casteljau(4, 4, points, u, v);
}
