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

//Define usar ensamblador o c++
#define USE_ASM 1

vector3D operator*(vector3D& V, Matrix3D& M) 
{//Matrix M must first be transposed!!!
	vector3D R;
#ifdef USE_ASM
	//Version ensamblador
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
#pragma region old code solo en debug
		//mov esi,V
		//mov edi,M
		//movups xmm0,[esi]
		//movups xmm1,[edi]
		//movups xmm2,[edi+16]
		//movups xmm3,[edi+32]
		//mulps  xmm1,xmm0
		//mulps  xmm2,xmm0
		//mulps  xmm3,xmm0
		//haddps xmm1,xmm1//x'
		//haddps xmm2,xmm2//y'
		//haddps xmm3,xmm3//z'
		//haddps xmm1, xmm1//x'
		//haddps xmm2, xmm2//y'
		//haddps xmm3, xmm3//z'
		//lea	   esi,R
		//movd   [esi],xmm1
		//movd   [esi+4],xmm2
		//movd   [esi+8],xmm3
		//xor    eax,eax
		//mov    [esi+12],eax
#pragma endregion
		
		//Debug y release
		mov    eax,V
		mov    ebx,M
		movups xmm0,[ebx]
		movups xmm1,[ebx+16]
		movups xmm2,[ebx+32]
		movups xmm3,[eax]
		mulps  xmm0,xmm3
		mulps  xmm1,xmm3
		mulps  xmm2,xmm3
		xorps  xmm3,xmm3
		haddps xmm0, xmm0//x'
		haddps xmm1, xmm1//y'
		haddps xmm2, xmm2//z'
		haddps xmm0, xmm0//x'
		haddps xmm1, xmm1//y'
		haddps xmm2, xmm2//z'
		unpcklps xmm0,xmm1//[d,c,b,a] unpcklps [h,g,f,e] -> [f,b,e,a]
		unpckhps xmm2,xmm3//[d,c,b,a] unpckhps [h,g,f,e] -> [h,d,g,c]
		shufps xmm0,xmm2,0xe4
		movups R,xmm0

	}
#else
	//Vercion C++
	R.x = M.m00*V.x + M.m01*V.y + M.m02*V.z;
	R.y = M.m10*V.x + M.m11*V.y + M.m12*V.z;
	R.z = M.m20*V.x + M.m21*V.y + M.m22*V.z;
	R.w = 0;
#endif // USE_ASM
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