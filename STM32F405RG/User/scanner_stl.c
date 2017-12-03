#include "scanner.h"

static void scanner_stl_write_vector(Vector3D * vector) {
	uart_tx_float(vector->x);
	uart_tx_float(vector->y);
	uart_tx_float(vector->z);
}

void scanner_stl_write_header(u32 trigCnt) {
	u8 cnt = 80;
	while (cnt--) uart_tx_byte(0x00);
	uart_tx_dword(trigCnt);
}

void scanner_stl_write_face(Vector3D * nml, Vector3D * vert1, Vector3D * vert2, Vector3D * vert3) {
	// Normal vector
	scanner_stl_write_vector(nml);
	
	// Vertices of triangle
	scanner_stl_write_vector(vert1);
	scanner_stl_write_vector(vert2);
	scanner_stl_write_vector(vert3);
	
	// Attrib. byte count
	uart_tx_word(0x0000);
}
