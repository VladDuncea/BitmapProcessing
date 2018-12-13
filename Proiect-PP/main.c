#include <stdio.h>
#include <stdlib.h>
#include "bitmap_pixel.h"
#include "pictencript.h"

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
	return write_picture(pict, "test_gray.bmp");
}

int main()
{
	/*
	if(cipherPicture("alb.bmp","peppers_cript.bmp","secret_key.txt")==-1)
		return -1;
	if(decipherPicture("peppers_cript.bmp","peppers_decript.bmp","secret_key.txt")==-1)
		return -1;
	//Afisare chi^2
	chi2_test("peppers.bmp");
	chi2_test("peppers_cript.bmp");
	chi2_test("peppers_decript.bmp");
	*/
	printf("Vlad are mere");
	return bmp_pattern_recognition("cifre_mana.bmp", "Templates\\cifra0.bmp", 0.5);
}
