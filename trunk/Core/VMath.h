// ---------------------------------------------------------------------------------------------------------------------------------
//                        _   _
//                       | | | |
// __   ___ __ ___   __ _| |_| |__
// \ \ / / '_ ` _ \ / _` | __| '_ \
//  \ V /| | | | | | (_| | |_| | | |
//   \_/ |_| |_| |_|\__,_|\__|_| |_|
//
// Description:
//
//   Generic 2-dimensional NxM matrix/vector mathematics class specialized for 3D usage
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   04/13/2001 by Paul Nettle: Original creation
//
// Restrictions & freedoms pertaining to usage and redistribution of this software:
//
//   This software is 100% free. If you use this software (in part or in whole) you must credit the author. This software may not be
//   re-distributed (in part or in whole) in a modified form without clear documentation on how to obtain a copy of the original
//   work. You may not use this software to directly or indirectly cause harm to others. This software is provided as-is and without
//   warrantee -- Use at your own risk. For more information, visit HTTP://www.FluidStudios.com/
//
// IMPORTANT:
//
//   Due to the nature of this class being a single solution for all 2D matrices of NxM size (which includes matrices, vectors &
//   points) there are two cases where the class's behavior does not make as much sense as you might like. These two cases are in
//   reference to the >> and ^ operators.
//
//   A multiplication operator is commonly used for component-wise multiplication, dot products and matrix concatenation. Since the
//   class supports the operators [+, -, /, *, +=, -=, /=, *=] it was prudent to maintain consistency and let operator* and
//   operator *= work as component-wise multiplication. So what operators should be used for dot products and concatenation? I have
//   chosen >> for concatenation, because it helps to serve as a reminder in which direction the operation's associativity goes
//   (from left-to-right). The dot product uses operator ^ (simply for lack of a better operator.)
//
//   The problem you may find then, is that operator >> and operator ^ have lower precedence than addition & subtraction. Without
//   the use of parenthesis around these multiplicative operations, you will not get the result you expect.
//
//   Furthermore, operator ^ (dot product) has a lower precedence than even operators [<, <=, >, >=]. This can cause prolems in
//   cases like: "if (v1 ^ v2 > 0)".
//
//   If you do not like the operators I have chosen, feel free to modify them. Or you may chose to perform these operations through
//   the function calls dot(), concat(), cross(), etc.
//
//   Consider yourself warned.
//
// Copyright 2001, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

#ifndef	_GEOM_VMATH
#define _GEOM_VMATH

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include <cmath>

#ifndef TemplateAssert
template <bool B> struct templateAssert {};
template<> struct templateAssert<true> {static void there_has_been_a_compile_time_assertion() {};};
#define	TemplateAssert(__a) {const bool __b = (__a) ? true:false; templateAssert<__b>::there_has_been_a_compile_time_assertion();}
#define	TemplateError(__msg) {const bool __b = (__a) ? true:false; templateAssert<__b>::##__msg();}
#endif // TemplateAssert

// ---------------------------------------------------------------------------------------------------------------------------------
// Define a matrix that is N columns wide and M rows tall. This matrix is row-major.
//
// N can be thought of as the [N]umber of elements per vector, and M can be thought of as the number of vectors in the matrix
// ---------------------------------------------------------------------------------------------------------------------------------

template <unsigned int N, unsigned int M, class T = float>
class	fsMatrix
{
public:
				// Default constructor

inline				fsMatrix() {}

				// Copy constructor

inline				fsMatrix(const fsMatrix & m)
				{
					//fstl::memcpy(_data, m._data, N*M);
					memcpy(_data, m._data, N*M * sizeof(T));
				}

				// Initialize with three vectors

inline				fsMatrix(const fsMatrix<N, 1, T> & xv, const fsMatrix<N, 1, T> & yv, const fsMatrix<N, 1, T> & zv)
				{
					setXVector(xv);
					setYVector(yv);
					setZVector(zv);
				}

				// Initialize with four values (useful for vectors)

inline				fsMatrix(const T & xv, const T & yv, const T & zv, const T & wv)
				{
					// This assertion guarantees that they are initializing the entire vector

					TemplateAssert(N == 4 && M == 1);

					x() = xv;
					y() = yv;
					z() = zv;
					w() = wv;
				}

				// Initialize with three values (useful for vectors)

inline				fsMatrix(const T & xv, const T & yv, const T & zv)
				{
					// This assertion guarantees that they are initializing the entire vector

					TemplateAssert(N == 3 && M == 1);

					x() = xv;
					y() = yv;
					z() = zv;
				}

				// Initialize with two values (useful for vectors)

inline				fsMatrix(const T & xv, const T & yv)
				{
					// This assertion guarantees that they are initializing the entire vector

					TemplateAssert(N == 2 && M == 1);

					x() = xv;
					y() = yv;
				}

				// These are handy 2D/3D/4D casts

inline				operator fsMatrix<2, 1, T>()
				{
					TemplateAssert(M == 1);

					fsMatrix<2, 1, T>	result;
					result.fill(0);
					
					int	c = N;
					if (c > 2) c = 2;

					for (int i = 0; i < c; i++) result(i,0) = (*this)(i,0);
					return result;
				}

inline				operator fsMatrix<3, 1, T>()
				{
					TemplateAssert(M == 1);

					fsMatrix<3, 1, T>	result;
					result.fill(0);
					
					int	c = N;
					if (c > 3) c = 3;

					for (int i = 0; i < c; i++) result(i,0) = (*this)(i,0);
					return result;
				}

inline				operator fsMatrix<4, 1, T>()
				{
					TemplateAssert(M == 1);

					fsMatrix<4, 1, T>	result;
					result.fill(0);
					result.w() = static_cast<T>(1);
					
					int	c = N;
					if (c > 4) c = 4;

					for (int i = 0; i < c; i++) result(i,0) = (*this)(i,0);
					return result;
				}

