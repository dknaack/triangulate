/*
 * triangulate - single header polygon triangulation library
 * 
 * This is free and unencumbered software released into the public domain.
 */

#ifndef TRIANGULATE_H
#define TRIANGULATE_H

/* 
 * Triangulate a simple polygon given by `points` into triangles stored in
 * `indices`.
 * 
 * The `points` array must have a size of at least `2 * npoints` with the first
 * element representing the x coordinate and the second the y coordinate.
 * 
 * The `indices` array must have a size of at least `3 * npoints`. Although
 * only `npoints - 2` triangles are generated, this library still requires the
 * indices array to have a size of at least `3 * npoints`.
 *
 * The function returns the number of triangles generated.
 * This should always be `3 * (npoints - 2)`.
 */
int triangulate(const float *points, int npoints, unsigned int *indices);

#endif /* TRIANGULATE_H */

#ifdef TRIANGULATE_IMPLEMENTATION
struct tristate {
	const float *points;
	unsigned int *indices;
	int npoints, r, c, e;
};

static int
triconvex(struct tristate *s, int i)
{
	float x, y, x1, y1, x2, y2;

	x = s->points[2 * s->indices[3 * i + 1] + 0];
	y = s->points[2 * s->indices[3 * i + 1] + 1];

	x1 = s->points[2 * s->indices[3 * i + 0] + 0] - x;
	y1 = s->points[2 * s->indices[3 * i + 0] + 1] - y;
	x2 = s->points[2 * s->indices[3 * i + 2] + 0] - x;
	y2 = s->points[2 * s->indices[3 * i + 2] + 1] - y;

	return x2 * y1 - x1 * y2 > 0;
}

static int
triinside(const float *p, const float *a, const float *b, const float *c)
{
	float d00, d01, d02, d11, d12, inv, u, v;
	float v0[2], v1[2], v2[2];

	v0[0] = c[0] - a[0], v0[1] = c[1] - a[1];
	v1[0] = b[0] - a[0], v1[1] = b[1] - a[1];
	v2[0] = p[0] - a[0], v2[1] = p[1] - a[1];

	d00 = v0[0] * v0[0] + v0[1] * v0[1];
	d01 = v0[0] * v1[0] + v0[1] * v1[1];
	d02 = v0[0] * v2[0] + v0[1] * v2[1];
	d11 = v1[0] * v1[0] + v1[1] * v1[1];
	d12 = v1[0] * v2[0] + v1[1] * v2[1];

	inv = 1.f / (d00 * d11 - d01 * d01);
	u = (d11 * d02 - d01 * d12) * inv;
	v = (d00 * d12 - d01 * d02) * inv;

	return u >= 0 && v >= 0 && u + v < 1;
}

static int
triempty(struct tristate *s, int i)
{
	unsigned int *j, ia, ib, ic;
	const float *a, *b, *c;

	ia = s->indices[3 * i + 0];
	ib = s->indices[3 * i + 1];
	ic = s->indices[3 * i + 2];

	a = &s->points[2 * ia];
	b = &s->points[2 * ib];
	c = &s->points[2 * ic];

	for (j = s->indices + 3 * s->r; j - s->indices < 3 * s->c; j += 3)
		if (j[1] == ia || j[1] == ib || j[1] == ic)
			/* ignore vertices of the triangle */;
		else if (triinside(&s->points[2 * j[1]], a, b, c))
			return 0;
	return 1;
}

static void
tricpy(unsigned int *dst, const unsigned int *src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

static void
triswap(unsigned int *a, unsigned int *b)
{
	unsigned int tmp[3];

	tricpy(tmp, a);
	tricpy(a, b);
	tricpy(b, tmp);
}

static int
tripartition(struct tristate *s, int l, int (*f)(struct tristate *, int))
{
	int i;

	for (i = l; i < s->npoints; i++)
		if (!f(s, i))
			triswap(&s->indices[3 * i], &s->indices[3 * l++]);
	return l;
}

static void
triupdate(struct tristate *s, unsigned int *ear, int i)
{
	int j;

	/* find the vertex */
	for (j = s->r; j < s->npoints; j++)
		if (s->indices[3 * j + 1] == ear[i])
			break;

	/* update the adjacent vertex */
	s->indices[3 * j + 2 - i] = ear[2 - i];

	/* move the vertex if neccessary */
	if (j < s->c) {
		if (!triconvex(s, j))
			return;
		triswap(s->indices + 3 * (--s->c), s->indices + 3 * j);
		if (triempty(s, s->c))
			triswap(s->indices + 3 * (--s->e), s->indices + 3 * s->c);
	} else if (j < s->e) {
		if (triempty(s, j))
			triswap(s->indices + 3 * (--s->e), s->indices + 3 * j);
	} else {
		if (triconvex(s, j) && triempty(s, j))
			return;
		triswap(s->indices + 3 * s->e++, s->indices + 3 * j);
		if (!triconvex(s, s->e - 1))
			triswap(s->indices + 3 * s->c++, s->indices + 3 * s->e - 1);
	}
}

int
triangulate(const float *points, int npoints, unsigned int *indices)
{
	unsigned int ear[3];
	struct tristate s;
	int i;

	if (npoints <= 0)
		return 0;
	if (!points || !indices)
		return 3 * npoints;

	s.npoints = npoints;
	s.points  = points;
	s.indices = indices;

	for (i = 0; i < npoints; i++) {
		s.indices[3 * i + 0] = (i - 1 + npoints) % npoints;
		s.indices[3 * i + 1] = i;
		s.indices[3 * i + 2] = (i + 1) % npoints;
	}

	s.r = 0;
	s.c = tripartition(&s, s.r, triconvex);
	s.e = tripartition(&s, s.c, triempty);

	while (s.r < npoints - 3) {
		/* remove an ear */
		tricpy(ear, s.indices + 3 * s.e);
		tricpy(s.indices + 3 * s.e++, s.indices + 3 * s.c);
		tricpy(s.indices + 3 * s.c++, s.indices + 3 * s.r);
		tricpy(s.indices + 3 * s.r++, ear);

		/* update adjacent vertices */
		triupdate(&s, ear, 0);
		triupdate(&s, ear, 2);
	}

	return s.r + 1;
}
#endif /* TRIANGULATE_IMPLEMENTATION */
