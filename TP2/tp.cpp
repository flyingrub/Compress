#include "ImageBase.h"
#include <stdio.h>
#include <tuple>
#include <iostream>

#include <iostream>
#include <fstream>

void display_plot(const char* filename, bool color);

void clean() {
	system("make clean");
}

int main(int argc, char **argv)
{
	clean();


	ImageBase imIn;
	imIn.load("../perroquet.ppm");
	cout << "init" << endl;
	auto out = imIn.toYCbCr();
	auto out2 = out->reduceSpace();
	auto out3 = out2->toRGB();
	out3->save("out.ppm");
	cout << "res / PSNR:" << out3->psnr(imIn);


	auto outTestConv = out->toRGB();
	cout << "test space conversion / PSNR:" << outTestConv->psnr(imIn);

	return 0;
}
