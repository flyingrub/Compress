/******************************************************************************
 * ICAR_Library
 *
 * Fichier : ImageBase.cpp
 *
 * Description : Voir le fichier .h
 *
 * Auteur : Mickael Pinto
 *
 * Mail : mickael.pinto@live.fr
 *
 * Date : Octobre 2012
 *
 *******************************************************************************/

#include "ImageBase.h"
#include "image_ppm.h"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <cstdlib>    /* srand, rand */
#include <numeric>
#include <sstream>
#include <vector>
#include <tuple>
using namespace std;

void print(char *s) { cout << "test" << endl; }

void write_plot(const char *filename, double *data, int size) {
	FILE *file = fopen(filename, "w");
	if (file == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
	for (int j = 0; j < size; j++) {
		fprintf(file, "%d %lf\n", j, data[j]);
	}
	fclose(file);
}

void display_plot(const char *filename, bool color) {
	stringstream stream;
	if (color) {
		stream << "gnuplot -p -e \"plot '" << filename
			   << "' using 1:2 with lines, '' using 1:3 with lines, '' "
				  "using 1:4 "
				  "with lines\"";
	} else {
		stream << "gnuplot -p -e \"plot '" << filename
			   << "' with lines\"";
	}
	cout << stream.str() << endl;
	system(stream.str().c_str());
}

ImageBase::ImageBase(void) {
	isValid = false;
	init();
}

ImageBase::ImageBase(int imWidth, int imHeight, bool isColor) {
	isValid = false;
	init();

	color = isColor;
	height = imHeight;
	width = imWidth;
	nTaille = height * width * (color ? 3 : 1);

	if (nTaille == 0)
		return;

	allocation_tableau(data, OCTET, nTaille);
	dataD = (double *)malloc(sizeof(double) * nTaille);
	isValid = true;
}

ImageBase::~ImageBase(void) { reset(); }

void ImageBase::init() {
	if (isValid) {
		free(data);
		free(dataD);
	}

	data = 0;
	dataD = 0;
	height = width = nTaille = 0;
	isValid = false;
}

void ImageBase::reset() {
	if (isValid) {
		free(data);
		free(dataD);
	}
	isValid = false;
}

void ImageBase::load(char *filename) {
	init();

	int l = strlen(filename);

	if (l <= 4) // Le fichier ne peut pas etre que ".pgm" ou ".ppm"
	{
		printf("Chargement de l'image impossible : Le nom de fichier "
			   "n'est pas "
			   "conforme, il doit comporter l'extension, et celle ci "
			   "ne peut �tre que "
			   "'.pgm' ou '.ppm'");
		exit(0);
	}

	int nbPixel = 0;

	if (strcmp(filename + l - 3, "pgm") ==
		0) // L'image est en niveau de gris
	{
		color = false;
		lire_nb_lignes_colonnes_image_pgm(filename, &height, &width);
		nbPixel = height * width;

		nTaille = nbPixel;
		allocation_tableau(data, OCTET, nTaille);
		lire_image_pgm(filename, data, nbPixel);
	} else if (strcmp(filename + l - 3, "ppm") ==
			   0) // L'image est en couleur
	{
		color = true;
		lire_nb_lignes_colonnes_image_ppm(filename, &height, &width);
		nbPixel = height * width;

		nTaille = nbPixel * 3;
		allocation_tableau(data, OCTET, nTaille);
		lire_image_ppm(filename, data, nbPixel);
	} else {
		printf("Chargement de l'image impossible : Le nom de fichier "
			   "n'est pas "
			   "conforme, il doit comporter l'extension, et celle ci "
			   "ne peut �tre que "
			   ".pgm ou .ppm");
		exit(0);
	}

	dataD = (double *)malloc(sizeof(double) * nTaille);

	isValid = true;
}

bool ImageBase::save(char *filename) {
	if (!isValid) {
		printf("Sauvegarde de l'image impossible : L'image courante "
			   "n'est pas valide");
		exit(0);
	}

	if (color)
		ecrire_image_ppm(filename, data, height, width);
	else
		ecrire_image_pgm(filename, data, height, width);

	return true;
}

ImageBase *ImageBase::getPlan(PLAN plan) {
	if (!isValid || !color)
		return 0;

	ImageBase *greyIm = new ImageBase(width, height, false);

	switch (plan) {
	case PLAN_R:
		planR(greyIm->data, data, height * width);
		break;
	case PLAN_G:
		planV(greyIm->data, data, height * width);
		break;
	case PLAN_B:
		planB(greyIm->data, data, height * width);
		break;
	default:
		printf("Il n'y a que 3 plans, les valeurs possibles ne sont "
			   "donc que "
			   "'PLAN_R', 'PLAN_G', et 'PLAN_B'");
		exit(0);
		break;
	}

	return greyIm;
}

void ImageBase::copy(const ImageBase &copy) {
	for (int i = 0; i < nTaille; ++i) {
		data[i] = copy.data[i];
	}
}

unsigned char *ImageBase::operator[](int i) {
	if (!isValid) {
		printf("L'image courante n'est pas valide");
		exit(0);
	}

	if ((!color && i >= nTaille) || (color && i >= nTaille)) {
		printf("L'indice se trouve en dehors des limites de l'image");
		exit(0);
	}

	return data + i;
}

ImageBase *ImageBase::seuil(int seuil) {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	for (int i = 0; i < nTaille; i++) {
		if (data[i] < seuil) {
			imageRes->data[i] = 0;
		} else {
			imageRes->data[i] = 255;
		}
	}
	return imageRes;
}

ImageBase *ImageBase::seuil(int seuil1, int seuil2, int seuil3) {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	for (int i = 0; i < nTaille; i++) {
		if (data[i] < seuil1) {
			imageRes->data[i] = 0;
		} else if (data[i] < seuil2) {
			imageRes->data[i] = 255 * 1 / 3;
		} else if (data[i] < seuil3) {
			imageRes->data[i] = 255 * 2 / 3;
		} else {
			imageRes->data[i] = 255;
		}
	}
	return imageRes;
}

void ImageBase::writeHistogram(const char *filename) {
	this->calcHistogram();
	FILE *file = fopen(filename, "w");
	if (file == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
	if (color) {
		for (int j = 0; j < 256; j++) {
			fprintf(file, "%d %d %d %d\n", j, histogramRed[j],
					histogramGreen[j], histogramBlue[j]);
		}
	} else {
		for (int j = 0; j < 256; j++) {
			fprintf(file, "%d %d\n", j, histogramRed[j]);
		}
	}
	fclose(file);
	display_plot(filename, color);
}

void ImageBase::calcHistogram() {
	std::fill_n(histogramRed, 256, 0);
	if (color) {
		std::fill_n(histogramGreen, 256, 0);
		std::fill_n(histogramBlue, 256, 0);
		for (int i = 0; i < nTaille; i += 3) {
			histogramRed[data[i]]++;
			histogramGreen[data[i + 1]]++;
			histogramBlue[data[i + 2]]++;
		}
	} else {
		for (int i = 0; i < nTaille; i++) {
			histogramRed[(int)data[i]]++;
		}
	}
}

struct alphaBeta {
	alphaBeta(int alpha, int beta) {
		this->alpha = alpha;
		this->beta = beta;
	}
	int alpha;
	int beta;
};

alphaBeta *getAlphaBeta(int *histogram) {
	int seuil = 10;
	int aMin = -1, aMax = -1;
	for (int j = 0; j < 256; j++) {
		if (histogram[j] > seuil && aMin == -1) {
			aMin = j;
		}
		if (histogram[255 - j] > seuil && aMax == -1) {
			aMax = 255 - j;
		}
	}
	int alpha = (-255 * aMin / (aMax - aMin));
	int beta = 255 / (aMax - aMin);

	return new alphaBeta(alpha, beta);
}

ImageBase *ImageBase::extension_dynamique() {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	if (color) {
		alphaBeta *resRed = getAlphaBeta(histogramRed);
		alphaBeta *resGreen = getAlphaBeta(histogramGreen);
		alphaBeta *resBlue = getAlphaBeta(histogramBlue);
		printf("R alpha: %d | beta: %d\n", resRed->alpha, resRed->beta);
		printf("G alpha: %d | beta: %d\n", resGreen->alpha,
			   resGreen->beta);
		printf("B alpha: %d | beta: %d\n", resBlue->alpha,
			   resBlue->beta);
		for (int i = 0; i < nTaille; i += 3) {
			imageRes->data[i] =
				resRed->alpha + resRed->beta * data[i];
			imageRes->data[i + 1] =
				resGreen->alpha + resGreen->beta * data[i + 1];
			imageRes->data[i + 2] =
				resBlue->alpha + resBlue->beta * data[i + 2];
		}
	} else {
		alphaBeta *res = getAlphaBeta(histogramRed);
		printf("alpha: %d | beta: %d\n", res->alpha, res->beta);
		for (int i = 0; i < nTaille; i++) {
			imageRes->data[i] = res->alpha + res->beta * data[i];
		}
	}
	return imageRes;
}

ImageBase *ImageBase::greyScale() {
	ImageBase *imageRes =
		new ImageBase(this->getWidth(), this->getHeight(), false);
	if (!color) {
		imageRes->copy(*this);
		return imageRes;
	}
	for (int i = 0; i < nTaille; i += 3) {
		int r = this->data[i], g = this->data[i + 1],
			b = this->data[i + 2], grey;
		// grey = (r + g + b) / 3; // Average
		grey = (0.21 * r) + (0.72 * g) +
			   (0.07 * b); // Luminosity
						   // grey = 0.5 * (max(max(r,g),b) +
						   // min(min(r,g),b)); //Lightness
		imageRes->data[i / 3] = grey;
	}
	return imageRes;
}

ImageBase *ImageBase::egalisation() {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	double f[256];
	double F[256];
	for (int i = 0; i < 256; i++) {
		f[i] = (double)histogramRed[i] / (double)(width * height);
	}
	// printf("%lf %lf %lf", f[0], (double)histogramRed[0], (double)(width *
	// height));
	write_plot("ddp.dat", f, 256);
	F[0] = f[0];
	for (int i = 1; i < 256; i++) {
		F[i] = F[i - 1] + f[i];
	}
	write_plot("F(a).dat", F, 256);
	display_plot("F(a).dat", false);
	for (int i = 0; i < nTaille; i++) {
		imageRes->data[i] = (double)255 * F[this->data[i]];
	}
	return imageRes;
}

ImageBase *ImageBase::erosion() {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	imageRes->copy(*this);
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (this->data[i * width + j] == 255) {
				imageRes->data[(i + 1) * width + j] = 255;
				imageRes->data[(i - 1) * width + j] = 255;
				imageRes->data[(i)*width + (j + 1)] = 255;
				imageRes->data[(i)*width + (j - 1)] = 255;
				imageRes->data[(i + 1) * width + (j + 1)] = 255;
				imageRes->data[(i + 1) * width + (j - 1)] = 255;
				imageRes->data[(i - 1) * width + (j + 1)] = 255;
				imageRes->data[(i - 1) * width + (j - 1)] = 255;
			}
		}
	}
	return imageRes;
}