				// Return a zero'd matrix

static		fsMatrix		zero()
				{
					fsMatrix	result;
					result.fill(static_cast<T>(0));
					return result;
				}

				// The infamous 'operator='

inline		fsMatrix &	operator =(const fsMatrix & m)
				{
					//if (&m != this) fstl::memcpy(_data, m._data, N*M);
					if (&m != this) memcpy(_data, m._data, N*M * sizeof(T));
					return *this;
				}

				// Indexing: format = i down, j across

inline	const	T &		operator()(const unsigned int i, const unsigned int j) const
				{
					return _data[j*N+i];
				}

				// Indexing: format = i down, j across

inline		T &		operator()(const unsigned int i, const unsigned int j)
				{
					return _data[j*N+i];
				}

				// fsMatrix concatenation
				//
				// Specialized to follow the rules of matrix multiplication, for NxM * OxP:
				//   where M must be equal to O and resulting matrix is NxP. Otherwise, a
				//   compiler error will occur.
				//
				// Note that we use the >> operator. This is because of the lack of available
				//   operators, and also it serves as a reminder that the operations are from
				//   left-to-right (the convenient way.)

// I'm pretty sure this is wrong (I've seen it crash), so it's conditionally compiled out. Although, given the time, I would
// like to do things this way instad...
#if	0
template <unsigned int P>
inline	const	fsMatrix<N, P, T>	operator >>(const fsMatrix<M, P, T> & m) const	{return concat(m);}
template <unsigned int P>
inline	const	fsMatrix<N, P, T>	operator >>=(const fsMatrix<M, P, T> & m)		{*this = concat(m); return *this;}
template <unsigned int P>
inline	const	fsMatrix<N, P, T>	concat(const fsMatrix<M, P, T> & m) const
				{
					fsMatrix<N, P, T>	result;
					result.fill(static_cast<T>(0));

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < P; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}
#else
// We're doing matrix*vector, which, as far as I can tell, doesn't work the same as vector*vector. Though, it would work the same
// if it was vector*matrix... so this sucker is specialized for that purpose

inline	const	fsMatrix<N, 1, T>	operator >>(const fsMatrix<M, 1, T> & m) const	{return concat(m);}
inline	const	fsMatrix<N, 1, T>	operator >>=(const fsMatrix<M, 1, T> & m)		{*this = concat(m); return *this;}
inline	const	fsMatrix<N, 1, T>	concat(const fsMatrix<M, 1, T> & m) const
				{
					fsMatrix<N, 1, T>	result;
					result.fill(static_cast<T>(0));

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					result(i,0) += (*this)(j,i) * m(j,0);

					return result;
				}

inline	const	fsMatrix<N, 2, T>	operator >>(const fsMatrix<M, 2, T> & m) const	{return concat(m);}
inline	const	fsMatrix<N, 2, T>	operator >>=(const fsMatrix<M, 2, T> & m)		{*this = concat(m); return *this;}
inline	const	fsMatrix<N, 2, T>	concat(const fsMatrix<M, 2, T> & m) const
				{
					fsMatrix<N, 2, T>	result;
					result.fill(static_cast<T>(0));

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < 2; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}

inline	const	fsMatrix<N, 3, T>	operator >>(const fsMatrix<M, 3, T> & m) const	{return concat(m);}
inline	const	fsMatrix<N, 3, T>	operator >>=(const fsMatrix<M, 3, T> & m)		{*this = concat(m); return *this;}
inline	const	fsMatrix<N, 3, T>	concat(const fsMatrix<M, 3, T> & m) const
				{
					fsMatrix<N, 3, T>	result;
					result.fill(static_cast<T>(0));

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < 3; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}

inline	const	fsMatrix<N, 4, T>	operator >>(const fsMatrix<M, 4, T> & m) const	{return concat(m);}
inline	const	fsMatrix<N, 4, T>	operator >>=(const fsMatrix<M, 4, T> & m)		{*this = concat(m); return *this;}
inline	const	fsMatrix<N, 4, T>	concat(const fsMatrix<M, 4, T> & m) const
				{
					fsMatrix<N, 4, T>	result;
					result.fill(static_cast<T>(0));

					for (unsigned int i = 0; i < N; i++)
					for (unsigned int j = 0; j < M; j++)
					for (unsigned int k = 0; k < 4; k++)
					result(i,j) += (*this)(i,k) * m(k,j);

					return result;
				}
#endif
				// 3D Vector cross product
				//
				// Note that the cross product is specifically a 3-dimensional operation. So this routine
				// will fill the remaining elements (if any) with the contents of this->_data[...]
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline		void		cross(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(N >= 3);

					T	tx = y() * m.z() - z() * m.y();
					T	ty = z() * m.x() - x() * m.z();
					T	tz = x() * m.y() - y() * m.x();
					x() = tx;
					y() = ty;
					z() = tz;
				}

inline	const	fsMatrix<N, 1, T>	operator %(const fsMatrix<N, 1, T> & m) const
				{
					fsMatrix<N, 1, T>	result = *this;
					result.cross(m);
					return result;
				}

inline	const	fsMatrix		operator %=(const fsMatrix & m)
				{
					cross(m);
					return *this;
				}

