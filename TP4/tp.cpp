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

	ImageBase imIn, *imOut1, *imOut2, imDecode;
	imIn.load("../perroquet.pgm");
	cout << "init" << endl;
	imOut1 = imIn.XOR(80);
	imOut1->save("encode.pgm");
	imDecode.load("encode.pgm");
	cout << "encode entropy:" << imDecode.entropy() << endl;
	imOut1 = imDecode.XOR(80);
	imOut1->save("decode.pgm");
	cout << "decode entropy:" << imIn.entropy() << endl;

	auto res = imDecode.bruteforce();
	if (res) res.value()->save("bruteforced.pgm");


	imOut1 = imIn.XOR(10);
	imOut2 = imIn.XOR(11);
	imDecode = *imOut1->diff(*imOut2);
	imDecode.save("diff.pgm");
	return 0;
}