ImageBase *ImageBase::dilatation() {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	imageRes->copy(*this);
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (this->data[i * width + j] == 0) {
				imageRes->data[(i + 1) * width + j] = 0;
				imageRes->data[(i - 1) * width + j] = 0;
				imageRes->data[(i)*width + (j + 1)] = 0;
				imageRes->data[(i)*width + (j - 1)] = 0;
				imageRes->data[(i + 1) * width + (j + 1)] = 0;
				imageRes->data[(i + 1) * width + (j - 1)] = 0;
				imageRes->data[(i - 1) * width + (j + 1)] = 0;
				imageRes->data[(i - 1) * width + (j - 1)] = 0;
			}
		}
	}
	return imageRes;
}

ImageBase *ImageBase::fermeture() {
	ImageBase *imageRes = this->dilatation();
	imageRes = imageRes->erosion();
	return imageRes;
}

ImageBase *ImageBase::ouverture() {
	ImageBase *imageRes = this->erosion();
	imageRes = imageRes->dilatation();
	return imageRes;
}

ImageBase *ImageBase::flou() {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	imageRes->copy(*this);
	if (color) {
		ImageBase *r = getPlan(PLAN_R);
		ImageBase *g = getPlan(PLAN_G);
		ImageBase *b = getPlan(PLAN_B);
		r = r->flou();
		g = g->flou();
		b = b->flou();
		for (int i = 0; i < nTaille; i += 3) {
			imageRes->data[i] = r->data[i / 3];
			imageRes->data[i + 1] = g->data[i / 3];
			imageRes->data[i + 2] = b->data[i / 3];
		}
	} else {
		for (int i = 1; i < height - 1; i++) {
			for (int j = 1; j < width - 1; j++) {
				imageRes->data[i * width + j] =
					(this->data[(i + 1) * width + j] +
					 this->data[(i - 1) * width + j] +
					 this->data[(i)*width + (j + 1)] +
					 this->data[(i)*width + (j - 1)] +
					 this->data[(i + 1) * width + (j + 1)] +
					 this->data[(i + 1) * width + (j - 1)] +
					 this->data[(i - 1) * width + (j + 1)] +
					 this->data[(i - 1) * width + (j - 1)] +
					 this->data[i * width + j]) /
					9;
			}
		}
	}
	return imageRes;
}

