/*
 * matrix.h
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
#include <iostream>
#include <stdarg.h>

#ifndef matrix_h
#define matrix_h

namespace core
{

template <class t, int v, int h>
struct mat
{
	mat()
	{
	}

	template <int v2, int h2>
	mat(mat<t, v2, h2> m)
	{
		int mv = min(v, v2);
		int mh = min(h, h2);
		for (int i = 0; i < mv; i++)
		{
			for (int j = 0; j < mh; j++)
				data[i][j] = m.data[i][j];
			for (int j = mh; j < h2; j++)
				data[i][j] = 0;
		}
		for (int i = mv; i < v2; i++)
			for (int j = 0; j < h2; j++)
				data[i][j] = 0;
	}

	template <class t2>
	mat(t2 first, ...)
	{
		va_list arguments;

		va_start(arguments, first);
		data[0][0] = first;
		for (int i = 0; i < v; i++)
			for (int j = (i == 0 ? 1 : 0); j < h; j++)
				data[i][j] = (t)va_arg(arguments, t2);
		va_end(arguments);
	}

	mat(float first, ...)
	{
		va_list arguments;

		va_start(arguments, first);
		data[0][0] = first;
		for (int i = 0; i < v; i++)
			for (int j = (i == 0 ? 1 : 0); j < h; j++)
				data[i][j] = (t)va_arg(arguments, double);
		va_end(arguments);
	}

	~mat()
	{

	}

	template<class t2>
	operator mat<t2, v, h>()
	{
		mat<t2, v, h> result;
		for (int i = 0; i < v; i++)
			for (int j = 0; j < h; j++)
				result.data[i][j] = (t2)data[i][j];

		return result;
	}

	vec<t, h> data[v];

	template <int v2, int h2>
	mat<t, v, h> &operator=(mat<t, v2, h2> m)
	{
		int mv = min(v, v2);
		int mh = min(h, h2);
		for (int i = 0; i < mv; i++)
		{
			for (int j = 0; j < mh; j++)
				data[i][j] = m.data[i][j];
			for (int j = mh; j < h2; j++)
				data[i][j] = 0;
		}
		for (int i = mv; i < v2; i++)
			for (int j = 0; j < h2; j++)
				data[i][j] = 0;
		return *this;
	}

	template <class t2>
	mat<t, v, h> &operator+=(mat<t2, v, h> m)
	{
		*this = *this + m;
		return *this;
	}

	template <class t2>
	mat<t, v, h> &operator-=(mat<t2, v, h> m)
	{
		*this = *this - m;
		return *this;
	}

	template <class t2>
	mat<t, v, h> &operator*=(mat<t2, h, v> m)
	{
		*this = *this * m;
		return *this;
	}

	template <class t2>
	mat<t, v, h> &operator/=(mat<t2, h, v> m)
	{
		*this = *this / m;
		return *this;
	}

	mat<t, v, h> &operator+=(t f)
	{
		*this = *this + f;
		return *this;
	}

	mat<t, v, h> &operator-=(t f)
	{
		*this = *this - f;
		return *this;
	}

	mat<t, v, h> &operator*=(t f)
	{
		*this = *this * f;
		return *this;
	}

	mat<t, v, h> &operator/=(t f)
	{
		*this = *this / f;
		return *this;
	}

	vec<t, h> &operator[](int index)
	{
		return data[index];
	}

	vec<t, h> operator[](int index) const
	{
		return data[index];
	}

	void set_row(int r, vec<t, h> m)
	{
		data[r] = m;
	}

	vec<t, h> row(int r) const
	{
		vec<t, h> result;
		for (int i = 0; i < h; i++)
			result[i] = data[r][i];
		return result;
	}

	template <int s2>
	void set_row(int a, int b, mat<t, s2, h> m)
	{
		for (int i = a; i < b; i++)
			data[i] = m.data[i-a];
	}

	mat<t,v,h> row(int a, int b) const
	{
		mat<t, v, h> result;
		for (int i = a; i < b; i++)
			for (int j = 0; j < h; j++)
				result.data[i-a][j] = data[i][j];

		for (int i = b-a; i < v; i++)
			for (int j = 0; j < h; j++)
				result.data[i][j] = 0;

		return result;
	}

	void set_col(int c, vec<t, v> m)
	{
		for (int i = 0; i < v; i++)
			data[i][c] = m[i];
	}

	vec<t, v> col(int c) const
	{
		vec<t, v> result;

		for (int i = 0; i < v; i++)
			result[i] = data[i][c];

		return result;
	}

	template <int s2>
	void set_col(int a, int b, mat<t, s2, v> m)
	{
		for (int i = a; i < b; i++)
			for (int j = 0; j < v; j++)
				data[j][i] = m.data[i-a][j];
	}

	mat<t,v,h> col(int a, int b) const
	{
		mat<t, v, h> result;

		for (int i = 0; i < v; i++)
			for (int j = a; j < b; j++)
				result.data[i][j-a] = data[i][j];

		for (int i = 0; i < v; i++)
			for (int j = b-a; j < h; j++)
				result.data[i][j] = 0;

		return result;
	}

	template <int v2, int h2>
	void set(int va, int vb, int ha, int hb, mat<t, v2, h2> m)
	{
		for (int i = va; i < vb; i++)
			for (int j = ha; j < hb; j++)
				data[i][j] = m.data[i-va][j-ha];
	}

	mat<t, v, h> operator()(int va, int vb, int ha, int hb) const
	{
		mat<t, v, h> result;
		for (int i = va; i < vb; i++)
			for (int j = ha; j < hb; j++)
				result.data[i-va][j-ha] = data[i][j];

		for (int i = vb-va; i < v; i++)
			for (int j = 0; j < h; j++)
				result.data[i][j] = 0;

		for (int i = 0; i < v; i++)
			for (int j = hb-ha; j < h; j++)
				result.data[i][j] = 0;

		return result;
	}

	mat<t, v-1, h-1> remove(int y, int x)
	{
		mat<t, v-1, h-1> result;

		for (int i = 0; i < v-1; i++)
			for (int j = 0; j < h-1; j++)
				result[i][j] = data[i + (i >= y)][j + (j >= x)];

		return result;
	}

	mat<t, v, h> &swapr(int a, int b)
	{
		vec<t, h> temp = data[a];
		data[a] = data[b];
		data[b] = temp;
		return *this;
	}

	mat<t, v, h> &swapc(int a, int b)
	{
		t temp;
		for (int i = 0; i < h; i++)
		{
			temp = data[i].data[a];
			data[i].data[a] = data[i].data[b];
			data[i].data[b] = temp;
		}
		return *this;
	}
};

template <class t, int v, int h>
std::ostream &operator<<(std::ostream &f, mat<t, v, h> m)
{
	for (int i = 0; i < v; i++)
		f << m[i] << "\n";
	return f;
}

template <class t, int v, int h>
mat<t, v, h> operator-(mat<t, v, h> m)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = -m[i];

	return result;
}

template <class t1, class t2, int v, int h>
mat<t1, v, h> operator+(mat<t1, v, h> m1, mat<t2, v, h> m2)
{
	mat<t1, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = m1[i] + m2[i];

	return result;
}

template <class t1, class t2, int v, int h>
mat<t1, v, h> operator-(mat<t1, v, h> m1, mat<t2, v, h> m2)
{
	mat<t1, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = m1[i] - m2[i];

	return result;
}

template <class t1, class t2, int a, int b, int c>
mat<t1, a, c> operator*(mat<t1, a, b> m1, mat<t2, b, c> m2)
{
	mat<t1, a, c> result;

	for (int j = 0; j < a; j++)
		for (int i = 0; i < c; i++)
		{
			result[j][i] = 0;
			for (int k = 0; k < b; k++)
				result[j][i] += m1[j][k] * m2[k][i];
		}

	return result;
}

template <class t1, class t2, int v, int h>
mat<t1, v, h> operator/(mat<t1, v, h> v1, mat<t2, v, v> v2)
{
	return rref(v2)*v1;
}

template <class t, int v, int h>
mat<t, v, h> operator+(t f, mat<t, v, h> m)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = f + m[i];

	return result;
}

template <class t, int v, int h>
mat<t, v, h> operator-(t f, mat<t, v, h> m)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = f - m[i];

	return result;
}

template <class t, int v, int h>
mat<t, v, h> operator*(t f, mat<t, v, h> m)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = f * m[i];

	return result;
}

template <class t, int s>
mat<t, s, s> operator/(t f, mat<t, s, s> m)
{
	return rref(m)*f;
}

template <class t, int v, int h>
mat<t, v, h> operator+(mat<t, v, h> m, t f)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = m[i] + f;

	return result;
}

template <class t, int v, int h>
mat<t, v, h> operator-(mat<t, v, h> m, t f)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = m[i] - f;

	return result;
}

template <class t, int v, int h>
mat<t, v, h> operator*(mat<t, v, h> m, t f)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = m[i] * f;

	return result;
}

template <class t, int v, int h>
mat<t, v, h> operator/(mat<t, v, h> m, t f)
{
	mat<t, v, h> result;
	for (int i = 0; i < v; i++)
		result[i] = m[i] / f;

	return result;
}

template <class t1, class t2, int a, int b>
vec<t1, a> operator*(mat<t1, a, b> m1, vec<t2, b> v)
{
	vec<t1, a> result;
	int j, k;

	for (j = 0; j < a; j++)
	{
		result[j] = 0;
		for (k = 0; k < b; k++)
			result[j] += m1[j][k] * v[k];
	}

	return result;
}

template <class t1, class t2, int a, int b>
vec<t1, a> operator/(vec<t2, b> v, mat<t1, a, b> m1)
{
	return inverse(m1)*v;
}

/* trace
 *
 * Calculates the trace of the matrix m.
 */
