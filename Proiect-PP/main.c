#include <stdio.h>
#include <stdlib.h>
#include "bitmap_pixel.h"
#include "pictencript.h"
#include <math.h>

typedef struct
{
	int x, y;
}point;
typedef struct
{
	point leftup;
	int h, w;
	double corelation;
}square;

void grayscale(picture *pict)
{
	int i, n = pict->H *pict->W;
	uchar aux;
	for (i = 0; i < n; i++)
	{
		aux =(uchar) (0.299*pict->pixels[i].R + 0.587*pict->pixels[i].G
			+ 0.114*pict->pixels[i].B);
		pict->pixels[i].R = pict->pixels[i].G = pict->pixels[i].B = aux;
	}
}
//Calculate avg color on a grayscale image in a given square(p1,p2)
//Does also correct for outside of image 
double avg_color(picture * pict,int p1[2],int p2[2])
{
	int average = 0;
	int i,j,n;
	n = (p2[0] - p1[0]+1) *(p2[1] - p1[1]+1);
	for (i = p1[0]; i <= p2[0]; i++)
	{
		for (j = p1[1]; j <= p2[1]; j++)
		{
			if(i>0&&j>0&&i<pict->H&&j<pict->W)
				average += pict->pixels[i*pict->W + j].R;
		}
	}
	return (double) average / n;
}

void color_square(picture *pict, square window, pixel color)
{
	int i, j;
	point p1 = window.leftup, p2;
	p2.x = p1.x + window.h;
	p2.y = p1.y + window.w;
	//Horizontal lines
	for (j = p1.y; j <= p2.y; j++)
	{
		///TODO Verificare in imagine
		if (j > 0 &&j < (int)pict->W)
		{
			if(p1.x>=0&&p1.x<pict->H)
				pict->pixels[p1.x * pict->W + j] = color;
			if (p2.x >= 0 && p2.x < pict->H)
				pict->pixels[p2.x * pict->W + j] = color;
		}
		
	}
	for (i = p1.x; i <= p2.x; i++)
	{
		if (i > 0 &&i < (int)pict->H)
		{
			if (p1.y >= 0 && p1.y < pict->W)
				pict->pixels[i * pict->W + p1.y] = color;
			if (p2.y >= 0 && p2.y < pict->W)
				pict->pixels[i * pict->W + p2.y] = color;
		}
	}
}

double cross_corelation(picture *pict,picture *p_template,int x,int y)
{
	int tw = p_template->W, th = p_template->H;
	int n = tw * th;
	int i, j,k;
	double cross_sum = 0;
	//Average grayscale intensity of template and image
	int p1[2] = { 0,0 }, p2[2] = { th - 1,tw - 1 };
	double S_bar = avg_color(p_template,p1,p2);
	p1[0] = x; 
	p1[1] = y;
	p2[0] = x + th - 1; 
	p2[1] = y + tw - 1;
	double f_bar = avg_color(pict, p1, p2);
	//printf("%lf %lf", S_bar, f_bar);
	//Calculate deviation
	//salveaza diferenta in o matrice pentru a nu o calucla de 2 ori
	double *diff1 = malloc(sizeof(double)*n);
	double *diff2 = malloc(sizeof(double)*n);
	if (diff1 == NULL || diff2 == NULL)
		return -1;
	double S_dev = 0, f_dev = 0;
	for (k=0,i = 0; i < th; i++)
	{
		for (j = 0; j < tw; j++,k++)
		{
			uchar S_ij = p_template->pixels[i*tw + j].R;
			diff1[k] = S_ij - S_bar;
			S_dev += diff1[k] *diff1[k];
		}
	}
	for (k=0,i = p1[0]; i <= p2[0]; i++)
	{
		for (j = p1[1]; j <= p2[1]; j++,k++)
		{
			uchar f_ij;
			if (i > 0 && j > 0 && i <(int) pict->H && j <(int) pict->W)
			{
				f_ij = pict->pixels[i*pict->W + j].R;
			}
			else
			{
				f_ij = 0;
			}
			diff2[k] = f_ij - f_bar;
			f_dev += diff2[k]*diff2[k];
		}
	}
	S_dev = sqrt(S_dev / (n - 1));
	f_dev = sqrt(f_dev / (n - 1));
	//Calculate corelation
	for (i = 0; i < n; i++)
	{
			cross_sum += (diff1[i] * diff2[i])/(S_dev*f_dev);
	}
	return cross_sum / n;
}


int bmp_pattern_recognition(picture *pict, picture *p_template, double acc,square **detections,int *nr_detections )
{
	
	//Grayscale image and template
	grayscale(pict);
	grayscale(p_template);
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
			int x, y;
			double corelation;
			x = i - (p_template->H / 2);
			y = j - (p_template->W / 2);
			corelation = cross_corelation(pict, p_template, x, y);
			if (corelation >= acc)
			{
				//printf("%d %d\n", i, j);
				square window;
				window.leftup.x = x;
				window.leftup.y = y;
				window.h = p_template->H;
				window.w = p_template->W;
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
	write_picture(pict, "cifre_pattern.bmp");
	return 0;
}

int main()
{
	/*
	if(cipherPicture("peppers.bmp","peppers_cript.bmp","secret_key.txt")==-1)
		return -1;
	if(decipherPicture("peppers_cript.bmp","peppers_decript.bmp","secret_key.txt")==-1)
		return -1;
	//Afisare chi^2
	chi2_test("peppers.bmp");
	chi2_test("peppers_cript.bmp");
	chi2_test("peppers_decript.bmp");
	
	int p1[2] = { 0,0 }, p2[2] = {14,10};
	picture * a, *b;
	a = read_picture("cifre_mana.bmp");
	b = read_picture("Templates\\abl.bmp");
	printf("%lf\n",avg_color(a, p1, p2));
	cross_corelation(a, b, 0, 0);
	*/
	/*
	picture *pict;
	pict = read_picture("peppers.bmp");
	if (pict == NULL)
		return -1;
	pixel color = { 0,0,250 };
	grayscale(pict);
	int p1[2] = { 50,50 }, p2[2] = { 100,100 };
	color_square(pict, p1, p2, color);
	
	return write_picture(pict, "peppers-colortest.bmp");
	*/
	//Open image and template
	picture *pict, *p_template;
	if ((pict = read_picture("cifre_mana.bmp")) == NULL)
		return -1;
	if ((p_template = read_picture("Templates\\cifra6.bmp")) == NULL)
		return -1;
	square * detections;
	int i,nr_detections;
	if (bmp_pattern_recognition(pict,p_template , 0.25,&detections,&nr_detections) == -1)
		return -1;
	//Color detections
	pixel color = { 0,0,255 };
	for (i = 0; i < nr_detections; i++)
		color_square(pict, detections[i], color);
	write_picture(pict,"cifre_pattern.bmp");
	system("cifre_pattern.bmp");
	return 0;
}