ImageBase *ImageBase::flouBack(ImageBase *background) {
	ImageBase *imageRes = this->flou();
	for (int i = 0; i < nTaille; i += 3) {
		if (background->data[i / 3] == 0) {
			imageRes->data[i] = this->data[i];
			imageRes->data[i + 1] = this->data[i + 1];
			imageRes->data[i + 2] = this->data[i + 2];
		}
	}
	return imageRes;
}

// TP7 //

vector<int> ImageBase::getNeighborhood(int x, int y, int size) {
	vector<int> neighborood;

	for (int i = -size; i <= size; i++) {
		for (int j = -size; j <= size; j++) {
			if (i == 0 && j == 0)
				continue;
			neighborood.push_back(this->data[(x + i) * width + y + j]);
		}
	}
	return neighborood;
}

vector<int> ImageBase::getNeighborhood(int x, int y, int size, ImageBase *mask) {
	vector<int> neighborood;

	for (int i = -size; i <= size; i++) {
		for (int j = -size; j <= size; j++) {
			int index = (x + i) * width + y + j;
			if ((i == 0 && j == 0) || (mask->data[index]) == 0)
				continue;
			neighborood.push_back(this->data[index]);
		}
	}
	return neighborood;
}

vector<int> ImageBase::getNeighborhoodAlt(int x, int y, int size, ImageBase *mask) {
	vector<int> neighborood;

	for (int i = -size; i <= size; i++) {
		for (int j = -size; j <= size; j++) {
			int index = (x + i) * width + y + j;
			if ((i == 0 && j == 0) || (mask->data[index]) == 0) {
				neighborood.push_back(-1);
			} else {
				neighborood.push_back(this->data[index]);
			}
		}
	}
	return neighborood;
}

