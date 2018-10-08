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
	Color color1 = {87, 25, 23};
	Color color2 = {34, 56, 17};

	ImageBase imIn;
	imIn.load("../perroquet.ppm");
	cout << "init" << endl;
	auto [imOutkmean2, imOutMoykmean2] = imIn.kmean2(color1, color2, 100);
	imOutkmean2->save("imOutkmean2.ppm");
	imOutMoykmean2->save("imOutMoykmean2.ppm");

	vector<Color> init = imIn.getInitColors();
	cout << "size cluster" << init.size() << endl;
	cout << "kmean" << endl;
	auto [imOutInit, imOutMoy, cluster] = imIn.kmean(init, 10);
	cout << "psnr:" << imOutMoy->psnr(imIn) << endl;
	imOutInit->save("resInit.ppm");
	imOutMoy->save("resMoy.ppm");
	ofstream datFile;
  	datFile.open ("colorcluster.dat");
	for (auto color : cluster) {
		datFile << color;
	}
  	datFile.close();

	auto imOutGrey = imOutInit->greyScale(cluster);
	imOutGrey->save("resGrey.pgm");

	auto imResFromGrey = ImageBase::color_from_cluster("resGrey.pgm");
	imResFromGrey.save("resFromGrey.ppm");


	return 0;
}
