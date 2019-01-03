#ifndef PATTERN_RECOGNITION_H_INCLUDED
#define PATTERN_RECOGNITION_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bitmap_pixel.h"

#define MIN_OVERLAP 0.2

typedef struct
{
	int x, y;
}point;

typedef struct
{
	point leftup;
	int good, digit;
	int h, w;
	double corelation;
}square;

int cmp(const void *a, const void *b)
{
	return (((square *)a)->corelation - ((square*)b)->corelation >= 0.0 ? -1 : 1);
}

void sort_detections(square * detections, int nr)
{
	qsort(detections, nr, sizeof(square), cmp);
}

int square_overlap(square * s1, square *s2)
{
	int i, j;
	point p1, p2, p3, p4;
	//Left up and right down points of s1
	p1 = s1->leftup;
	p2.x = p1.x + s1->h;
	p2.y = p1.y + s1->w;
	//Left up and right down points of s2
	p3 = s2->leftup;
	p4.x = p3.x + s2->h;
	p4.y = p3.y + s2->w;

	int overlap = 0;
	for (i = p1.x; i <= p2.x; i++)
		for (j = p1.y; j <= p2.y; j++)
			if (i >= p3.x && i <= p4.x&& j >= p3.y && j <= p4.y)
				overlap++;

	if ((double)overlap / (s1->h*s1->w + s2->h*s2->w - overlap) > MIN_OVERLAP)
		return 1;
	return 0;
}

double avg_color(picture * pict, point p1, point p2)
{
	//Calculate avg color on a grayscale image in a given square(p1,p2)
	//Does also correct for outside of image
	int average = 0;
	int i, j, n;
	n = (p2.x - p1.x + 1) *(p2.y - p1.y + 1);
	for (i = p1.x; i <= p2.x; i++)
	{
		for (j = p1.y; j <= p2.y; j++)
		{
			if (i > 0 && j > 0 && i < pict->H&&j < pict->W)
				average += pict->pixels[i*pict->W + j].R;
		}
	}
	return (double)average / n;
}

void color_square(picture *pict, square window, pixel color)
{
	int i, j;
	point p1 = window.leftup, p2;
	p2.x = p1.x + window.h;
	p2.y = p1.y + window.w;
	if (p1.x < 0)
		p1.x = 0;
	if (p2.x > pict->H)
		p2.x = pict->H - 1;
	if (p1.y < 0)
		p1.y = 0;
	if (p2.y >= pict->W)
		p2.y = pict->W - 1;
	//Horizontal lines
	for (j = p1.y; j <= p2.y; j++)
	{
		pict->pixels[p1.x * pict->W + j] = color;
		pict->pixels[p2.x * pict->W + j] = color;
	}
	for (i = p1.x; i <= p2.x; i++)
	{
		pict->pixels[i * pict->W + p1.y] = color;
		pict->pixels[i * pict->W + p2.y] = color;
	}
}

double cross_corelation(picture *pict, picture *p_template, point p)
{
	int tw = p_template->W, th = p_template->H;
	int n = tw * th;
	int i, j, k;
	double cross_sum = 0;
	//Average grayscale intensity of template and image
	point p1, p2;
	p1.x = 0; 
	p1.y = 0;
	p2.x = th - 1; 
	p2.y = tw - 1;
	double S_bar = avg_color(p_template, p1, p2);
	p1.x= p.x;
	p1.y = p.y;
	p2.x = p.x + th - 1;
	p2.y = p.y + tw - 1;
	double f_bar = avg_color(pict, p1, p2);
	//printf("%lf %lf", S_bar, f_bar);
	//Calculate deviation
	//salveaza diferenta in doi vectori pentru a nu o calcula de 2 ori
	double *diff1 = malloc(sizeof(double)*n);
	double *diff2 = malloc(sizeof(double)*n);
	if (diff1 == NULL || diff2 == NULL)
		return -1;
	double S_dev = 0, f_dev = 0;
	for (k = 0, i = 0; i < th; i++)
	{
		for (j = 0; j < tw; j++, k++)
		{
			uchar S_ij = p_template->pixels[i*tw + j].R;
			diff1[k] = S_ij - S_bar;
			S_dev += diff1[k] * diff1[k];
		}
	}
	for (k = 0, i = p1.x; i <= p2.x; i++)
	{
		for (j = p1.y; j <= p2.y; j++, k++)
		{
			uchar f_ij;
			if (i > 0 && j > 0 && i < (int)pict->H && j < (int)pict->W)
			{
				f_ij = pict->pixels[i*pict->W + j].R;
			}
			else
			{
				f_ij = 0;
			}
			diff2[k] = f_ij - f_bar;
			f_dev += diff2[k] * diff2[k];
		}
	}
	S_dev = sqrt(S_dev / (n - 1));
	f_dev = sqrt(f_dev / (n - 1));
	//Calculate corelation
	for (i = 0; i < n; i++)
	{
		cross_sum += (diff1[i] * diff2[i]) / (S_dev*f_dev);
	}
	free(diff1);
	free(diff2);
	return cross_sum / n;
}