ImageBase *ImageBase::reconstructMoy(ImageBase *mask) {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	imageRes->copy(*this);
	ImageBase *contour = mask->getContour();
	bool reconstructed = false;
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (contour->data[i * width + j] == 0) {
				vector<int> neighborhood = this->getNeighborhood(i, j, 3, mask);

				int sum = accumulate(neighborhood.begin(),
									 neighborhood.end(), 0);
				int n = neighborhood.size();
				if (n != 0) {
					reconstructed = true;
					imageRes->data[i * width + j] = sum / n;
				}
			}
		}
	}
	if (reconstructed) {
		return imageRes->reconstructMoy(mask->erosion());
	}
	return imageRes;
}

ImageBase *ImageBase::reconstructDil(ImageBase *mask) {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	imageRes->copy(*this);
	ImageBase *contour = mask->getContour();
	bool reconstructed = false;
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (contour->data[i * width + j] == 0) {
				vector<int> neighborhood = this->getNeighborhood(i, j, 1, mask);
				int max = *max_element(neighborhood.begin(), neighborhood.end());
				reconstructed = true;
				imageRes->data[i * width + j] = max;
			}
		}
	}
	if (reconstructed) {
		return imageRes->reconstructDil(mask->erosion());
	}
	return imageRes;
}

ImageBase *ImageBase::getContour() {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	imageRes->copy(*this);
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			imageRes->data[i * width + j] = 255;
			if (this->data[i * width + j] == 0) {
				vector<int> neighborhood = this->getNeighborhood(i, j, 1);
				for (auto const &neighbor : neighborhood) {
					if (neighbor == 255) {
						imageRes->data[i * width + j] = 0;
						break;
					}
				}
			}
		}
	}
	return imageRes;
}

