#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <math.h>

#define ERROR 1
#define MAX_ITERATION 2000


void setPixel(png_byte *ptr, png_byte* colors, int iterations, double zn);
double PixelCoordinatesToComplexPlaneCoordinates(int axis, double axis_center, double zoom, int width);
void setUpColors(png_byte* colors);
int TestMandelbrotConvergence(double c_re, double c_im, double *zn);
int Clamp(int i);
void HsvToRgb(double h, double S, double V, int *r, int *g, int *b);

int main(int argc, char const *argv[])
{

	// apertura file	
	FILE *file = fopen(argv[1], "w");
	if( file != NULL ){
		printf("File creato con successo\n");
	}else{
		printf("Impossibile creare il file\n");
		return EXIT_FAILURE;
	}

	png_structp str = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!str)
		return EXIT_FAILURE;

	png_infop info = png_create_info_struct(str);	
	if(!info){
		png_destroy_write_struct(&str, NULL);
		return EXIT_FAILURE;
	}

	// error handling
	if( setjmp(png_jmpbuf(str)) ){
		png_destroy_write_struct(&str, NULL);
		fclose(file);
		return EXIT_FAILURE;
	}	

	png_init_io(str, file);

	int width = 1920, height = 1080, bit_depth = 8;

	png_set_IHDR(str,
				 info,
				 width,
				 height, 
				 bit_depth,
				 PNG_COLOR_TYPE_RGB,
				 PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE,
				 PNG_FILTER_TYPE_BASE);

	png_write_info(str, info);

	png_byte colors[16*3];
	setUpColors(colors);

	png_bytep row = 0, col = 0;
	int row_len = 3 * width;
	row = (png_bytep) calloc( 3 * width , sizeof(png_byte) );
	double c_re = 0.0;
	double c_im = 0.0;

	double shift_x = -0.5;
	double shift_y = -0.0;
	double zoom = 4.5;
/*	
	double shift_x = -0.51;
	double shift_y = -0.620;	
	double zoom = 0.13;
*/
	int iteration = 0;
	
	double x_center = width / 2.0;
	double y_center = height / 2.0;

	double nsmooth = 0.0;
	double zn = 0.0;

	printf("Inizio scrittura,\n");
	int x,y,z;
	for(x=0; x < height; x++){
		for(y=0; y < width; y++){
			c_re = PixelCoordinatesToComplexPlaneCoordinates(y, x_center, zoom, width);
			c_im = PixelCoordinatesToComplexPlaneCoordinates(x, y_center, zoom, width);
			c_re += shift_x;
			c_im += shift_y;

			iteration = TestMandelbrotConvergence(c_re, c_im, &zn);
			setPixel( &(row[y*3]), colors, iteration, zn );
			iteration = 0;			
		}
		png_write_row(str, row);
	}

	png_write_end(str, NULL);

printf("fine scrittura\n");
	//chiusura file
	if( file != NULL )
		fclose(file);
	if( info != NULL)
		png_free_data(str, info, PNG_FREE_ALL, -1);
	if(	str != NULL)
		png_destroy_write_struct(&str, (png_infopp) NULL);
	if ( row != NULL)
		free(row);

	return EXIT_SUCCESS;
}

double PixelCoordinatesToComplexPlaneCoordinates(int axis, double axis_center, double zoom, int width) {
	return 	(axis - axis_center) * zoom / width;
}

void setPixel(png_byte *ptr, png_byte* colors, int iterations, double zn){
	
	double nsmooth = 0.0;
	double x = 0.0;

	if (iterations < MAX_ITERATION -1 ) {

		nsmooth = log( iterations + 2 - log( log(zn)/log(2) )) / 4.0;
		if(nsmooth < 1.0){
			//blue
			ptr[0] = nsmooth * nsmooth * nsmooth * nsmooth * 255;
			ptr[1] = nsmooth * nsmooth * 255;
			ptr[2] = nsmooth * 255;	
		}else{

			x = 2.0 - nsmooth;
			if( x > 0.0){
				nsmooth = x;
			}else{
				nsmooth = 0.0;
			}
			//sepia ???
			ptr[0] = nsmooth * 255;
			ptr[1] = nsmooth * 255;
			ptr[2] = nsmooth * nsmooth * 255;
		}

	}else {
		//black
		ptr[0] = 0;
		ptr[1] = 0;
		ptr[2] = 0;		
	}
}



void setRGB(png_byte *color, int red, int green, int blue){
	color[0] = red;
	color[1] = green;
	color[2] = blue;
}


void setUpColors(png_byte* colors){
	setRGB( &(colors[0]), 66, 30, 15);
	setRGB( &(colors[3]), 25, 7, 26);
	setRGB( &(colors[6]), 9, 1, 47);
	setRGB( &(colors[9]), 4, 4, 73);
	setRGB( &(colors[12]), 0, 7, 100);
	setRGB( &(colors[15]), 12, 44, 138);
	setRGB( &(colors[18]), 24, 82, 177);
	setRGB( &(colors[21]), 57, 125, 209);
	setRGB( &(colors[24]), 134, 181, 229);
	setRGB( &(colors[27]), 211, 236, 248);
	setRGB( &(colors[30]), 241, 233, 191);
	setRGB( &(colors[33]), 248, 201, 95);
	setRGB( &(colors[36]), 255, 170, 0);
	setRGB( &(colors[39]), 204, 120, 0);
	setRGB( &(colors[42]), 253, 87, 0);
	setRGB( &(colors[45]), 106, 52, 3);
}

int TestMandelbrotConvergence(double c_re, double c_im, double *zn){
	int iteration = 0;
	double x = 0.0, y = 0.0, x_new = 0.0;

	while (((x*x) + (y*y) < 20) && (iteration < MAX_ITERATION)) {
		x_new = (x * x) - (y * y) + c_re;
		y = (2 * x*y) + c_im;
		x = x_new;
		iteration++;

	}
	//absolute value of last zn
	*zn = sqrt( x*x + y*y );
	return iteration;
}


