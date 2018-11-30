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
		return {r+other.r, g+other.g, b+other.b};
	}

	inline bool operator<(const Color& other) {
		return luminance() < other.luminance();
	}

	Color round() {
		return {std::round(r),std::round(g),std::round(b)};
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
		r = clamp(std::round(r), 0.0,255.0);
		g = clamp(std::round(g), 0.0,255.0);
		b = clamp(std::round(b), 0.0,255.0);
	}

	int luminance() const {
		return 0.299*r + 0.587*g + 0.114*b;
	}

	float distance(const Color& other) {
		return sqrt(pow(other.r - r, 2) + pow(other.g - g, 2) + pow(other.b - b, 2));
	}

	bool isBlack(){
		return r == 0 && g == 0 && b == 0;
	}

};

inline std::ostream &operator<<(std::ostream &stream, Color const &c) {
    return stream << c.r << " " << c.g << " " << c.b << endl;
}

typedef std::array<std::array<int, 8>, 8>  matrix;

struct pixel_block {
	int start_index;
	bool color;
	Color data[8][8];
	double dataGrey[8][8];
	matrix quantize_matrix = {{
		{16, 11, 10, 16, 24, 40, 51, 61},
		{12, 12, 14, 19, 26, 58, 60, 55},
		{14, 13, 16, 24, 40, 57, 69, 56},
		{14, 17, 22, 29, 51, 87, 80, 62},
		{18, 22, 37, 56, 68, 109, 103, 77},
		{24, 35, 55, 64, 81, 104, 113, 92},
		{49, 64, 78, 87, 103, 121, 120, 101},
		{72, 92, 95, 98, 112, 100, 103, 99}
	}};

	pixel_block(bool color, int start_index) {
		this->color = color;
		this->start_index = start_index;
	}

