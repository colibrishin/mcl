#pragma once
/**
	@file
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
	@note for only 32bit not full bit prime version
	assert((p[N - 1] & 0x80000000) == 0);
*/
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

namespace mcl {

template<size_t N>
uint64_t addT(uint32_t z[N], const uint32_t x[N], const uint32_t y[N])
{
#if 1
	return mcl::bint::addT<N>(z, x, y);
#else
	uint64_t c = 0;
	for (size_t i = 0; i < N; i++) {
		uint64_t v = uint64_t(x[i]) + y[i] + c;
		z[i] = uint32_t(v);
		c = v >> 32;
	}
	return c;
#endif
}

template<size_t N>
uint64_t subT(uint32_t z[N], const uint32_t x[N], const uint32_t y[N])
{
#if 1
	return mcl::bint::subT<N>(z, x, y);
#else
	uint64_t c = 0;
	for (size_t i = 0; i < N; i++) {
		uint64_t v = uint64_t(x[i]) - y[i] - c;
		z[i] = uint32_t(v);
		c = v >> 63;
	}
	return c;
#endif
}

// [return:z[N]] = x[N] * y
template<size_t N>
uint64_t mulUnitT(uint32_t z[N], const uint32_t x[N], uint32_t y_)
{
#if 1
	return mcl::bint::mulUnitT<N>(z, x, y_);
#else
	uint64_t H = 0;
	uint64_t y = y_;
	for (size_t i = 0; i < N; i++) {
		uint64_t v = x[i] * y;
		v += H;
		z[i] = uint32_t(v);
		H = v >> 32;
	}
	return H;
#endif
}

// [return:z[N]] = z[N] + x[N] * uint32_t(y)
template<size_t N>
uint64_t addMulUnitT(uint32_t z[N], const uint32_t x[N], uint32_t y_)
{
#if 1
	return mcl::bint::mulUnitAddT<N>(z, x, y_);
#else
	// reduce cast operation
	uint64_t H = 0;
	uint64_t y = y_;
	for (size_t i = 0; i < N; i++) {
		uint64_t v = x[i] * y;
		v += H;
		v += z[i];
		z[i] = uint32_t(v);
		H = v >> 32;
	}
	return H;
#endif
}

// z[N * 2] = x[N] * y[N]
template<size_t N>
void mulPreT(uint32_t z[N * 2], const uint32_t x[N], const uint32_t y[N])
{
#if 1
	mcl::bint::mulT<N>(z, x, y);
#else
	z[N] = mulUnitT<N>(z, x, y[0]);
	for (size_t i = 1; i < N; i++) {
		z[N + i] = addMulUnitT<N>(&z[i], x, y[i]);
	}
#endif
}

#if 0
/*
	z[N * 2] = x[N] * y[N]
	H = N/2
	W = 1 << (H * 32)
	x = aW + b, y = cW + d
	assume a < W/2, c < W/2
	(aW + b)(cW + d) = acW^2 + (ad + bc)W + bd
	ad + bc = (a + b)(c + d) - ac - bd < (1 << (N * 32))
	slower than mulPreT on Core i7 with -m32 for N <= 12
*/
template<size_t N>
void karatsubaT(uint32_t z[N * 2], const uint32_t x[N], const uint32_t y[N])
{
	assert((N % 2) == 0);
	assert((x[N - 1] & 0x80000000) == 0);
	assert((y[N - 1] & 0x80000000) == 0);
	const size_t H = N / 2;
	uint32_t a_b[H];
	uint32_t c_d[H];
	uint64_t c1 = addT<H>(a_b, x, x + H); // a + b
	uint64_t c2 = addT<H>(c_d, y, y + H); // c + d
	uint32_t tmp[N];
	mulPreT<H>(tmp, a_b, c_d);
	if (c1) {
		addT<H>(tmp + H, tmp + H, c_d);
	}
	if (c2) {
		addT<H>(tmp + H, tmp + H, a_b);
	}
	mulPreT<H>(z, x, y); // bd
	mulPreT<H>(z + N, x + H, y + H); // ac
	// c:tmp[N] = (a + b)(c + d)
	subT<N>(tmp, tmp, z);
	subT<N>(tmp, tmp, z + N);
	// c:tmp[N] = ad + bc
	if (addT<N>(z + H, z + H, tmp)) {
		addUnitT<H>(z + N + H, 1);
	}
}
#endif

/*
	y[N * 2] = x[N] * x[N]
	(aW + b)^2 = a^2 W + b^2 + 2abW
	(a+b)^2 - a^2 - b^2
*/
template<size_t N>
void sqrT(uint32_t y[N * 2], const uint32_t x[N])
{
	assert((N % 2) == 0);
	assert((x[N - 1] & 0x80000000) == 0);
	const size_t H = N / 2;
	uint32_t a_b[H];
	uint64_t c = addT<H>(a_b, x, x + H); // a + b
	uint32_t tmp[N];
	mulPreT<H>(tmp, a_b, a_b);
	if (c) {
		bint::shlT<H>(a_b, a_b, 1);
		addT<H>(tmp + H, tmp + H, a_b);
	}
	mulPreT<H>(y, x, x); // b^2
	mulPreT<H>(y + N, x + H, x + H); // a^2
	// tmp[N] = (a + b)^2
	subT<N>(tmp, tmp, y);
	subT<N>(tmp, tmp, y + N);
	// tmp[N] = 2ab
	if (addT<N>(y + H, y + H, tmp)) {
		bint::addUnit(y + N + H, H, 1);
	}
}

template<size_t N>
void addModT(uint32_t z[N], const uint32_t x[N], const uint32_t y[N], const uint32_t p[N])
{
	uint32_t t[N];
	addT<N>(z, x, y);
	uint64_t c = subT<N>(t, z, p);
	if (!c) {
		bint::copyT<N>(z, t);
	}
}

template<size_t N>
void subModT(uint32_t z[N], const uint32_t x[N], const uint32_t y[N], const uint32_t p[N])
{
	uint64_t c = subT<N>(z, x, y);
	if (c) {
		addT<N>(z, z, p);
	}
}

/*
	z[N] = Montgomery(x[N], y[N], p[N])
	@remark : assume p[-1] = rp
*/
template<size_t N>
void mulMontT(uint32_t z[N], const uint32_t x[N], const uint32_t y[N], const uint32_t p[N])
{
	const uint32_t rp = p[-1];
	assert((p[N - 1] & 0x80000000) == 0);
	uint32_t buf[N * 2];
	buf[N] = mulUnitT<N>(buf, x, y[0]);
	uint32_t q = buf[0] * rp;
	buf[N] += addMulUnitT<N>(buf, p, q);
	for (size_t i = 1; i < N; i++) {
		buf[N + i] = addMulUnitT<N>(buf + i, x, y[i]);
		uint32_t q = buf[i] * rp;
		buf[N + i] += addMulUnitT<N>(buf + i, p, q);
	}
	if (subT<N>(z, buf + N, p)) {
		bint::copyT<N>(z, buf + N);
	}
}

// [return:z[N+1]] = z[N+1] + x[N] * y + (cc << (N * 32))
template<size_t N>
uint32_t addMulUnit2T(uint32_t z[N + 1], const uint32_t x[N], uint32_t y, const uint32_t *cc = 0)
{
	uint32_t H = 0;
	for (size_t i = 0; i < N; i++) {
		uint64_t v = uint64_t(x[i]) * y;
		v += H;
		v += z[i];
		z[i] = uint32_t(v);
		H = uint32_t(v >> 32);
	}
	if (cc) H += *cc;
	uint64_t v = uint64_t(z[N]);
	v += H;
	z[N] = uint32_t(v);
	return uint32_t(v >> 32);
}

/*
	z[N] = Montgomery reduction(y[N], xy[N], p[N])
	@remark : assume p[-1] = rp
*/
template<size_t N>
void modT(uint32_t y[N], const uint32_t xy[N * 2], const uint32_t p[N])
{
	const uint32_t rp = p[-1];
	assert((p[N - 1] & 0x80000000) == 0);
	uint32_t buf[N * 2];
	bint::copyT<N * 2>(buf, xy);
	uint32_t c = 0;
	for (size_t i = 0; i < N; i++) {
		uint32_t q = buf[i] * rp;
		c = addMulUnit2T<N>(buf + i, p, q, &c);
	}
	if (subT<N>(y, buf + N, p)) {
		bint::copyT<N>(y, buf + N);
	}
}

/*
	z[N] = Montgomery(x[N], y[N], p[N])
	@remark : assume p[-1] = rp
*/
template<size_t N>
void sqrMontT(uint32_t y[N], const uint32_t x[N], const uint32_t p[N])
{
#if 1
	mulMontT<N>(y, x, x, p);
#else
	// slower
	uint32_t xx[N * 2];
	sqrT<N>(xx, x);
	modT<N>(y, xx, p);
#endif
}

#if 0
template<size_t N>
void normalizeT(uint32_t y[N], const uint64_t x[N])
{
	uint64_t H = x[0];
	y[0] = H & 0xffffffff;
	for (size_t i = 1; i < N; i++) {
		uint64_t v = x[i] + (H >> 32);
		y[i] = uint32_t(v & 0xffffffff);
		H = v;
	}
}

template<size_t N, typename T>
int cmpT(const T *x, const T* y)
{
	for (size_t i = N - 1; i != size_t(-1); i--) {
		T a = x[i];
		T b = y[i];
		if (a != b) return a < b ? -1 : 1;
	}
	return 0;
}
#endif

} // mcl