template <class t, int s>
t trace(mat<t, s, s> m)
{
	t result;
	int i;

	for (i = 0; i < s; i++)
		result += m[i][i];

	return result;
}

/* determinant
 *
 * Calculates the determinant of an s x s matrix.
 */
template <class t, int s>
t determinant(mat<t, s, s> m)
{
	int i, j;
	t result = 0;
	for (i = 0, j = 1; i < s; i++, j*=-1)
		if (m[0][i] != 0)
			result += (t)j*m[0][i]*determinant(m.remove(0, i));
	return result;
}

/* determinant
 *
 * Calculates the determinant of a 1 x 1 matrix.
 */
template <class t>
t determinant(mat<t, 1, 1> m)
{
	return m[0][0];
}

/* determinant
 *
 * Calculates the determinant of a 2 x 2 matrix.
 */
template <class t>
t determinant(mat<t, 2, 2> m)
{
	return m[0][0]*m[1][1] - m[0][1]*m[1][0];
}

/* determinant
 *
 * Calculates the determinant of a 3 x 3 matrix.
 */
template <class t>
t determinant(mat<t, 3, 3> m)
{
	return m[0][0]*(m[1][1]*m[2][2] - m[1][2]*m[2][1]) +
		   m[0][1]*(m[1][2]*m[2][0] - m[1][0]*m[2][2]) +
		   m[0][2]*(m[1][0]*m[2][1] - m[1][1]*m[2][0]);
}

