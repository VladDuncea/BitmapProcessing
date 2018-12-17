#ifndef BITMAP_PIXEL_H_INCLUDED
#define BITMAP_PIXEL_H_INCLUDED

typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct
{
	uchar B, G, R;
} pixel;

typedef struct
{
	int W, H;
	pixel * pixels;
	char * header;
} picture;

//Read and write picture
picture * read_picture(char * path)
{
	//Deschidere fisier pt citire binara
	FILE * f = fopen(path, "rb");
	if (f == NULL)
	{
		fprintf(stderr, "\nNu se poate deschide imaginea %s !\n", path);
		return NULL;
	}

	//Alocare mem pt struct pozei
	picture *pict;
	if ((pict = malloc(sizeof(picture))) == NULL)
		return NULL;

	if ((pict->header = malloc(54)) == NULL)
		return NULL;
	if (fread(pict->header, 54, 1, f) != 1)
		return NULL;
	//Reading W and H
	int * p = (int*)(pict->header + 18);
	pict->W = *p;
	pict->H = *(p + 1);

	//Transforming matrix of pixels into vector
	uint w = pict->W, h = pict->H;
	pixel * pixels = pict->pixels = malloc(sizeof(pixel) * w * h);
	if (pixels == NULL)
	{
		fclose(f);
		return NULL;
	}
	int i;
	for (i = h - 1; i >= 0; i--)
	{
		if (fread(pixels + i * w, 3, w, f) != w)
			return NULL;
		//Jumping over padding
		if (w % 4 != 0)
			fseek(f, 4 - (w * 3) % 4, SEEK_CUR);
	}

	fclose(f);
	return pict;
}

int write_picture(picture * const pict, char * path)
{
	if (pict == NULL)
		return -1;
	FILE * f = fopen(path, "wb");
	if (f == NULL)
	{
		fprintf(stderr, "\nNu se poate salva imaginea %s !\n", path);
		return -1;
	}
	uint h = pict->H, w = pict->W;
	//Write header
	if (fwrite(pict->header, 54, 1, f) != 1)
	{
		fclose(f);
		return -1;
	}
	//Write picture line by line
	char padding = 0;
	int i;
	for (i = h - 1; i >= 0; i--)
	{
		if (fwrite(pict->pixels + i * w, 3, w, f) != w)
		{
			fclose(f);
			return -1;
		}
		//Create padding
		if (w % 4 != 0)
			fwrite(&padding, 1, 4 - (w * 3) % 4, f);
	}
	fclose(f);
	return 0;
}

//Utility
void grayscale(picture *pict)
{
	int i, n = pict->H *pict->W;
	uchar aux;
	for (i = 0; i < n; i++)
	{
		aux = (uchar)(0.299*pict->pixels[i].R + 0.587*pict->pixels[i].G
			+ 0.114*pict->pixels[i].B);
		pict->pixels[i].R = pict->pixels[i].G = pict->pixels[i].B = aux;
	}
}

void free_pict(picture ** pict)
{
	free((*pict)->header);
	free((*pict)->pixels);
	free((*pict));
	*pict = NULL;
}

#endif // BITMAP_PIXEL_H_INCLUDED