				// Vector dot product
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline	const	T		dot(const fsMatrix<N, 1, T> & m) const
				{
					T	result = static_cast<T>(0);
					for (unsigned int i = 0; i < N; i++) result += _data[i] * m.data()[i];
					return result;
				}

inline	const	T		operator ^(const fsMatrix<N, 1, T> & m) const
				{
					return dot(m);
				}

				// Component-wise multiplication with matrix

inline	const	fsMatrix		operator *(const fsMatrix & m) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] * m._data[i];
					return result;
				}

				// Component-wise multiplication with scalar

inline	const	fsMatrix		operator *(const T & value) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] * value;
					return result;
				}

				// Component-wise multiplication with matrix (into self)

inline	const	fsMatrix		operator *=(const fsMatrix & m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] *= m._data[i];
					return *this;
				}

				// Component-wise multiplication with scalar (into self)

inline	const	fsMatrix		operator *=(const T & value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] *= value;
					return *this;
				}

				// Component-wise division with matrix

inline	const	fsMatrix		operator /(const fsMatrix & m) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] / m._data[i];
					return result;
				}

				// Component-wise division with scalar

inline	const	fsMatrix		operator /(const T & value) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] / value;
					return result;
				}

				// Component-wise division with scalar (scalar / component)

inline	const	fsMatrix		inverseDivide(const T & value) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = value / _data[i];
					return result;
				}

				// Component-wise division with matrix (into self)

inline	const	fsMatrix		operator /=(const fsMatrix & m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] /= m._data[i];
					return *this;
				}

				// Component-wise division with scalar (into self)

inline	const	fsMatrix		operator /=(const T & value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] /= value;
					return *this;
				}

				// Component-wise addition with matrix

inline	const	fsMatrix		operator +(const fsMatrix & m) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] + m._data[i];
					return result;
				}

				// Component-wise addition with scalar

inline	const	fsMatrix		operator +(const T & value) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] + value;
					return result;
				}

				// Component-wise addition with matrix (into self)

inline	const	fsMatrix		operator +=(const fsMatrix & m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] += m._data[i];
					return *this;
				}

				// Component-wise addition with scalar (into self)

inline	const	fsMatrix		operator +=(const T & value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] += value;
					return *this;
				}

				// Component-wise negation

inline	const	fsMatrix		operator -() const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = -_data[i];
					return result;
				}

				// Component-wise subtraction with matrix

inline	const	fsMatrix		operator -(const fsMatrix & m) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] - m._data[i];
					return result;
				}

				// Component-wise subtraction with scalar

inline	const	fsMatrix		operator -(const T & value) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = _data[i] - value;
					return result;
				}

				// Component-wise subtraction with scalar (scalar - component)

inline	const	fsMatrix		inverseSubtract(const T & value) const
				{
					fsMatrix	result;
					for (unsigned int i = 0; i < N*M; i++) result._data[i] = value - _data[i];
					return result;
				}

				// Component-wise subtraction with matrix (into self)

inline	const	fsMatrix		operator -=(const fsMatrix & m)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] -= m._data[i];
					return *this;
				}

				// Component-wise subtraction with scalar (into self)

inline	const	fsMatrix		operator -=(const T & value)
				{
					for (unsigned int i = 0; i < N*M; i++) _data[i] -= value;
					return *this;
				}

				// Total all components

inline		T		total()
				{
					T	tot = static_cast<T>(0);
					for (unsigned int i = 0; i < N*M; i++) tot += _data[i];
					return tot;
				}

				// Comparison for equality

inline	const	bool		operator ==(const fsMatrix & m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] != m._data[i]) return false;
					return true;
				}

				// Comparison for inequality

inline	const	bool		operator !=(const fsMatrix & m) const
				{
					return !(*this == m);
				}

inline	const	bool		operator <(const fsMatrix & m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] >= m._data[i]) return false;
					return true;
				}

inline	const	bool		operator <=(const fsMatrix & m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] > m._data[i]) return false;
					return true;
				}

inline	const	bool		operator >(const fsMatrix & m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] <= m._data[i]) return false;
					return true;
				}

inline	const	bool		operator >=(const fsMatrix & m) const
				{
					for (unsigned int i = 0; i < N*M; i++) if (_data[i] < m._data[i]) return false;
					return true;
				}

				// Generates a converted matrix. Result is a matrix that has been converted from one type to another.
				//
				// Offers two options:
				//
				// rowColumnSwap: if true, toggle between row-major and column-major
				// leftRightSwap: if true, toggle between left-handed and right-handed coordinate systems
				//
				// Default behaviour is a row-/column-major swap, but not a left-/right-handed swap.
				
inline		fsMatrix		genConvertedType(const bool rowColumnSwap = true, const bool leftRightSwap = false) const
				{
					TemplateAssert(N >= 3 && M >= 3);

					fsMatrix	result = *this;
					if (leftRightSwap)
					{
						for (unsigned int i = 2; i < N; i++)
						{
							result(i,2) = -result(i,2);
						}
					}

					if (rowColumnSwap)
					{
						result.transpose();
					}
					return result;
				}

				// Orthogonal transpose
				//
				// Note that matrix must be square (i.e. N == M)

inline		void		transpose()
				{
					TemplateAssert(N == M);

					// Transpose the matrix

					fsMatrix result;
					for (unsigned int j = 0; j < M; j++)
					{
						for (unsigned int i = 0; i < N; i++)
						{
							result(j,i) = (*this)(i,j);
						}
					}
					*this = result;
				}

				// Returns determinant of the matrix (4x4 only)