/* adjugate
 *
 * Calculates the adjugate matrix of the matrix m.
 */
template <class t, int s>
mat<t, s, s> adjugate(mat<t, s, s> m)
{
	mat<t, s, s> result;
	int i, j, k, l;

	for (i = 0, k = 1; i < s; i++, k*=-1)
		for (j = 0, l = 1; j < s; j++, l*=-1)
			result[i][j] = (t)(k*l)*determinant(m.remove(i, j));

	return result;
}

/* transpose
 *
 * Returns the transpose of the matrix m.
 */
template <class t, int v, int h>
mat<t, h, v> transpose(mat<t, v, h> m)
{
	mat<t, h, v> result;
	int i, j;

	for (i = 0; i < v; i++)
		for (j = 0; j < h; j++)
			result[i][j] = m[j][i];

	return result;
}

/* identity
 *
 * Returns a v,h-dimensional identity matrix.
 */
template <class t, int v, int h>
mat<t, v, h> identity()
{
	mat<t, v, h> result;

	for (int i = 0; i < v; i++)
		for (int j = 0; j < h; j++)
			result[i][j] = (i == j);

	return result;
}

/* rref
 *
 * returns the reduced row-echelon form of the matrix m.
 *
 * A matrix is in reduced row-echelon form (RREF) if it satisfies all of the following conditions.
 *  1. If a row has nonzero entries, then the first non-zero entry is 1 called the leading 1 in this row.
 *  2. If a column contains a leading one then all other entries in that column are zero.
 *  3. If a row contains a leading one the each row above contains a leading one further to the left.
 * The last point implies that if a matrix in rref has any zero rows they must appear as the
 * last rows of the matrix.
 */
