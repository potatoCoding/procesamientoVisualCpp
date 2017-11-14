# procesamientoVisualCpp
Cornejo Procesamiento de Imagenes<br/>
- Echo en vsCommunity 2017
- Tiene problemas con los colores al guardar imagnes (solo 24 bits)
#14/11/17
- Agregada clase de image processor
#10/11/17
- Agregada libreria de video
- envia video a la ventana desde una web cam
# 20/10/17
Agregadomatriz de procesamiento 3x3
# 6/10/17
Agregada Interpolacion Bilineal
# 25/9/17
Agregada fucnionalidad:
- zoom
- rotacion
- traslacion
# 22/9/17 Girar Captura de Pantalla en debug no funciona en release
Agregadas fucniones:
- Matrix3D Identity(); 
- Matrix3D zero(); 
- Matrix3D Scaling(float sx, float sy); 
- Matrix3D Rotation(float thetha); 
- Matrix3D Translate(float dx, float dy); 
- Matrix3D operator*(Matrix3D& A, Matrix3D& B); 
- vector3D operator*(vector3D& V, Matrix3D& M);//Matrix M must first be transposed!!! 
- Matrix3D Transpose(Matrix3D& M); 
# 18/9/17
Agregadas estructuras Vector y Matrix 3D