ImageBase *ImageBase::calcMatch(ImageBase *neighborhoodMask, vector<int> patch, int patch_size) {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	for (int i = 0; i < patch_size; i++) {
		for (int j = 0; j < width; j++) {
			imageRes->dataD[i * width + j] = INFINITY;
			imageRes->dataD[(height - i - 1) * width + j] = INFINITY;
		}
		for (int j = 0; j < height; j++) {
			imageRes->dataD[j * width + i] = INFINITY;
			imageRes->dataD[j * width + width - i - 1] = INFINITY;
		}
	}
	for (int i = patch_size; i < height - patch_size; i++) {
		for (int j = patch_size; j < width - patch_size; j++) {
			int index = i * width + j;
			if (neighborhoodMask->data[index] == 0) {
				imageRes->dataD[index] = INFINITY;
				continue;
			}
			vector<int> neighborhood = this->getNeighborhoodAlt(i, j, patch_size, neighborhoodMask);
			float sum = 0;
			float n = 0;
			for (int k = 0; k < patch.size(); k++) {
				if (patch[k] != -1 && neighborhood[k] != -1) {
					int delta = abs(neighborhood[k] - patch[k]);
					sum += delta;
					n++;
				}
			}
			if (n < patch.size() / 3) {
				imageRes->dataD[index] = INFINITY;
			} else {
				imageRes->dataD[index] = sum / n;
			}
		}
	}
	return imageRes;
}

int ImageBase::getBestMatchColor(ImageBase *match, int current_index) {
	int best_index = 0;
	double best_match = match->dataD[0];
	for (int i = 1; i < nTaille; i++) {
		// Don't compare with ourself;
		if (i == current_index) {
			continue;
		}
		if (match->dataD[i] < best_match) {
			best_index = i;
			best_match = match->dataD[i];
		}
	}
	cout << best_index << " - " << best_match << endl;
	return this->data[best_index];
}

ImageBase *ImageBase::getMaskNeighborhood() {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	ImageBase *dilatation = new ImageBase(this->getWidth(), this->getHeight(),
										  this->getColor());
	dilatation->copy(*this);
	for (int i = 0; i < 6; i++)
		dilatation = dilatation->dilatation();

	for (int i = 0; i < nTaille; i++) {
		if (dilatation->data[i] == 0 && this->data[i] != 0) {
			imageRes->data[i] = 255;
		} else {
			imageRes->data[i] = 0;
		}
	}
	return imageRes;
}