template <class t, int v, int h>
mat<t, v, h> rref(mat<t, v, h> m)
{
	int i = 0, j = 0;
	while (i < v && j < h)
	{
		// If m_ij = 0 swap the i-th row with some other row below to guarantee that
		// m_ij != 0. The non-zero entry in the (i, j)-position is called a pivot.
		if (m[i][j] == 0)
		{
			for (int k = 0; k < v; k++)
				if (m[k][j] != 0)
				{
					m.swapr(i, k);
					k = v;
				}
		}

		// If all entries in the column are zero, increase j by 1.
		if (m[i][j] != 0)
		{
			// Divide the i-th row by m_ij to make the pivot entry = 1.
			m[i] /= m[i][j];

			// Eliminate all other entries in the j-th column by subtracting suitable
			// multiples of the i-th row from the other rows.
			for (int k = 0; k < v; k++)
				if (k != i)
					m[k] -= m[i]*m[k][j];

			// Increase i by 1 and j by 1 to choose the new pivot element. Return to Step 1.
			i++;
		}
		j++;
	}
	return m;
}

/* inverse
 *
 * Attempts to invert the matrix: This is not guaranteed to be correct.
 * If the matrix isn't invertible given the value type, then the matrix returned
 * won't be the inverse.
 *
 * A matrix is in reduced row-echelon form (RREF) if it satisfies all of the following conditions.
 *  1. If a row has nonzero entries, then the first non-zero entry is 1 called the leading 1 in this row.
 *  2. If a column contains a leading one then all other entries in that column are zero.
 *  3. If a row contains a leading one the each row above contains a leading one further to the left.
 * The last point implies that if a matrix in rref has any zero rows they must appear as the
 * last rows of the matrix.
 */
template <class t, int v, int h>
mat<t, v, h> inverse(mat<t, v, h> m)
{
	mat<t, v, h> result = identity<t, v, h>();
	int i = 0, j = 0;
	while (i < v && j < h)
	{
		// If m_ij = 0 swap the i-th row with some other row below to guarantee that
		// m_ij != 0. The non-zero entry in the (i, j)-position is called a pivot.
		if (m[i][j] == 0)
		{
			for (int k = 0; k < v; k++)
				if (m[k][j] != 0)
				{
					m.swapr(i, k);
					result.swapr(i, k);
					k = v;
				}
		}

		// If all entries in the column are zero, increase j by 1.
		if (m[i][j] != 0)
		{
			// Divide the i-th row by m_ij to make the pivot entry = 1.
			result[i] /= m[i][j];
			m[i] /= m[i][j];

			// Eliminate all other entries in the j-th column by subtracting suitable
			// multiples of the i-th row from the other rows.
			for (int k = 0; k < v; k++)
				if (k != i)
				{
					result[k] -= result[i]*m[k][j];
					m[k] -= m[i]*m[k][j];
				}

			// Increase i by 1 and j by 1 to choose the new pivot element. Return to Step 1.
			i++;
		}
		j++;
	}
	return result;
}

/* invertible
 *
 * Checks to see if a matrix is invertible
 *
 * A matrix m is invertible if it is square (v = h) and determinant(m) != 0
 */
template <class t, int v, int h>
bool invertible(mat<t, v, h> m)
{
	if (v != h)
		return false;

	if (determinant(m) == 0)
		return false;

	return true;
}

/* rank
 *
 * returns the number of linearly independent columns
 */
template <class t, int v, int h>
int rank(mat<t, v, h> m)
{
	int count = 0;
	mat<t, v, h> check = rref(m);
	for (int i = 0; i < v; i++)
		for (int j = 0; j < h; j++)
			if (check[i][j] != 0)
			{
				count++;
				j = h;
			}
	return count;
}

}

#endif
