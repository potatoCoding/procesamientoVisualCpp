#pragma once

struct vector3D
{
	union {
		struct 	{ float x, y, z;};
		float v[3];
	};
};

struct Matrix3D
{
	union {
		struct 
		{
			float m00, m01, m02, d0;
			float m10, m11, m12, d1;
			float m20, m21, m22, d2;
		};
		//de 0 a 2 son visibles los demas son relleno
		float m[3][4];
	};
};

float Dot(vector3D& A, vector3D& B);