ImageBase *ImageBase::reconstructDif(ImageBase *mask) {
	ImageBase *imageRes = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	imageRes->copy(*this);
	ImageBase *newMask = new ImageBase(this->getWidth(), this->getHeight(),
									   this->getColor());
	newMask->copy(*mask);
	ImageBase *neighborhoodMask = mask->getMaskNeighborhood();
	bool reconstructed = true;
	while (reconstructed) {
		reconstructed = false;
		ImageBase *contour = newMask->getContour();
		for (int i = 1; i < height - 1; i++) {
			for (int j = 1; j < width - 1; j++) {
				int index = i * width + j;
				if (contour->data[index] == 0) {
					cout << index << endl;
					vector<int> neighborhood = imageRes->getNeighborhoodAlt(i, j, 4, newMask);
					reconstructed = true;
					ImageBase *match = this->calcMatch(neighborhoodMask, neighborhood, 4);
					imageRes->data[index] = getBestMatchColor(match, index);
				}
			}
		}
		newMask = newMask->erosion();
	}
	return imageRes;
}

tuple<ImageBase*, ImageBase*> ImageBase::kmean2(Color color1, Color color2, int iter) {
	ImageBase *imageResInit = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	ImageBase *imageResMoy = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());

	for (int i =0; i < nTaille; i+=3) {
		Color bestColor;
		Color c = Color(i , data);
		float d1 = c.distance(color1);
		float d2 = c.distance(color2);
		if (d1 < d2) {
			bestColor = color1;
		} else {
			bestColor = color2;
		}

		imageResInit->data[i] = bestColor.r;
		imageResInit->data[i+1] = bestColor.g;
		imageResInit->data[i+2] = bestColor.b;
	}

	Color sumCluster1;
	Color sumCluster2;
	int sizeCluster1 = 0;
	int sizeCluster2 = 0;
	for (int i = 0; i < nTaille; i+=3) {
		if (Color c = Color(i, imageResInit->data); c.equal(color1)) {
			sumCluster1.r += this->data[i];
			sumCluster1.g += this->data[i+1];
			sumCluster1.b += this->data[i+2];
			sizeCluster1++;
		} else {
			sumCluster2.r += this->data[i];
			sumCluster2.g += this->data[i+1];
			sumCluster2.b += this->data[i+2];
			sizeCluster2++;
		}
	}

	Color moyCluster1 = sumCluster1 / sizeCluster1;
	Color moyCluster2 = sumCluster2 / sizeCluster2;

	for (int i = 0; i < nTaille; i+=3) {
		if (Color c = Color(i, imageResInit->data); c.equal(color1)) {
			imageResMoy->data[i] = moyCluster1.r;
			imageResMoy->data[i+1] = moyCluster1.g;
			imageResMoy->data[i+2] = moyCluster1.b;
		} else {
			imageResMoy->data[i] = moyCluster2.r;
			imageResMoy->data[i+1] = moyCluster2.g;
			imageResMoy->data[i+2] = moyCluster2.b;
		}
	}

	if (iter>0) {
		return this->kmean2(moyCluster1, moyCluster2, --iter);
	}

	return make_tuple(imageResInit, imageResMoy);
}