int template_matching(picture *pict, picture *p_template, double acc, int digit, square **detections, int *nr_detections)
{
	//Saving image data
	int h = pict->H, w = pict->W;
	//Verifying every possible position in image
	*detections = NULL;
	*nr_detections = 0;
	int i, j;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			point p;
			double corelation;
			p.x = i - (p_template->H / 2);
			p.y = j - (p_template->W / 2);
			corelation = cross_corelation(pict, p_template, p);
			if (corelation >= acc)
			{
				square window;
				window.leftup.x = p.x;
				window.leftup.y = p.y;
				window.h = p_template->H;
				window.w = p_template->W;
				window.good = 1;
				window.digit = digit;
				window.corelation = corelation;
				//Add window to detections
				square * aux;
				aux = realloc(*detections, sizeof(square)*(*nr_detections + 1));
				if (aux == NULL)
				{
					fprintf(stderr, "Eroarea alocare memorie");
					return -1;
				}
				*detections = aux;
				(*detections)[*nr_detections] = window;
				(*nr_detections)++;
			}
		}
	}
	return 0;
}

void remove_non_max(square * detections, int n)
{
	int removed = 0;
	int i, j;
	for (i = 0; i < n-1; i++)
	{
		if (!detections[i].good)
			continue;
		for (j = i + 1; j < n; j++)
		{
			if (detections[j].good && square_overlap(detections + i, detections + j))
			{
				removed++;
				detections[j].good = 0;
			}
		}
	}

	//Print stats
	printf("Am eliminat %d ferestre\n", removed);
}

int pattern_recognition(char *path_pict, char *path_templates, double acc)
{
	//Open image and make it grayscale
	picture *pict;
	if ((pict = read_picture(path_pict)) == NULL)
		return -1;
	grayscale(pict);

	//Open the .txt with the names of all the templates
	picture *p_template;
	square * detections = NULL;
	int i, nr_detections = 0;
	FILE * f = fopen(path_templates, "r");
	if (f == NULL)
	{
		fprintf(stderr, "\nNu se poate deschide fisieriul %s !\n", path_templates);
		return -1;
	}

	//Getting base directory
	char template_location[100];
	strcpy(template_location, path_templates);
	while (template_location[strlen(template_location) - 1] != '\\')
		template_location[strlen(template_location) - 1] = '\0';
	int nrchar = strlen(template_location);

	//Open templates one by one and save the detections
	for (i = 0; i <= 9; i++)
	{
		//Open template for i digit
			//Path for i template
		fscanf(f, "%s", template_location + nrchar);
		fgetc(f); //Clear \n
		if ((p_template = read_picture(template_location)) == NULL)
			return -1;

		//Grayscale template
		grayscale(p_template);

		//Search for this templates detections
		square * current_detections;
		int current_nr_detections;
		if (template_matching(pict, p_template, acc, i, &current_detections, &current_nr_detections) == -1)
			return -1;

		//If new squares are found add them
		if (current_nr_detections != 0)
		{
			//Create space for the new detections
			square * aux = realloc(detections, sizeof(square)*(nr_detections + current_nr_detections));
			if (aux == NULL)
			{
				fprintf(stderr, "Eroare alocare memorie!");
				return -1;
			}
			detections = aux;

			//Copy the detections at the end of the already found ones
			memcpy(detections + nr_detections, current_detections, current_nr_detections * sizeof(square));
			nr_detections += current_nr_detections;

			//Free the used memory
			free(current_detections);
		}

		//Free the template from memory
		free_pict(&p_template);

		//Print stats
		printf("Am verificat template-ul pt cifra %d si am %d detectii\n", i, current_nr_detections);
	}
	fclose(f);

	//Sort detections 
	sort_detections(detections, nr_detections);

	//Remove non-maxim
	remove_non_max(detections, nr_detections);

	//Color detections
		//Reopen the image to eliminate the grayscale effect
	if ((pict = read_picture(path_pict)) == NULL)
		return -1;
	pixel color[10] = { { 0,0,255 },{0,255,255 }, { 0,255,0 }, { 255,255,0 }, { 255,0,255 }, { 255,0,0 }, { 192,192,192 }, { 0,140,255 }, { 128,0,128 }, { 0,0,128 } };
	for (i = 0; i < nr_detections; i++)
	{
		if (detections[i].good)
			color_square(pict, detections[i], color[detections[i].digit]);
	}

	//Save picture with colored detections
	write_picture(pict, path_pict);

	//Free memory
	free(detections);
	free_pict(&pict);
	return 0;
}

#endif // PATTERN_RECOGNITION_H_INCLUDED