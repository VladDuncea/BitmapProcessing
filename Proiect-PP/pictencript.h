#ifndef PICTENCRIPT_H_INCLUDED
#define PICTENCRIPT_H_INCLUDED
#include "bitmap_pixel.h"
#include <string.h>


uint xorshift32(uint seed)
{
    //Uses seed first time or previous element
    uint x = seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

float chi2_sum(uint * collor_freq,float avg_freq)
{
    int i;
    float x=0,z;
    for(i=0;i<256;i++)
    {   z = (float)collor_freq[i]-avg_freq;
        x+= (z*z)/avg_freq;
    }
    return x;
}

void chi2_test(char *path)
{
    //Open image
    picture *pict;
    if((pict = read_picture(path)) == NULL)
        return ;
    //Calculate average freq of a color on a picture of n pixels
    int n = pict->H * pict->W;
    float avg_freq = (float) (n/256.0);
    //Calculate actual freq on this picture
    uint * collor_freq = calloc(sizeof(uint),256*3);
    int i;
    for(i=0;i<n;i++)
    {
        collor_freq[pict->pixels[i].R]++;
        collor_freq[pict->pixels[i].G+256]++;
        collor_freq[pict->pixels[i].B+512]++;
    }
    //Print chi^2 numbers
    printf("\nR:%.2f ",chi2_sum(collor_freq + 256*0,avg_freq));
    printf("G:%.2f ",chi2_sum(collor_freq + 256*1,avg_freq));
    printf("B:%.2f \n",chi2_sum(collor_freq + 256*2,avg_freq));
}

uint * random_array(int seed,int n)
{
    uint * rand_arr;
    if((rand_arr = malloc(sizeof(uint)*n)) == NULL)
    {
        fprintf(stderr, "Eroare la alocare memorie !");
        return NULL;
    }
    int i;
    uint prev = seed;
    for(i=0;i < n;i++)
    {
        rand_arr[i] = xorshift32(prev);
        prev = rand_arr[i];
    }
    return rand_arr;
}

uint * random_permutation (int n,uint * rand_arr)
{
    uint * const permutation = malloc(sizeof(uint)*n);
    int i;
    if(permutation == NULL)
    {
        fprintf(stderr, "Eroare la alocare memorie!\n");
        return NULL;
    }
        //Initialise to identic permutation
    for(i=0;i<n;i++)
    {
        permutation[i] = i;
    }
        //Shuffle permutation
    int j=0;
    for(i=n-1;i>0;i--)
    {
        uint r,aux;
        r = rand_arr[j++]%(i+1);
        if(i!=r)
        {
            aux = permutation[i];
            permutation[i] = permutation[r];
            permutation[r] = aux;
        }
    }
    return permutation;
}

int shuffle_pixels(picture * pict,uint *permutation,int n)
{
    int i;
        //Create a new picture with the same header, W and H
    pixel * shuffled_pixels;
        //Get memory for shuffled pixels
    if((shuffled_pixels = malloc(sizeof(pixel)*n)) == NULL)
        return -1;
        //Shuffle the pixels
    for(i=0;i<n;i++)
    {
        shuffled_pixels[permutation[i]] = pict->pixels[i];
    }

    memcpy(pict->pixels,shuffled_pixels,sizeof(pixel)*n);
    free(shuffled_pixels);
    //for(i = 0;i<n;i++)
    //   printf("%d %d %d||",shuffled_pixels[i].R,shuffled_pixels[i].G,shuffled_pixels[i].B);
    return 0;
}

void xor_pixels(picture *pict,uint * rand_arr,uint s_value,int n)
{
    int i;
       //Create pointer to each byte
    uchar * rand_byte=(uchar*) (rand_arr + n-1);
    uchar * s_val_byte = (uchar*) &s_value;
    pict->pixels[0].B = s_val_byte[0] ^ pict->pixels[0].B ^ rand_byte[0];
    pict->pixels[0].G = s_val_byte[1] ^ pict->pixels[0].G ^ rand_byte[1];
    pict->pixels[0].R = s_val_byte[2] ^ pict->pixels[0].R ^ rand_byte[2];
    for(i=1;i<n;i++)
    {
        rand_byte= (uchar*) (rand_arr + n + i + -1);
        pict->pixels[i].B = pict->pixels[i-1].B ^ pict->pixels[i].B ^ rand_byte[0];
        pict->pixels[i].G = pict->pixels[i-1].G ^ pict->pixels[i].G ^ rand_byte[1];
        pict->pixels[i].R = pict->pixels[i-1].R ^ pict->pixels[i].R ^ rand_byte[2];
    }

}

int read_key(char *path,uint *seed,uint *starting_value)
{
    //Read cipher key
    FILE * f = fopen(path,"r");
    if(f==NULL)
        return -1;
    fscanf(f,"%d%d",seed,starting_value);
    fclose(f);
    return 1;
}

void unxor_pixels(picture *pict,uint * rand_arr,uint s_value,int n)
{
    int i;
       //Create pointer to each byte
    uchar * rand_byte;
    uchar * s_val_byte = (uchar*) &s_value;

    for(i=n-1;i>0;i--)
    {
        rand_byte= (uchar*) (rand_arr + n + i + -1);
        pict->pixels[i].B = pict->pixels[i-1].B ^ pict->pixels[i].B ^ rand_byte[0];
        pict->pixels[i].G = pict->pixels[i-1].G ^ pict->pixels[i].G ^ rand_byte[1];
        pict->pixels[i].R = pict->pixels[i-1].R ^ pict->pixels[i].R ^ rand_byte[2];
    }
    rand_byte= (uchar*) (rand_arr + n -1);
    pict->pixels[0].B = s_val_byte[0] ^ pict->pixels[0].B ^ rand_byte[0];
    pict->pixels[0].G = s_val_byte[1] ^ pict->pixels[0].G ^ rand_byte[1];
    pict->pixels[0].R = s_val_byte[2] ^ pict->pixels[0].R ^ rand_byte[2];

}

int cipherPicture (char *path_pict,char* path_cript,char* path_key)
{
    //Open picture
    picture *pict=NULL;
    if((pict = read_picture(path_pict)) == NULL)
        return -1;
    //Read cipher key
    uint seed, s_value;
    read_key(path_key,&seed,&s_value);
    //Save image dimensions
    uint w = pict->W, h = pict->H;
    int n=w*h;
    //Create random array
    uint *rand_arr;
    if ((rand_arr = random_array(seed,n*2-1)) == NULL)
    {
        return -1;
    }
    //Create sigma to shuffle the array of pixels
    uint *permutation;
    if((permutation = random_permutation(n,rand_arr))== NULL)
        return -1;
    //Create shuffled picture
    shuffle_pixels(pict,permutation,n);
    //Cipher pixels
    xor_pixels(pict,rand_arr,s_value,n);
    //Write encrypted image
    return write_picture(pict,path_cript);
}

int decipherPicture (char *path_cript,char* path_decript,char* path_key)
{
    //Open image
    picture *pict=NULL;
    if((pict = read_picture(path_cript)) == NULL)
        return -1;
    //Read cipher key
    uint seed, s_value;
    read_key(path_key,&seed,&s_value);
    //Save image dimensions
    uint w = pict->W, h = pict->H;
    int n= w*h;
    //Create random array
    uint *rand_arr;
    if ((rand_arr = random_array(seed,n*2-1)) == NULL)
    {
        return -1;
    }
    //Create sigma to shuffle the array of pixels
    uint *permutation;
    if((permutation = random_permutation(n,rand_arr))== NULL)
        return -1;
        //Make sigma^-1
    uint *inv_permutation;
    if((inv_permutation = malloc(sizeof(uint)*n)) == NULL)
        return -1;
    int i;
    for(i=0;i<n;i++)
        inv_permutation[permutation[i]] = i;
    free(permutation);
    //Decipher pixels
    unxor_pixels(pict,rand_arr,s_value,n);
    //Create unshuffled picture
    shuffle_pixels(pict,inv_permutation,n);
    //Write decripted image
    return write_picture(pict,path_decript);
}



#endif // PICTENCRIPT_H_INCLUDED
