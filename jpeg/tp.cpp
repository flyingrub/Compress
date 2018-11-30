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
	for (int i = 0; i<blocks.size(); i++) {
		blocks[0] = blocks[0].dct().quantize(50).invquantize(50).idct();
	}
	auto im = ImageBase::fromBlock(blocks, imIn.getWidth(), imIn.getHeight(), imIn.getColor());
	auto psnr = imIn.psnr(*im);
	im->save("test.ppm");
	cout << "init" << endl;
	cout << psnr << endl;

	return 0;
}
