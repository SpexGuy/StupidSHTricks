//
// Created by Martin Wickham on 10/25/2016.
//

#ifndef STUPIDSHTRICKS_LEGENDRE_H
#define STUPIDSHTRICKS_LEGENDRE_H

#include <math.h>
#include <cmath>
#include <glm/glm.hpp>

const float legendre_0_scalar = 1.f / 2.f * float(M_1_PI);
const float legendre_1_scalar = std::sqrt(3.f) / 2.f * float(M_1_PI);
const float legendre_2_scalar = std::sqrt(15.f) / 2.f * float(M_1_PI);
const float legendre_2_zz_scalar = std::sqrt(5.f) / 4.f * float(M_1_PI);


inline float legendre_0_0(const glm::vec3 &pos) { return  legendre_0_scalar; }

inline float legendre_1_0(const glm::vec3 &pos) { return -legendre_1_scalar * pos.y; }
inline float legendre_1_1(const glm::vec3 &pos) { return  legendre_1_scalar * pos.z; }
inline float legendre_1_2(const glm::vec3 &pos) { return -legendre_1_scalar * pos.x; }

inline float legendre_2_0(const glm::vec3 &pos) { return  legendre_2_scalar * pos.y * pos.x; }
inline float legendre_2_1(const glm::vec3 &pos) { return -legendre_2_scalar * pos.y * pos.z; }
inline float legendre_2_2(const glm::vec3 &pos) { return  legendre_2_zz_scalar * (3 * pos.z * pos.z - 1); }
inline float legendre_2_3(const glm::vec3 &pos) { return -legendre_2_scalar * pos.z * pos.x; }
inline float legendre_2_4(const glm::vec3 &pos) { return  legendre_2_scalar / 2.f * (pos.x*pos.x - pos.y*pos.y); }

inline float legendre_total(const glm::vec3 &pos, float *coefficients) {
    float total_scalar = legendre_0_0(pos) * coefficients[0] +
                         legendre_1_0(pos) * coefficients[1] +
                         legendre_1_1(pos) * coefficients[2] +
                         legendre_1_2(pos) * coefficients[3] +
                         legendre_2_0(pos) * coefficients[4] +
                         legendre_2_1(pos) * coefficients[5] +
                         legendre_2_2(pos) * coefficients[6] +
                         legendre_2_3(pos) * coefficients[7] +
                         legendre_2_4(pos) * coefficients[8];
    return total_scalar;
}

#endif //STUPIDSHTRICKS_LEGENDRE_H