inline		T		determinant()
				{
					TemplateAssert(N == 4 && M == 4);

					fsMatrix &	m = *this;
					return	  (m(0,0) * m(1,1) - m(1,0) * m(0,1)) * (m(2,2) * m(3,3) - m(3,2) * m(2,3))
						- (m(0,0) * m(2,1) - m(2,0) * m(0,1)) * (m(1,2) * m(3,3) - m(3,2) * m(1,3))
						+ (m(0,0) * m(3,1) - m(3,0) * m(0,1)) * (m(1,2) * m(2,3) - m(2,2) * m(1,3))
						+ (m(1,0) * m(2,1) - m(2,0) * m(1,1)) * (m(0,2) * m(3,3) - m(3,2) * m(0,3))
						- (m(1,0) * m(3,1) - m(3,0) * m(1,1)) * (m(0,2) * m(2,3) - m(2,2) * m(0,3))
						+ (m(2,0) * m(3,1) - m(3,0) * m(2,1)) * (m(0,2) * m(1,3) - m(1,2) * m(0,3));
				}

				// Inverts the matrix (4x4 only)

inline		void		invert()
				{
					TemplateAssert(N == 4 && M == 4);

					T	d = determinant();
					if (d == 0.0) return;

					d = 1.0 / d;

					fsMatrix &	m = *this;
					fsMatrix		result;
					result(0,0) = d * (m(1,1) * (m(2,2) * m(3,3) - m(3,2) * m(2,3)) + m(2,1) * (m(3,2) * m(1,3) - m(1,2) * m(3,3)) + m(3,1) * (m(1,2) * m(2,3) - m(2,2) * m(1,3)));
					result(1,0) = d * (m(1,2) * (m(2,0) * m(3,3) - m(3,0) * m(2,3)) + m(2,2) * (m(3,0) * m(1,3) - m(1,0) * m(3,3)) + m(3,2) * (m(1,0) * m(2,3) - m(2,0) * m(1,3)));
					result(2,0) = d * (m(1,3) * (m(2,0) * m(3,1) - m(3,0) * m(2,1)) + m(2,3) * (m(3,0) * m(1,1) - m(1,0) * m(3,1)) + m(3,3) * (m(1,0) * m(2,1) - m(2,0) * m(1,1)));
					result(3,0) = d * (m(1,0) * (m(3,1) * m(2,2) - m(2,1) * m(3,2)) + m(2,0) * (m(1,1) * m(3,2) - m(3,1) * m(1,2)) + m(3,0) * (m(2,1) * m(1,2) - m(1,1) * m(2,2)));
					result(0,1) = d * (m(2,1) * (m(0,2) * m(3,3) - m(3,2) * m(0,3)) + m(3,1) * (m(2,2) * m(0,3) - m(0,2) * m(2,3)) + m(0,1) * (m(3,2) * m(2,3) - m(2,2) * m(3,3)));
					result(1,1) = d * (m(2,2) * (m(0,0) * m(3,3) - m(3,0) * m(0,3)) + m(3,2) * (m(2,0) * m(0,3) - m(0,0) * m(2,3)) + m(0,2) * (m(3,0) * m(2,3) - m(2,0) * m(3,3)));
					result(2,1) = d * (m(2,3) * (m(0,0) * m(3,1) - m(3,0) * m(0,1)) + m(3,3) * (m(2,0) * m(0,1) - m(0,0) * m(2,1)) + m(0,3) * (m(3,0) * m(2,1) - m(2,0) * m(3,1)));
					result(3,1) = d * (m(2,0) * (m(3,1) * m(0,2) - m(0,1) * m(3,2)) + m(3,0) * (m(0,1) * m(2,2) - m(2,1) * m(0,2)) + m(0,0) * (m(2,1) * m(3,2) - m(3,1) * m(2,2)));
					result(0,2) = d * (m(3,1) * (m(0,2) * m(1,3) - m(1,2) * m(0,3)) + m(0,1) * (m(1,2) * m(3,3) - m(3,2) * m(1,3)) + m(1,1) * (m(3,2) * m(0,3) - m(0,2) * m(3,3)));
					result(1,2) = d * (m(3,2) * (m(0,0) * m(1,3) - m(1,0) * m(0,3)) + m(0,2) * (m(1,0) * m(3,3) - m(3,0) * m(1,3)) + m(1,2) * (m(3,0) * m(0,3) - m(0,0) * m(3,3)));
					result(2,2) = d * (m(3,3) * (m(0,0) * m(1,1) - m(1,0) * m(0,1)) + m(0,3) * (m(1,0) * m(3,1) - m(3,0) * m(1,1)) + m(1,3) * (m(3,0) * m(0,1) - m(0,0) * m(3,1)));
					result(3,2) = d * (m(3,0) * (m(1,1) * m(0,2) - m(0,1) * m(1,2)) + m(0,0) * (m(3,1) * m(1,2) - m(1,1) * m(3,2)) + m(1,0) * (m(0,1) * m(3,2) - m(3,1) * m(0,2)));
					result(0,3) = d * (m(0,1) * (m(2,2) * m(1,3) - m(1,2) * m(2,3)) + m(1,1) * (m(0,2) * m(2,3) - m(2,2) * m(0,3)) + m(2,1) * (m(1,2) * m(0,3) - m(0,2) * m(1,3)));
					result(1,3) = d * (m(0,2) * (m(2,0) * m(1,3) - m(1,0) * m(2,3)) + m(1,2) * (m(0,0) * m(2,3) - m(2,0) * m(0,3)) + m(2,2) * (m(1,0) * m(0,3) - m(0,0) * m(1,3)));
					result(2,3) = d * (m(0,3) * (m(2,0) * m(1,1) - m(1,0) * m(2,1)) + m(1,3) * (m(0,0) * m(2,1) - m(2,0) * m(0,1)) + m(2,3) * (m(1,0) * m(0,1) - m(0,0) * m(1,1)));
					result(3,3) = d * (m(0,0) * (m(1,1) * m(2,2) - m(2,1) * m(1,2)) + m(1,0) * (m(2,1) * m(0,2) - m(0,1) * m(2,2)) + m(2,0) * (m(0,1) * m(1,2) - m(1,1) * m(0,2)));
					*this = result;
				}

				// Fill the matrix with a single value

