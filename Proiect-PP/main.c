#include <stdio.h>
#include <stdlib.h>
#include "bitmap_pixel.h"
#include "pictencript.h"
#include <math.h>

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
	double average = 0;
	int i,j,n;
	n = (p2[0] - p1[0]) *(p2[1] - p1[1]);
	//p1[0] = max(0, p1[0]);
	//p1[1] = min(pict->W, p1[1]);
	//p2[0] = max(0, p2[0]);
	for (i = p1[0]; i <= p2[0]; i++)
	{
		for (j = p1[1]; j <= p2[1]; j++)
		{
			if(i>0&&j>0&&i<pict->H&&j<pict->W)
				average += pict->pixels[i*pict->W + j].R;
		}
	}
	return average / n;
}

double cross_corelation(picture *pict,picture *p_template,int x,int y)
{
	int tw = p_template->W, th = p_template->H;
	int n = tw * th;
	int i, j;
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
	//TODO salveaza diferenta in o matrice pentru a nu o calucla de 2 ori
	double S_dev = 0, f_dev = 0;
	for (i = 0; i < tw; i++)
	{
		for (j = 0; j < th; j++)
		{
			uchar S_ij = p_template->pixels[i*tw + j].R;
			S_dev += S_ij - S_bar;
		}
	}
	for (i = p1[0]; i <= p2[0]; i++)
	{
		for (j = p1[1]; j <= p2[1]; j++)
		{
			uchar f_ij;
			if (i > 0 && j > 0 && i < pict->H && j < pict->W)
			{
				f_ij = pict->pixels[i*pict->W + j].R;
			}
			else
			{
				f_ij = 0;
			}	
			f_dev += f_ij - f_bar;
		}
	}
	S_dev = sqrt(S_dev / (n - 1));
	f_dev = sqrt(f_dev / (n - 1));
	//Calculate corelation
	for (i = 0; i < tw; i++)
	{
		for (j = 0; j < th; j++)
		{
			uchar S_ij = p_template->pixels[i*tw + j].R;
			uchar f_ij = pict->pixels[i*pict->W + j].R;
			double interm = (S_ij - S_bar) * (f_ij - f_bar);
			//cross_sum += ;
		}
	}
	return cross_sum / n;
}


int bmp_pattern_recognition(char *path_pict, char *path_template, float acc)
{
	//Open image and template
	picture *pict, *p_template;
	if ((pict = read_picture(path_pict)) == NULL)
		return -1;
	if ((p_template = read_picture(path_template)) == NULL)
		return -1;
	//Grayscale image and template
	grayscale(pict);
	grayscale(p_template);
	//Saving image data
	int h = pict->H, w = pict->W;
	int i, j;
	for (i = 0; i < w; i++)
	{
		for (j = 0; j < h; j++)
		{
			int x, y;
			x = i - (p_template->W / 2);
			y = j - (p_template->H / 2);
			if (cross_corelation(pict, p_template, x, y) > acc)
				printf("%d %d", i, j);
		}
	}
		
	return write_picture(pict, "test_gray.bmp");
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
	*/
	int p1[2] = { 0,0 }, p2[2] = {14,10};
	picture * a, *b;
	a = read_picture("cifre_mana.bmp");
	b = read_picture("Templates\\abl.bmp");
	printf("%lf\n",avg_color(a, p1, p2));
	cross_corelation(a, b, 0, 0);
	//return bmp_pattern_recognition("cifre_mana.bmp", "Templates\\cifra0.bmp", 0.5);
}
