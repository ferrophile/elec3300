#include "scanner.h"

static void scanner_stl_write_vector(Vector3D * vector) {
	uart_tx_dword((u32)vector->x);
	uart_tx_dword((u32)vector->y);
	uart_tx_dword((u32)vector->z);
}

void scanner_stl_write_header(u32 trigCnt) {
	u8 cnt = 80;
	//while (cnt--) uart_tx_byte(0x00);
	uart_tx_dword(trigCnt);
}

void scanner_stl_write_face(Vector3D * nml, Vector3D (* verts)[3]) {
	// Normal vector
	scanner_stl_write_vector(nml);
	
	// Vertices of triangle
	scanner_stl_write_vector(verts[0]);
	scanner_stl_write_vector(verts[1]);
	scanner_stl_write_vector(verts[2]);
	
	// Attrib. byte count
	uart_tx_word(0x0000);
}
