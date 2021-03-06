#ifndef _DL_MATH_H_
#define _DL_MATH_H_

#include "d3dx9math.h"

//-----------------------------------------------------------------------------

struct BoundingSphere {
	D3DXVECTOR3 center;
	float radius;

	BoundingSphere();
	BoundingSphere( const BoundingSphere & sphere );
	BoundingSphere & operator=( const BoundingSphere & sphere );

	~BoundingSphere();
	BoundingSphere & operator+=( const BoundingSphere & rh );
	BoundingSphere operator+( const BoundingSphere & rh );
};

//-----------------------------------------------------------------------------

struct BoundingBox {
	D3DXVECTOR3 corners[8];

	BoundingBox();
	BoundingBox( const BoundingBox & rh );
	~BoundingBox();
	BoundingBox& operator=( const BoundingBox & rh );
	BoundingBox( const D3DXVECTOR3 & min, const D3DXVECTOR3 & max );
	void Set( const D3DXVECTOR3 & min, const D3DXVECTOR3 & max );
	void Transform( const D3DXMATRIX & m );
	//void Report();

};

//-----------------------------------------------------------------------------

struct ViewFrustum {
	D3DXPLANE frustum[6];
	enum Containment { INSIDE, OUTSIDE, INTERSECTS };

	ViewFrustum(D3DXMATRIX* viewProj);
	
	Containment ContainsSphere( const BoundingSphere & sphere ) const;
	Containment ContainsBox( const BoundingBox & box ) const;
};

//-----------------------------------------------------------------------------

#endif
