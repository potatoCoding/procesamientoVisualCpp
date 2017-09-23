#pragma once

struct vector3D
{
	//4 dimensiones por eficiencia computacional
	union {
		struct 	{ float x, y, z, w;};
		float v[4];
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
Matrix3D Identity();
Matrix3D zero();
Matrix3D Scaling(float sx, float sy);
Matrix3D Rotation(float thetha);
Matrix3D Translate(float dx, float dy);
Matrix3D operator*(Matrix3D& A, Matrix3D& B);
vector3D operator*(vector3D& V, Matrix3D& M);//Matrix M must first be transposed!!!
Matrix3D Transpose(Matrix3D& M);