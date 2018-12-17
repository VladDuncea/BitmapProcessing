#include <stdio.h>
#include <stdlib.h>
#include "bitmap_pixel.h"
#include "pictencript.h"
#include "pattern_recognition.h"

#include <vld.h>
#include <time.h>



int main()
{
	clock_t tStart = clock();
	if(cipherPicture("peppers.bmp","peppers_cript.bmp","secret_key.txt")==-1)
		return -1;
	if(decipherPicture("peppers_cript.bmp","peppers_decript.bmp","secret_key.txt")==-1)
		return -1;
	//Afisare chi^2
	chi2_test("peppers.bmp");
	chi2_test("peppers_cript.bmp");
	
	//Open image and template
	
	
	pattern_recognition("cifre_mana.bmp", "Templates\\templates.txt", 0.5);
	printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
	system("cifre_pattern.bmp");
	return 0;
}