inline		void		fill(const T & value)
				{
					T	*ptr = _data;
					for (unsigned int i = 0; i < N*M; i++, ptr++) *ptr = value;
				}

				// Generate identity matrix
				//
				// Note that matrix must be square (i.e. N == M)

static	const	fsMatrix		genIdentity()
				{
					TemplateAssert(N == M);

					// Make it identity

					fsMatrix		result;
					T		*ptr = result._data;
					for (unsigned int j = 0; j < M; j++)
					{
						for (unsigned int i = 0; i < N; i++, ptr++)
						{
							if (i == j)	*ptr = static_cast<T>(1);
							else		*ptr = static_cast<T>(0);
						}
					}
					return result;
				}

				// Generate rotation matrix (3x3) for rotation about the X axis (i.e. rotation happens along the Y/Z plane)
				//
				// Rotation happens in a counter-clockwise direction around the X vector

static	const	fsMatrix		genXRotation(const T & theta)
				{
					TemplateAssert(N >= 3 && M >= 3);

					// Start with identity

					fsMatrix	result = genIdentity();

					// Fill it in

					T	ct = static_cast<T>(cos(static_cast<double>(theta)));
					T	st = static_cast<T>(sin(static_cast<double>(theta)));
					result(1,1) =  ct;
					result(2,1) = -st;
					result(1,2) =  st;
					result(2,2) =  ct;
					return result;
				}

				// Generate rotation matrix (3x3) for rotation about the Y axis (i.e. rotation happens along the X/Z plane)
				//
				// Rotation happens in a counter-clockwise direction around the Y vector
				//
				// Note that the matrix must be a minimum of 3x3

static	const	fsMatrix		genYRotation(const T & theta)
				{
					TemplateAssert(N >= 3 && M >= 3);

					// Start with identity

					fsMatrix	result = genIdentity();

					// Fill it in

					T	ct = static_cast<T>(cos(static_cast<double>(theta)));
					T	st = static_cast<T>(sin(static_cast<double>(theta)));
					result(0,0) =  ct;
					result(2,0) =  st;
					result(0,2) = -st;
					result(2,2) =  ct;
					return result;
				}

				// Generate rotation matrix (3x3) for rotation about the Z axis (i.e. rotation happens along the X/Y plane)
				//
				// Rotation happens in a counter-clockwise direction around the Z vector
				//
				// Note that this matrix is allowed to be only 2x2 as this is a 2-D rotation

static	const	fsMatrix		genZRotation(const T & theta)
				{
					TemplateAssert(N >= 3 && M >= 3);

					// Start with identity

					fsMatrix	result = genIdentity();

					// Fill it in

					T	ct = static_cast<T>(cos(static_cast<double>(theta)));
					T	st = static_cast<T>(sin(static_cast<double>(theta)));
					result(0,0) =  ct;
					result(1,0) = -st;
					result(0,1) =  st;
					result(1,1) =  ct;
					return result;
				}

				// Generate a concatenated rotation matrix (3x3) for rotation about all axes (i.e. arbitrary rotation)
				//
				// Rotation happens in a counter-clockwise direction around each vector
				//
				// Rotation happens in the following order: First Z, then Y and finally X.

static	const	fsMatrix		genRotation(const T & xTheta, const T & yTheta, const T & zTheta)
				{
					return genZRotation(zTheta) >> genYRotation(yTheta) >> genXRotation(xTheta);
				}

				// Generate a 'look-at' matrix. Must be a 3x3 result because this routine uses a cross product

static	const	fsMatrix<3, 3, T>	genLookat(const fsMatrix<3, 1, T> & v, const T & theta = static_cast<T>(0))
				{
					fsMatrix<3, 1, T>	zAxis = v;
					zAxis.normalize();

					fsMatrix<3, 1, T>	yAxis;
					yAxis.fill(static_cast<T>(0));

					// Handle the degenerate case... (this acts exactly like 3DS-R4)

					if (!zAxis.x() && !zAxis.z())	yAxis.z() = -zAxis.y();
					else				yAxis.y() = static_cast<T>(1);

					fsMatrix<3, 1, T>	xAxis = yAxis % zAxis;
					xAxis.normalize();

					yAxis = xAxis % zAxis;
					yAxis.normalize();
					yAxis = -yAxis;

					fsMatrix<3, 3, T>	m(xAxis, yAxis, zAxis);
					return m >> genZRotation(theta);
				}

				// Scale a matrix

inline		void		scale(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(N <= M);

					for (unsigned int i = 0; i < N; i++)
					{
						(*this)(i,i) *= m(i,0);
					}
				}

				// Generate a scale matrix

