/******************************************************************************
* ICAR_Library
*
* Fichier : ImageBase.h
*
* Description : Classe contennant quelques fonctionnalit�s de base
*
* Auteur : Mickael Pinto
*
* Mail : mickael.pinto@live.fr
*
* Date : Octobre 2012
*
*******************************************************************************/

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <iostream>
#include <algorithm>
#include "huffman.h"
#include <variant>

using namespace std;

struct pixel {
	int i,j, color;
};

struct Color {
	double r,g,b;

	Color() {
		r=0;g=0;b=0;
	}

	Color (double r, double g, double b) {
		this->r = r;
		this->g = g;
		this->b = b;
	}

	Color(int i, unsigned char* data) {
		r = data[i];
		g = data[i+1];
		b = data[i+2];
	}

	Color(int i, double* data) {
		r = data[i];
		g = data[i+1];
		b = data[i+2];
	}

	bool equal(const Color& other) {
		return r == other.r && g == other.g && b == other.b;
	}

	inline Color operator*(float n) {
		return {r*n, g*n, b*n};
	}

	inline Color operator/(float n) {
		return {r/n, g/n, b/n};
	}

	inline Color operator+(const Color& other) {
		return {r+other.r, g+other.g, b=other.b};
	}

	inline bool operator<(const Color& other) {
		return luminance() < other.luminance();
	}

	void toYCbCr() {
    	double y = 0.299*r + 0.587*g + 0.114*b ;
    	double cb = 128.0 - (0.168736*r) - (0.331264*g) + (0.5*b);
    	double cr = 128.0 + (0.5*r) - (0.418688*g) - (0.081312*b);
		r = y;
		g = cb;
		b = cr;
	}

	void toRGB() {
		double y = r;
		double cb = g;
		double cr = b;
		r = 1.402*(cr-128.0) + y;
    	g = -0.344136*(cb-128.0)-0.714136*(cr-128.0) + y;
    	b = 1.772*(cb-128.0) + y;
		r = clamp(round(r), 0.0,255.0);
		g = clamp(round(g), 0.0,255.0);
		b = clamp(round(b),0.0,255.0);
	}

	int luminance() const {
		return 0.299*r + 0.587*g + 0.114*b;
	}

	float distance(const Color& other) {
		return sqrt(pow(other.r - r, 2) + pow(other.g - g, 2) + pow(other.b - b, 2));
	}

};

struct pixel_block {
	int start_index;
	bool color;
	Color data[8][8];
	double dataGrey[8][8];

	pixel_block(bool color) {
		this->color = color;
	}

	pixel_block dct() {
		pixel_block res(color);
		res.start_index = start_index;
		double Ci,Cj;
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
				auto color = Color();
				Ci = i == 0 ? 1./sqrt(2) : 1.;
				Cj = j == 0 ? 1./sqrt(2) : 1.;
				for (int x = 0; x < 8; x++) {
					for (int y = 0; y < 8; y++) {
						color = color + data[x][y] * cos(((2*x+1) * i * M_PI)/16)
												   * cos(((2*y+1) * j * M_PI)/16);
					}
				}
				res.data[i][j] = color * Ci * Cj * 2.0 / 8.0;
			}
		}
		return res;
	}

	pixel_block idct() {
		pixel_block res(color);
		res.start_index = start_index;
		double Ci,Cj;
		for (int x = 0; x < 8; ++x) {
			for (int y = 0; y < 8; ++y) {
				auto color = Color();
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						Ci = i == 0 ? 1./sqrt(2) : 1.;
						Cj = j == 0 ? 1./sqrt(2) : 1.;
						color = color + data[i][j] * cos(((2*x+1) * i * M_PI)/16)
												   * cos(((2*y+1) * j * M_PI)/16)
												   * Ci * Cj;
					}
				}
				res.data[x][y] = color * 2.0/8.0;
			}
		}
		return res;
	}

};

inline std::ostream &operator<<(std::ostream &stream, Color const &c) {
    return stream << c.r << " " << c.g << " " << c.b << endl;
}

class ImageBase
{
	///////////// Enumerations
  public:
	typedef enum { PLAN_R,
				   PLAN_G,
				   PLAN_B } PLAN;

	///////////// Attributs
  protected:
	unsigned char *data;
	double *dataD;

	bool color;
	int height;
	int width;
	int nTaille;
	bool isValid;
	int histogramRed[256]; // used for grey
	int histogramGreen[256];
	int histogramBlue[256];

	///////////// Constructeurs/Destructeurs
  protected:
	void init();
	void reset();

  public:
	ImageBase(void);
	ImageBase(int imWidth, int imHeight, bool isColor);
	~ImageBase(void);

	///////////// Methodes
  protected:
	void copy(const ImageBase &copy);
	void calcHistogramGrey();
	void calcHistogramColor();

  public:
	int getHeight() { return height; };
	int getWidth() { return width; };
	int getTotalSize() { return nTaille; };
	int getValidity() { return isValid; };
	bool getColor() { return color; };
	unsigned char *getData() { return data; };

	void load(char *filename);
	bool save(char *filename);

	ImageBase *getPlan(PLAN plan);

	void writeHistogram(const char *filename);
	void calcHistogram();

	ImageBase *greyScale();

	ImageBase *extension_dynamique();
	ImageBase *egalisation();
	ImageBase *seuil(int s1);
	ImageBase *seuil(int s1, int s2, int s3);
	ImageBase *erosion();
	ImageBase *dilatation();
	ImageBase *ouverture();
	ImageBase *fermeture();
	ImageBase *flou();
	ImageBase *flouBack(ImageBase *back);

	vector<int> getNeighborhood(int i, int j, int size);
	vector<int> getNeighborhood(int i, int j, int size, ImageBase *mask);
	vector<int> getNeighborhoodAlt(int i, int j, int size, ImageBase *mask);
	ImageBase *getContour();
	ImageBase *calcMatch(ImageBase* mask, vector<int> patch, int patch_size);
	int getBestMatchColor(ImageBase* match, int index);
	ImageBase *getMaskNeighborhood();

	ImageBase *reconstructMoy(ImageBase *mask);
	ImageBase *reconstructDil(ImageBase *mask);
	ImageBase *reconstructDif(ImageBase *mask);


	tuple<ImageBase*, ImageBase*> kmean2(Color color1, Color color2, int iter);
	tuple<ImageBase*, ImageBase*, vector<Color>> kmean(vector<Color> cluster, int iter);
	ImageBase* greyScale(vector<Color> cluster);

	float eqm(const ImageBase& other);
	float psnr(const ImageBase& other);

	static ImageBase color_from_cluster(char* filename);

	Color randomColor();
	vector<Color> getInitColors();

	ImageBase* reduceSpace();
	ImageBase* toYCbCr();
	ImageBase* toRGB();

	tuple<string, HuffmanTree> huffmanCompress();
	unsigned char* huffmanDecode(string data, HuffmanTree htree);

	vector<pixel_block> toBlock();
	static ImageBase* fromBlock(vector<pixel_block> blocks, int width, int height, bool color);

	unsigned char *operator[](int l);
};
