#include "Matrix3D.h"

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