static		fsMatrix		genScale(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(N <= M);

					fsMatrix	result;
					result = genIdentity();
					for (unsigned int i = 0; i < N; i++)
					{
						result(i,i) *= m(i,0);
					}
					return result;
				}

				// Generate a translation matrix

static		fsMatrix		genTranslate(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(M <= N);

					fsMatrix	result;
					result = genIdentity();
					for (unsigned int i = 0; i < M; i++)
					{
						result(N-1,i) += m(i,0);
					}
					return result;
				}

				// Generate a shear matrix

static		fsMatrix		genShear(const T x, const T y)
				{
					TemplateAssert(N > 1 && M > 1);

					fsMatrix	result;
					result = genIdentity();
					result(1,0) = x;
					result(0,1) = y;
					return result;
				}

				// Generate a (4x4) perspective projection matrix (as per D3D)

static	const	fsMatrix<4, 4, T>	genProjectPerspectiveD3D(const T & fov, const T & aspect, const T & n, const T & f)
				{
					T	w  = static_cast<T>(1.0 / tan(fov / static_cast<T>(2)));
					T	h  = static_cast<T>(1.0 / tan(fov / static_cast<T>(2)));
					if (aspect > 1.0)	w /= aspect;
					else			h *= aspect;
					T	q  = f / (f - n);

					fsMatrix<4, 4, T>	result;
					result.fill(static_cast<T>(0));
					result(0,0) = w;
					result(1,1) = h;
					result(2,2) = q;
					result(3,2) = -q*n;
					result(2,3) = 1;
					return result;
				}

				// Generate a (4x4) perspective projection matrix (as per Blinn)

static	const	fsMatrix<4, 4, T>	genProjectPerspectiveBlinn(const T & fov, const T & aspect, const T & n, const T & f)
				{
					T	w  = static_cast<T>(cos(fov / static_cast<T>(2)));
					T	h  = static_cast<T>(cos(fov / static_cast<T>(2)));
					if (aspect > 1.0)	w /= aspect;
					else			h *= aspect;
					T	s  = static_cast<T>(sin(fov / static_cast<T>(2)));
					T	d  = static_cast<T>(1) - n/f;

					fsMatrix<4, 4, T>	result;
					result.fill(static_cast<T>(0));
					result(0,0) = w;
					result(1,1) = h;
					result(2,2) = s / d;
					result(3,2) = -(s * n / d);
					result(2,3) = s;
					return result;
				}

				// Generate a (4x4) perspective projection matrix (as per glFrustum)

static	const	fsMatrix<4, 4, T>	genProjectPerspectiveGlFrustum(const T & l, const T & r, const T & b, const T & t, const T & n, const T & f)
				{
					fsMatrix<4, 4, T>	result;
					result.fill(static_cast<T>(0));
					result(0,0) = (2*n)/(r-l);
					result(2,0) = (r+l)/(r-l);
					result(1,1) = (2*n)/(t-b);
					result(2,1) = (t+b)/(t-b);
					result(2,2) = (-(f+n))/(f-n);
					result(3,2) = (-2*f*n)/(f-n);
					result(2,3) = -1;
					return result;
				}

				// Generate a (4x4) orthogonal projection matrix (as per glOrtho)

static	const	fsMatrix<4, 4, T>	genProjectGlOrtho(const T & l, const T & r, const T & b, const T & t, const T & n, const T & f)
				{
					fsMatrix<4, 4, T>	result;
					result.fill(static_cast<T>(0));
					result(0,0) =  2/(r-l);
					result(1,1) =  2/(t-b);
					result(2,2) = -2/(f-n);
					result(3,3) =  1;
					result(3,0) = -((r+l)/(r-l));
					result(3,1) = -((t+b)/(t-b));
					result(3,2) = -((f+n)/(f-n));
					return result;
				}

				// Generate a (4x4) orthogonal projection matrix (standard -- maps to z=0 plane)

static	const	fsMatrix<4, 4, T>	genProjectScaleOrtho(const T & xScale, const T & yScale)
				{
					fsMatrix<4, 4, T>	result;
					result.fill(static_cast<T>(0));
					result(0,0) = xScale;
					result(1,1) = yScale;
					result(3,3) = 1;
					return result;
				}

				// Generate a (4x4) orthogonal projection matrix (translates to screen coordinates, maps to z=0 plane)

static	const	fsMatrix<4, 4, T>	genProjectScreenOrtho(const T & width, const T & height)
				{
					fsMatrix<4, 4, T>	result;
					result.fill(static_cast<T>(0));
					result(0,0) =  2 / width;
					result(1,1) = -2 / height;
					result(3,3) =  1;
					result(3,0) = -1;
					result(3,1) =  1;
					return result;
				}

				// Extract the X vector from the matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		fsMatrix<N, 1, T>	extractXVector() const
				{
					TemplateAssert(M > 0);

					fsMatrix<N, 1, T>	result;
					for (unsigned int i = 0; i < N; i++)
					{
						result(i,0) = (*this)(i,0);
					}
					return result;
				}

				// Extract the Y vector from the matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		fsMatrix<N, 1, T>	extractYVector() const
				{
					TemplateAssert(M > 1);

					fsMatrix<N, 1, T>	result;
					for (unsigned int i = 0; i < N; i++)
					{
						result(i,0) = (*this)(i,1);
					}
					return result;
				}

				// Extract the Z vector from the matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		fsMatrix<N, 1, T>	extractZVector() const
				{
					TemplateAssert(M > 2);

					fsMatrix<N, 1, T>	result;
					for (unsigned int i = 0; i < N; i++)
					{
						result(i,0) = (*this)(i,2);
					}
					return result;
				}

				// Extract the W vector from the matrix
				//
				// Note that the matrix must be a minimum of 4x4

