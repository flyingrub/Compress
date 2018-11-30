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
	int quantize_quality = 90;
	auto data = imIn.fullCompress(quantize_quality);
	auto im = ImageBase::fromBlock(data, imIn.getWidth(), imIn.getHeight(), imIn.getColor(), quantize_quality);	// auto psnr = imIn.psnr(*im);
	im->save("test.ppm");
	cout << "psnr:" << im->psnr(imIn) << endl;

	return 0;
}
