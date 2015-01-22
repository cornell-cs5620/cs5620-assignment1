/*
 * geometry.h
 * Blaze Game Engine v0.11
 *
 *  Created on: Jan 20, 2011
 *      Author: Ned Bingham
 *
 * This file is part of corelib.
 *
 * corelib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * corelib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with corelib.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vector.h"
#include "matrix.h"

#ifndef geometry_h
#define geometry_h

namespace core
{
typedef vec<float,  1>	vec1f;
typedef vec<float,  2>	vec2f;
typedef vec<float,  3>	vec3f;
typedef vec<float,  4>	vec4f;
typedef vec<float,  5>	vec5f;
typedef vec<float,  6>	vec6f;
typedef vec<float,  7>	vec7f;
typedef vec<float,  8>	vec8f;
typedef vec<float,  9>	vec9f;
typedef vec<float,  10>	vec10f;
typedef vec<float,  11>	vec11f;
typedef vec<float,  12>	vec12f;

typedef vec<int,    1>	vec1i;
typedef vec<int,    2>	vec2i;
typedef vec<int,    3>	vec3i;
typedef vec<int,    4>	vec4i;
typedef vec<int,    5>	vec5i;
typedef vec<int,    6>	vec6i;
typedef vec<int,    7>	vec7i;
typedef vec<int,    8>	vec8i;
typedef vec<int,    9>	vec9i;
typedef vec<int,    10>	vec10i;
typedef vec<int,    11>	vec11i;
typedef vec<int,    12>	vec12i;

typedef mat<float,  1, 1> mat1f;
typedef mat<float,  2, 2> mat2f;
typedef mat<float,  3, 3> mat3f;
typedef mat<float,  4, 4> mat4f;
typedef mat<float,  5, 5> mat5f;

typedef mat<int,  1, 1>	mat1i;
typedef mat<int,  2, 2>	mat2i;
typedef mat<int,  3, 3>	mat3i;
typedef mat<int,  4, 4>	mat4i;
typedef mat<int,  5, 5>	mat5i;

}

#endif