	pixel_block toYCbCr() {
		pixel_block res(color, start_index);
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				res.data[x][y] = data[x][y];
				res.data[x][y].toYCbCr();
			}
		}
		return res;
	}

	pixel_block toRGB() {
		pixel_block res(color, start_index);
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				res.data[x][y] = data[x][y];
				res.data[x][y].toRGB();
			}
		}
		return res;
	}

	pixel_block dct() {
		if (color) {
			return dct_color();
		} else {
			return dct_grey();
		}
	}

	pixel_block dct_grey() {
		pixel_block res(color, start_index);
		double Ci,Cj;
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
				double sum = 0;
				Ci = i == 0 ? 1./sqrt(2) : 1.;
				Cj = j == 0 ? 1./sqrt(2) : 1.;
				for (int x = 0; x < 8; x++) {
					for (int y = 0; y < 8; y++) {
						sum += dataGrey[x][y] * cos(((2*x+1) * i * M_PI)/16)
											  * cos(((2*y+1) * j * M_PI)/16);
					}
				}
				res.dataGrey[i][j] = sum * Ci * Cj * 2.0 / 8.0;
			}
		}
		return res;
	}

	pixel_block dct_color() {
		pixel_block res(color, start_index);
		double Ci,Cj;
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
				auto c = Color();
				Ci = i == 0 ? 1./sqrt(2) : 1.;
				Cj = j == 0 ? 1./sqrt(2) : 1.;
				for (int x = 0; x < 8; x++) {
					for (int y = 0; y < 8; y++) {
						c = c + data[x][y] * cos(((2*x+1) * i * M_PI)/16)
										   * cos(((2*y+1) * j * M_PI)/16);
					}
				}
				res.data[i][j] = c * Ci * Cj * 2.0 / 8.0;
			}
		}
		return res;
	}

	pixel_block idct() {
		if (color) {
			return idct_color();
		} else {
			return idct_grey();
		}
	}

	pixel_block idct_grey() {
		pixel_block res(color, start_index);
		double Ci,Cj;
		for (int x = 0; x < 8; ++x) {
			for (int y = 0; y < 8; ++y) {
				double sum = 0;
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						Ci = i == 0 ? 1./sqrt(2) : 1.;
						Cj = j == 0 ? 1./sqrt(2) : 1.;
						sum += dataGrey[i][j] * cos(((2*x+1) * i * M_PI)/16)
											  * cos(((2*y+1) * j * M_PI)/16)
											  * Ci * Cj;
					}
				}
				res.dataGrey[x][y] = sum * 2.0 / 8.0;
			}
		}
		return res;
	}

	pixel_block idct_color() {
		pixel_block res(color, start_index);
		double Ci,Cj;
		for (int x = 0; x < 8; ++x) {
			for (int y = 0; y < 8; ++y) {
				auto c = Color();
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						Ci = i == 0 ? 1./sqrt(2) : 1.;
						Cj = j == 0 ? 1./sqrt(2) : 1.;
						c = c + data[i][j] * cos(((2*x+1) * i * M_PI)/16)
										   * cos(((2*y+1) * j * M_PI)/16)
										   * Ci * Cj;
					}
				}
				res.data[x][y] = c * 2.0 / 8.0;
			}
		}
		return res;
	}

	matrix create_quantification_matrix(int qualityFactor) {
		matrix res;
		if (qualityFactor<50) {
			for (int i = 0; i<8; i++) {
				for (int j = 0; j<8; j++) {
					res[i][j] = ((quantize_matrix[i][j] * (5000 / qualityFactor) + 50)/100);
				}
			}
		} else {
			for (int i = 0; i<8; i++) {
				for (int j = 0; j<8; j++) {
					res[i][j] = ((quantize_matrix[i][j] * (200 - 2 * qualityFactor) + 50)/100);
				}
			}
		}
		for (int i = 0; i<8; i++) {
			for (int j = 0; j<8; j++) {
				res[i][j] = (int) clamp(res[i][j],1,255);
			}
		}
		return res;
	}

	pixel_block quantize(int quality_factor) {
		pixel_block res(color, start_index);
		matrix qm = create_quantification_matrix(quality_factor);
		if (color) {
			for (int i = 0; i<8; i++) {
				for (int j = 0; j<8; j++) {
					res.data[i][j] = (data[i][j]/qm[i][j]).round();
				}
			}
		} else {
			for (int i = 0; i<8; i++) {
				for (int j = 0; j<8; j++) {
					res.dataGrey[i][j] = round(dataGrey[i][j]/qm[i][j]);
				}
			}
		}
		return res;
	}

	pixel_block invquantize(int quality_factor) {
		pixel_block res(color, start_index);
		matrix qm = create_quantification_matrix(quality_factor);
		if (color) {
			for (int i = 0; i<8; i++) {
				for (int j = 0; j<8; j++) {
					res.data[i][j] = data[i][j]*qm[i][j];
				}
			}
		} else {
			for (int i = 0; i<8; i++) {
				for (int j = 0; j<8; j++) {
					res.dataGrey[i][j] = dataGrey[i][j]*qm[i][j];
				}
			}
		}
		return res;
	}

	vector<Color> zigzag() { // MinimumCodeUnit
		vector<Color> res;
		int n = 8;
		for (int slice = 0; slice < 2 * n - 1; ++slice) {
			int z = (slice < n) ? 0 : slice - n + 1;
			for (int j = z; j <= slice - z; ++j) {
				if (color) {
					res.push_back(data[j][slice - j]);
				} else {
					res.push_back({dataGrey[j][slice - j],-1,-1});
				}
			}
		}
		vector<Color>::const_reverse_iterator last = std::find_if(res.rbegin(), res.rend(), [](Color& c) {
			return !c.isBlack();
		});
		res.erase(last.base(), res.end());
		return res;
	}

	pixel_block static fromZigZag(vector<Color> mcu, bool color, int start_index) {
		int count = 0;
		int n = 8;
		pixel_block res(color, start_index);
		for (int slice = 0; slice < 2 * n - 1; ++slice) {
			int z = (slice < n) ? 0 : slice - n + 1;
			for (int j = z; j <= slice - z; ++j) {
				Color c = count < mcu.size() ? mcu[count] : Color(0,0,0);
				if (color) {
					res.data[j][slice - j] = c;
				} else {
					res.dataGrey[j][slice - j] = mcu[count].r;
				}
				count++;
			}
		}
		return res;
	};
};



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
	static ImageBase* fromBlock(vector<pixel_block> blocks, int width, int height, bool color, int quality);

	vector<pixel_block> fullCompress(int quality);
	void fullDecode(string data, HuffmanTree htree, int quality);

	unsigned char *operator[](int l);
};
