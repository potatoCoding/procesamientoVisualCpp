#include "Matrix3D.h"
#include <memory.h>
#include <math.h>

float Dot(vector3D& A, vector3D& B)
{
	float r;
	_asm {
		lea esi, A
		lea edi, B
		movups xmm0, [esi]
		movups xmm1, [edi]
		mulps  xmm0, xmm1
		haddps xmm0, xmm0 //[x+y, z+w, x+y, z+w]
		haddps xmm0, xmm0 //[x+y+z+w, x+y+z+w, x+y+z+w, x+y+z+w]
		movd   r, xmm0
	}
	return r;
}
Matrix3D Identity() {
	Matrix3D I = zero();
	I.m00 = 1.0f; I.m11 = 1.0f; I.m22 = 1.0f;
	return I;
}

Matrix3D zero() {
	Matrix3D z;
	memset(&z, 0, sizeof(z));
	return z;
}

Matrix3D Scaling(float sx, float sy) {
	Matrix3D S = Identity();
	S.m00 = sx;
	S.m11 = sy;
	return S;
}

Matrix3D Rotation(float theta) {
	Matrix3D R = Identity();
	R.m00 = cosf(theta);
	R.m11 = R.m00;
	R.m01 = sinf(theta);
	R.m10 = -R.m01;
	return R;
}

Matrix3D Translate(float dx, float dy) {
	
	Matrix3D  T = Identity();
	T.m20 = dx; T.m21 = dy;
	return T;
}


Matrix3D operator*(Matrix3D& A, Matrix3D& B) {
	Matrix3D R = zero();
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < 3; k++)
			{
				R.m[j][i] += A.m[j][k] * B.m[k][i];
			}
		}

	}
	return R;
}

vector3D operator*(vector3D& V, Matrix3D& M) 
{//Matrix M must first be transposed!!!
	vector3D R;
	__asm {
		/*[x, y, z] *[m00, m01, m02]
					 [m10, m11, m12]
					 [m20, m21, m22]
					 Transponiendo se puede optinizar el producto usando 
					 combinacion de filas y np filas x columnas
					 ^T ->*/
		/*[x, y, z] *[m00, m10, m20]
					 [m01, m11, m21]
					 [m02, m12, m22]*/
		mov esi,V
		mov edi,M
		movups xmm0,[esi]
		movups xmm1,[edi]
		movups xmm2,[edi+16]
		movups xmm3,[edi+32]
		mulps  xmm1,xmm0
		mulps  xmm2,xmm0
		mulps  xmm3,xmm0
		haddps xmm1,xmm1//x'
		haddps xmm2,xmm2//y'
		haddps xmm3,xmm3//z'
		haddps xmm1, xmm1//x'
		haddps xmm2, xmm2//y'
		haddps xmm3, xmm3//z'
		lea	   esi,R
		movd   [esi],xmm1
		movd   [esi+4],xmm2
		movd   [esi+8],xmm3
		xor    eax,eax
		mov    [esi+12],eax
	}
	return R;

}

Matrix3D Transpose(Matrix3D& M) {
	Matrix3D T = zero();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			T.m[j][i] = M.m[i][j];
		}
	}
	return T;
}