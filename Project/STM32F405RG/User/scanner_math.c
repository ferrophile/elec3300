#include "scanner.h"

s16 sines[200] = {
	0, 314, 628, 941, 1253, 1564, 1874, 2181, 2487, 2790, 3090, 3387, 3681, 3971, 4258, 4540, 4818, 5090, 5358, 5621,
	5878, 6129, 6374, 6613, 6845, 7071, 7290, 7501, 7705, 7902, 8090, 8271, 8443, 8607, 8763, 8910, 9048, 9178, 9298, 9409,
	9511, 9603, 9686, 9759, 9823, 9877, 9921, 9956, 9980, 9995, 10000, 9995, 9980, 9956, 9921, 9877, 9823, 9759, 9686, 9603,
	9511, 9409, 9298, 9178, 9048, 8910, 8763, 8607, 8443, 8271, 8090, 7902, 7705, 7501, 7290, 7071, 6845, 6613, 6374, 6129,
	5878, 5621, 5358, 5090, 4818, 4540, 4258, 3971, 3681, 3387, 3090, 2790, 2487, 2181, 1874, 1564, 1253, 941, 628, 314,
	0, -314, -628, -941, -1253, -1564, -1874, -2181, -2487, -2790, -3090, -3387, -3681, -3971, -4258, -4540, -4818, -5090, -5358, -5621,
	-5878, -6129, -6374, -6613, -6845, -7071, -7290, -7501, -7705, -7902, -8090, -8271, -8443, -8607, -8763, -8910, -9048, -9178, -9298, -9409,
	-9511, -9603, -9686, -9759, -9823, -9877, -9921, -9956, -9980, -9995, -10000, -9995, -9980, -9956, -9921, -9877, -9823, -9759, -9686, -9603,
	-9511, -9409, -9298, -9178, -9048, -8910, -8763, -8607, -8443, -8271, -8090, -7902, -7705, -7501, -7290, -7071, -6845, -6613, -6374, -6129,
	-5878, -5621, -5358, -5090, -4818, -4540, -4258, -3971, -3681, -3387, -3090, -2790, -2487, -2181, -1874, -1564, -1253, -941, -628, -314
};

s16 cosines[200] = {
	10000, 9995, 9980, 9956, 9921, 9877, 9823, 9759, 9686, 9603, 9511, 9409, 9298, 9178, 9048, 8910, 8763, 8607, 8443, 8271,
	8090, 7902, 7705, 7501, 7290, 7071, 6845, 6613, 6374, 6129, 5878, 5621, 5358, 5090, 4818, 4540, 4258, 3971, 3681, 3387,
	3090, 2790, 2487, 2181, 1874, 1564, 1253, 941, 628, 314, 0, -314, -628, -941, -1253, -1564, -1874, -2181, -2487, -2790,
	-3090, -3387, -3681, -3971, -4258, -4540, -4818, -5090, -5358, -5621, -5878, -6129, -6374, -6613, -6845, -7071, -7290, -7501, -7705, -7902,
	-8090, -8271, -8443, -8607, -8763, -8910, -9048, -9178, -9298, -9409, -9511, -9603, -9686, -9759, -9823, -9877, -9921, -9956, -9980, -9995,
	-10000, -9995, -9980, -9956, -9921, -9877, -9823, -9759, -9686, -9603, -9511, -9409, -9298, -9178, -9048, -8910, -8763, -8607, -8443, -8271,
	-8090, -7902, -7705, -7501, -7290, -7071, -6845, -6613, -6374, -6129, -5878, -5621, -5358, -5090, -4818, -4540, -4258, -3971, -3681, -3387,
	-3090, -2790, -2487, -2181, -1874, -1564, -1253, -941, -628, -314, 0, 314, 628, 941, 1253, 1564, 1874, 2181, 2487, 2790,
	3090, 3387, 3681, 3971, 4258, 4540, 4818, 5090, 5358, 5621, 5878, 6129, 6374, 6613, 6845, 7071, 7290, 7501, 7705, 7902,
	8090, 8271, 8443, 8607, 8763, 8910, 9048, 9178, 9298, 9409, 9511, 9603, 9686, 9759, 9823, 9877, 9921, 9956, 9980, 9995
};

u16 layerBuffer[200] = {0};
Vector3D nmlBuffer;
Vector3D vertsBuffer[3];

static void scanner_math_set_nml(u8 step) {
	nmlBuffer.x = sines[step] / 10000.0;
	nmlBuffer.y = cosines[step] / 10000.0;
	nmlBuffer.z = 0.0;
}

static void scanner_math_set_vert(u8 id, u8 step, u16 rawDist, u16 height) {
	vertsBuffer[id].x = sines[step] * rawDist / 10000.0;
	vertsBuffer[id].y = cosines[step] * rawDist / 10000.0;
	vertsBuffer[id].z = height * 10.0;
}

void scanner_math_handler() {
	u16 rawValue = laser_get_raw_value();
	static u16 rawValue0 = 0;
	u8 step = (stepper_get_count(PLATE_STEPPER) - 1) % 200;
	
	if (step == 0)
		rawValue0 = rawValue;
	
	if (heightCnt > 0) {		
		// Upper triangle
		if (step > 0) {
			scanner_math_set_nml(step - 1);
			scanner_math_set_vert(0, step, layerBuffer[step], heightCnt - 1);
			scanner_math_set_vert(1, step, rawValue, heightCnt);
			scanner_math_set_vert(2, step - 1, layerBuffer[step - 1], heightCnt);
			scanner_stl_write_face(&nmlBuffer, vertsBuffer, vertsBuffer + 1, vertsBuffer + 2);
			
			if (step == 199) {
				scanner_math_set_nml(199);
				scanner_math_set_vert(0, 0, rawValue0, heightCnt - 1);
				scanner_math_set_vert(1, 0, layerBuffer[0], heightCnt);
				scanner_math_set_vert(2, 199, layerBuffer[199], heightCnt);
				scanner_stl_write_face(&nmlBuffer, vertsBuffer, vertsBuffer + 1, vertsBuffer + 2);
			}
		}
		
		// Lower triangle
		scanner_math_set_nml(step);
		scanner_math_set_vert(0, step, layerBuffer[step], heightCnt - 1);
		if (step < 199)
			scanner_math_set_vert(1, step + 1, layerBuffer[step + 1], heightCnt - 1);
		else
			scanner_math_set_vert(1, 0, rawValue0, heightCnt - 1);
		scanner_math_set_vert(2, step, rawValue, heightCnt);
		scanner_stl_write_face(&nmlBuffer, vertsBuffer, vertsBuffer + 1, vertsBuffer + 2);
	}
	layerBuffer[step] = rawValue;
}
