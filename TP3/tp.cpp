#include "ImageBase.h"
#include <stdio.h>
#include <tuple>
#include <iostream>

#include <iostream>
#include <fstream>

void display_plot(const char* filename, bool color);

void clean() {
	system("make clean");
	system("pkill gnuplot");
}

int main(int argc, char **argv)
{
	clean();

	ImageBase imIn;
	imIn.load("../perroquet.pgm");
	cout << "init" << endl;
	imIn.writeHistogram("histo.dat");
	auto [data, htree] = imIn.huffmanCompress();

	return 0;
}
