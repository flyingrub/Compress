#include "ImageBase.h"
#include <stdio.h>
#include <tuple>
#include <iostream>

#include <iostream>
#include <fstream>

void display_plot(const char* filename, bool color);

void clean() {
//	system("make clean");
//	system("pkill gnuplot");
}

int main(int argc, char **argv)
{
	clean();

	ImageBase imIn, im2;
	imIn.load("../perroquet.ppm");
	auto blocks = imIn.toBlock();
	// for (int i = 0; i<blocks.size(); i++) {
	// 	blocks[0] = blocks[0].dct().quantize(50).invquantize(50).idct();
	// }
	auto b = blocks[0];
	vector<Color> z = b.dct().quantize(50).zigzag();
	auto nb = pixel_block::fromZigZag(z, b.color);
	for (int i = 0; i<8; i++) {
		for (int j = 0; j<8; j++) {
			cout << b.data[i][j];
			cout << nb.data[i][j] << endl;
		}
	}
	auto im = ImageBase::fromBlock(blocks, imIn.getWidth(), imIn.getHeight(), imIn.getColor());
	auto psnr = imIn.psnr(*im);
	im->save("test.ppm");
	cout << "init" << endl;
	cout << psnr << endl;

	return 0;
}
