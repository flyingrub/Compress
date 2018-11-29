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
	for (auto& b : blocks) {
		b = b.dct().idct();
	}
	auto blocksdct = blocks[4].dct();
	auto blockidct = blocksdct.idct();
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			cout << blockidct.data[i][j] << "|" << blocks[4].data[i][j] <<endl;
		}
	}
	auto im = ImageBase::fromBlock(blocks, imIn.getWidth(), imIn.getHeight(), imIn.getColor());
	auto psnr = imIn.psnr(*im);
	im->save("test.ppm");
	cout << "init" << endl;
	cout << psnr << endl;

	return 0;
}