inline		fsMatrix<N, 1, T>	extractWVector() const
				{
					TemplateAssert(M > 3);

					fsMatrix<N, 1, T>	result;
					for (unsigned int i = 0; i < N; i++)
					{
						result(i,0) = (*this)(i,3);
					}
					return result;
				}

				// Extract the translation from the matrix

inline		fsMatrix<N,1, T>	extractTranslation() const
				{
					TemplateAssert(M <= N);

					fsMatrix<N, 1, T>	result;
					for (unsigned int i = 0; i < M; i++)
					{
						result(i,0) = (*this)(N-1,i);
					}
					return result;
				}

				// Replace the X vector within matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		void		setXVector(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(N > 0);

					for (unsigned int i = 0; i < N; i++)
					{
						(*this)(i,0) = m(i,0);
					}
				}

				// Replace the Y vector within matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		void		setYVector(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(N > 1);

					for (unsigned int i = 0; i < N; i++)
					{
						(*this)(i,1) = m(i,0);
					}
				}

				// Replace the Z vector within matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		void		setZVector(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(N > 2);

					for (unsigned int i = 0; i < N; i++)
					{
						(*this)(i,2) = m(i,0);
					}
				}
				
				// Replace the W vector within matrix
				//
				// Note that the matrix must be a minimum of 3x3

inline		void		setWVector(const fsMatrix<N, 1, T> & m)
				{
					TemplateAssert(N > 3);

					for (unsigned int i = 0; i < N; i++)
					{
						(*this)(i,3) = m(i,0);
					}
				}

				// Vector length calculation (squared)
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline	const	T		lengthSquared() const
				{
					TemplateAssert(M == 1);

					return dot(*this);
				}

				// Vector length calculation
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline	const	T		length() const
				{
					TemplateAssert(M == 1);

					return static_cast<T>(sqrt(lengthSquared()));
				}

				// Vector length
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline		void		setLength(const T & len)
				{
					TemplateAssert(M == 1);

					T	l = len / length();

					for (unsigned int i = 0; i < N; ++i)
					{
						data()[i] *= l;
					}
				}

				// Vector length
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline		T		distance(const fsMatrix & m) const
				{
					TemplateAssert(M == 1);

					fsMatrix	temp = *this - m;
					return temp.length();
				}

				// Normalize
				//
				// Specialized for Nx1 matrices (i.e. vectors)

inline		void		normalize()
				{
					setLength(static_cast<T>(1));
				}

				// Normalize an orthogonal matrix (i.e. make sure the internal vectors are all perpendicular)
				//
				// Note that the matrix must be a 3x3, due to the fact that this routine uses a cross product

inline		void		orthoNormalize()
				{
					TemplateAssert(N == 3 && M == 3);

					fsMatrix<N, 1, T>	xVector = extractXVector();
					fsMatrix<N, 1, T>	yVector = extractYVector();
					fsMatrix<N, 1, T>	zVector = extractZVector();

					xVector -= yVector * (xVector * yVector);
					xVector -= zVector * (xVector * zVector);
					xVector.normalize();

					yVector -= xVector * (yVector * xVector);
					yVector -= zVector * (yVector * zVector);
					yVector.normalize();

					zVector = xVector % yVector;

					setXVector(xVector);
					setYVector(yVector);
					setZVector(zVector);
				}

				// Absolute value of all components

inline		void		abs()
				{
					for (unsigned int i = 0; i < N*M; i++)
						_data[i] = static_cast<T>(fabs(static_cast<double>(_data[i])));
				}

				// Saturate all components

inline		void		saturate(const T & min, const T & max)
				{
					for (unsigned int i = 0; i < N*M; i++)
					{
						if (_data[i] < min) _data[i] = min;
						if (_data[i] > max) _data[i] = max;
					}
				}

				// Specialized 'convenience accessors' for vectors, points, etc.
				//
				// Note that the matrix must have a value of N large enough to store the value
				// in question and M must always be 1.

inline	const	T &		x() const {TemplateAssert(N > 0 && M == 1); return _data[0];}
inline		T &		x()       {TemplateAssert(N > 0 && M == 1); return _data[0];}
inline	const	T &		y() const {TemplateAssert(N > 1 && M == 1); return _data[1];}
inline		T &		y()       {TemplateAssert(N > 1 && M == 1); return _data[1];}
inline	const	T &		z() const {TemplateAssert(N > 2 && M == 1); return _data[2];}
inline		T &		z()       {TemplateAssert(N > 2 && M == 1); return _data[2];}
inline	const	T &		w() const {TemplateAssert(N > 3 && M == 1); return _data[3];}
inline		T &		w()       {TemplateAssert(N > 3 && M == 1); return _data[3];}

				// UVs (simply a convenience thing)

inline	const	T &		u() const {return x();}
inline		T &		u()       {return x();}
inline	const	T &		v() const {return y();}
inline		T &		v()       {return y();}

				// For use with colors (simply a convenience thing)

inline	const	T &		r() const {return x();}
inline		T &		r()       {return x();}
inline	const	T &		g() const {return y();}
inline		T &		g()       {return y();}
inline	const	T &		b() const {return z();}
inline		T &		b()       {return z();}
inline	const	T &		a() const {return w();}
inline		T &		a()       {return w();}
inline	const	unsigned int	rgb() const
				{
					TemplateAssert(N > 2 && M == 1);
					unsigned int	nr = static_cast<unsigned int>(r() * static_cast<T>(255));
					unsigned int	ng = static_cast<unsigned int>(g() * static_cast<T>(255));
					unsigned int	nb = static_cast<unsigned int>(b() * static_cast<T>(255));
					return (nr << 16) | (ng << 8) | nb;
				}
