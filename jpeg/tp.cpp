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
	int quantize_quality = 30;
	for (int i = 0; i<blocks.size(); i++) {
		auto zigzag = blocks[i].toYCbCr().dct().quantize(quantize_quality).zigzag();
		// cout << blocks[i].data[3][3];
		blocks[i] = pixel_block::fromZigZag(zigzag,blocks[i].color,blocks[i].start_index).invquantize(quantize_quality).idct().toRGB();
		// cout << blocks[i].data[3][3] << endl;
	}
	auto im = ImageBase::fromBlock(blocks, imIn.getWidth(), imIn.getHeight(), imIn.getColor());
	auto psnr = imIn.psnr(*im);
	im->save("test.ppm");
	cout << "psnr:" << psnr << endl;

	return 0;
}
