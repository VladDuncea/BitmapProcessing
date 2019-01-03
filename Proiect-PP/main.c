#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bitmap_pixel.h"
#include "pictencript.h"
#include "pattern_recognition.h"
//#include <vld.h>

int main()
{
	//Fisier cu datele de rulare
	FILE * fin = fopen("date.txt", "r");
	if (fin == NULL)
		return -1;

	//Citire cai criptare
	char path_pict[200], path_cript[200],path_key[200];
	fscanf(fin,"%s", path_pict);
	fgetc(fin);	//Eliminare /n
	fscanf(fin,"%s", path_cript);
	fgetc(fin);	//Eliminare /n
	fscanf(fin,"%s", path_key);
	fgetc(fin);	//Eliminare /n

	//Criptare
	if(cipherPicture(path_pict, path_cript, path_key)==-1)
		return -1;

	//Citire cai decriptare
	char path_decript[200];
	fscanf(fin, "%s", path_cript);
	fgetc(fin);	//Eliminare /n
	fscanf(fin, "%s", path_decript);
	fgetc(fin);	//Eliminare /n
	fscanf(fin, "%s", path_key);
	fgetc(fin);	//Eliminare /n

	//Decriptare
	if(decipherPicture(path_cript,path_decript,path_key)==-1)
		return -1;

	//Afisare chi^2
	chi2_test(path_pict);
	chi2_test(path_cript);
	
	//Pattern recognition
	char path_template_data[200];
	fscanf(fin, "%s", path_pict);
	fgetc(fin);	//Eliminare /n
	fscanf(fin, "%s", path_template_data);
	fgetc(fin);	//Eliminare /n

	//Rulare pattern recognition
	pattern_recognition(path_pict,path_template_data, 0.5);

	//Afisare imagine colorata
	system(path_pict);
	return 0;
}