inline	const	unsigned int	argb() const
				{
					TemplateAssert(N > 3 && M == 1);
					unsigned int	na = static_cast<unsigned int>(a() * static_cast<T>(255));
					unsigned int	nr = static_cast<unsigned int>(r() * static_cast<T>(255));
					unsigned int	ng = static_cast<unsigned int>(g() * static_cast<T>(255));
					unsigned int	nb = static_cast<unsigned int>(b() * static_cast<T>(255));
					return (na << 24) | (nr << 16) | (ng << 8) | nb;
				}
inline	const	unsigned int	bgr() const
				{
					TemplateAssert(N > 2 && M == 1);
					unsigned int	nr = static_cast<unsigned int>(r() * static_cast<T>(255));
					unsigned int	ng = static_cast<unsigned int>(g() * static_cast<T>(255));
					unsigned int	nb = static_cast<unsigned int>(b() * static_cast<T>(255));
					return (nb << 16) | (ng << 8) | nr;
				}
inline	const	unsigned int	abgr() const
				{
					TemplateAssert(N > 3 && M == 1);
					unsigned int	na = static_cast<unsigned int>(a() * static_cast<T>(255));
					unsigned int	nr = static_cast<unsigned int>(r() * static_cast<T>(255));
					unsigned int	ng = static_cast<unsigned int>(g() * static_cast<T>(255));
					unsigned int	nb = static_cast<unsigned int>(b() * static_cast<T>(255));
					return (na << 24) | (nb << 16) | (ng << 8) | nr;
				}

				// Polar & spherical coordinates (simply a convenience thing)

inline	const	T &		phi() const   {return x();}	// phi = rotational distance from the axis (like lattitude)
inline		T &		phi()         {return x();}
inline	const	T &		theta() const {return y();}	// theta = rotation around the axis (like longitude)
inline		T &		theta()       {return y();}
inline	const	T &		rho() const   {return z();}	// rho = distance from origin
inline		T &		rho()         {return z();}

				// Only use this if you need to... prefer operator(int,int) for access

inline	const	T		*data() const {return _data;}
inline		T		*data()       {return _data;}

				// Dimensions

inline		unsigned int	width() const {return N;}
inline		unsigned int	height() const {return M;}

				// Debugging functions

#ifdef _MSC_VER
inline		void		debugTrace() const
				{
					for (unsigned int i = 0; i < N; i++)
					{
						char	temp[90];
						strcpy(temp, "[");
						for (unsigned int j = 0; j < M; j++)
						{
							char	t[90];
							sprintf(t, "%12.5f", (*this)(i,j));
							strcat(temp, t);
						}
						strcat(temp, " ]\n");
						TRACE(temp);
					}
					TRACE("\n");
				}
#endif

#ifdef _H_LOGGER
inline		void		debugLog(const char *title) const
				{
					LOGBLOCK(title);
					for (unsigned int i = 0; i < N; i++)
					{
						char	temp[90];
						strcpy(temp, "[");
						for (unsigned int j = 0; j < M; j++)
						{
							char	t[90];
							sprintf(t, "%12.5f", (*this)(i,j));
							strcat(temp, t);
						}
						strcat(temp, " ]");
						LOG(temp);
					}
				}
#endif

private:
		T		_data[N*M];
};

// ---------------------------------------------------------------------------------------------------------------------------------
// Convenience types - Most common uses
// ---------------------------------------------------------------------------------------------------------------------------------

typedef	fsMatrix<3, 3> fsMatrix3;
typedef	fsMatrix<4, 4> fsMatrix4;
typedef	fsMatrix<2, 1> fsVector2;
typedef	fsMatrix<3, 1> fsVector3;
typedef	fsMatrix<4, 1> fsVector4;
typedef	fsMatrix<2, 1> fsPoint2;
typedef	fsMatrix<3, 1> fsPoint3;
typedef	fsMatrix<4, 1> fsPoint4;
typedef	fsMatrix<2, 1> fsColor2;
typedef	fsMatrix<3, 1> fsColor3;
typedef	fsMatrix<4, 1> fsColor4;

// ---------------------------------------------------------------------------------------------------------------------------------
// Mixed-mode global overrides
// ---------------------------------------------------------------------------------------------------------------------------------

template <unsigned int N, unsigned int M, class T>
inline	const	fsMatrix<N, M, T>	operator *(const T & value, const fsMatrix<N, M, T> & m) {return m * value;}

template <unsigned int N, unsigned int M, class T>
inline	const	fsMatrix<N, M, T>	operator /(const T & value, const fsMatrix<N, M, T> & m) {return m.inverseDivide(value);}

template <unsigned int N, unsigned int M, class T>
inline	const	fsMatrix<N, M, T>	operator +(const T & value, const fsMatrix<N, M, T> & m) {return m + value;}

template <unsigned int N, unsigned int M, class T>
inline	const	fsMatrix<N, M, T>	operator -(const T & value, const fsMatrix<N, M, T> & m) {return m.inverseSubtract(value);}

#endif // _GEOM_VMATH
// ---------------------------------------------------------------------------------------------------------------------------------
// vmath - End of file
// ---------------------------------------------------------------------------------------------------------------------------------

