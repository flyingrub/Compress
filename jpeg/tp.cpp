#include "ImageBase.h"
#include <stdio.h>
#include <tuple>
#include <iostream>

#include <iostream>
#include <fstream>
#include <sstream>

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
	for (int i= 1; i<= 10; i++) {
		int quantize_quality = i*10;
		auto data = imIn.fullCompress(quantize_quality);
		auto im = ImageBase::fromBlock(data, imIn.getWidth(), imIn.getHeight(), imIn.getColor(), quantize_quality);	// auto psnr = imIn.psnr(*im);
		std::ostringstream stringStream;
  		stringStream << "Test" << quantize_quality << ".ppm";
		char* filename =(char*) stringStream.str().c_str();
		im->save(filename);
		cout << quantize_quality << " | psnr:" << im->psnr(imIn) << endl;
	}


	return 0;
}