tuple<ImageBase*, ImageBase*, vector<Color>> ImageBase::kmean(vector<Color> colorCluster, int iter) {
	ImageBase *imageResInit = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());
	ImageBase *imageResMoy = new ImageBase(this->getWidth(), this->getHeight(),
										this->getColor());


	cout << "bestDistance" << endl;
	for (int i =0; i < nTaille; i+=3) {
		Color bestColorCluster;
		int bestDistance = -1;
		Color c = Color(i , data);

		for (auto& currentCluster : colorCluster) {
			float distance = c.distance(currentCluster);
			if (bestDistance == -1 || bestDistance > distance) {
				bestColorCluster = currentCluster;
				bestDistance = distance;
			}
		}
		imageResInit->data[i] = bestColorCluster.r;
		imageResInit->data[i+1] = bestColorCluster.g;
		imageResInit->data[i+2] = bestColorCluster.b;
	}

	cout << "moyCluster " << endl;
	vector<Color> sumRes(colorCluster.size());
	vector<int> sizeRes(colorCluster.size(), 0);
	for (int i = 0; i < nTaille; i+=3) {
		Color imageResColor = Color(i, imageResInit->data);
		for (int j=0; j<colorCluster.size(); j++) {
			Color currentCluster = colorCluster[j];
			if (imageResColor.equal(currentCluster)) {
				sumRes[j] = sumRes[j] + Color(i, this->data);
				sizeRes[j]++;
				break;
			}
		}
	}

	vector<Color> moyCluster(colorCluster.size());
	for (int i = 0; i < colorCluster.size(); i++) {
		if (sizeRes[i]==0) {
			moyCluster[i] = this->randomColor();
			continue;
		}
		moyCluster[i] = sumRes[i] / sizeRes[i];
	}

	cout << "ImageResMoy " << endl;

	for (int i = 0; i < nTaille; i+=3) {
		Color imageResColor = Color(i, imageResInit->data);
		for (int j=0; j<colorCluster.size(); j++) {
			Color currentCluster = colorCluster[j];
			if (imageResColor.equal(currentCluster)) {
				imageResMoy->data[i] = moyCluster[j].r;
				imageResMoy->data[i+1] = moyCluster[j].g;
				imageResMoy->data[i+2] = moyCluster[j].b;
				break;
			}
		}
	}

	if (iter>0) {
		return this->kmean(moyCluster, --iter);
	}
	sort(colorCluster.begin(), colorCluster.end());
	return make_tuple(imageResInit, imageResMoy, colorCluster);
}

ImageBase* ImageBase::greyScale(vector<Color> cluster) {
	ImageBase *imageResGrey= new ImageBase(this->getWidth(), this->getHeight(),
										false);
	for (int i = 0; i < nTaille; i+=3) {
		Color currentColor = Color(i, this->data);
		for (int j=0; j<cluster.size(); j++) {
			Color currentCluster = cluster[j];
			if (currentColor.equal(currentCluster)) {
				imageResGrey->data[i/3] = j;
				break;
			}
		}
	}
	return imageResGrey;
}
#include <fstream>

ImageBase ImageBase::color_from_cluster(char* filename) {
	ImageBase imIn;
	imIn.load(filename);
	vector<Color> cluster;
	ImageBase imageRes = ImageBase(imIn.getWidth(), imIn.getHeight(),
										true);

	ifstream infile("colorcluster.dat");
	int r,g,b;
	while (infile >> r >> g >> b) {
		cluster.push_back({r,g,b});
	}

	for (int i = 0; i < imageRes.getTotalSize(); i+=3) {
		int colorIndex = imIn.data[i/3];
		imageRes.data[i] = cluster[colorIndex].r;
		imageRes.data[i+1] = cluster[colorIndex].g;
		imageRes.data[i+2] = cluster[colorIndex].b;
	}

	return imageRes;
}


Color ImageBase::randomColor() {
	return Color(rand()%nTaille, data);
}

vector<Color> ImageBase::getInitColors() {
	int blockSize = nTaille / (256*3);
	vector<Color> res;
	Color sum;
	// for (int i = 0; i < nTaille; i+=3) {
	// 	if (i!=0 && i % blockSize == 0) {
	// 		res.push_back(sum / blockSize);
	// 		sum = {0,0,0};
	// 	} else {
	// 		Color current = Color (i, this->data);
	// 		sum = sum + current;
	// 	}
	// }
	for (int i = 0; i< 256; i++) {
		res.push_back(this->randomColor());
	}
	return res;
}

float ImageBase::eqm(const ImageBase& other) {
	float res = 0;
	for (int i = 0; i<nTaille; i++) {
		res += pow(this->data[i] - other.data[i],2);
	}
	return res / nTaille;
}

float ImageBase::psnr(const ImageBase& other) {
	return 10 * log10(pow(255,2) / this->eqm(other));
}

