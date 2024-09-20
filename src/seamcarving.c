#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "seamcarving.h"
#include "c_img.h"
#define MIN3(a, b, c) (((((a)< (b)) ? (a) : (b)) < (c)) ? (((a)< (b)) ? (a) : (b)) : (c))

#define R 0
#define G 1
#define B 2

#define SRC_IMG_PATH ROOT_DIR "/assets/img_sm_7.bin"

//////////////////////////////////////////
// 	     Pt 0 Debugging-functions       //
//////////////////////////////////////////
void print_mat(double * arr, int w, int h)
{
    for(int j = 0; j < h; j ++)
    {
        for (int i = 0; i < w; i ++)
        {
            printf("%g,     ", arr[i + j*w]);
        }
        printf("\n");
    }
}

void print_arr(int * arr, int len)
{
    for(int i = 0; i < len; i ++)
    {
        printf("%d, ", *(arr+i));
    }
}

//////////////////////////////////////////
// 	      Pt 1: Dual-grad energy        //
//////////////////////////////////////////
void calc_energy(struct rgb_img *im, struct rgb_img **grad)
{
    create_img(grad, im->height, im->width);
    int height = (int) im->height;
    int width  = (int) im->width;

    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            // printf("(y, x) : (%d, %d) ", y, x);
            int lx = (x != 0 ? x-1 : width-1); // left x
            int rx = (x != width-1 ? x+1 : 0); // right x
            int ty = (y != 0 ? y-1 : height-1); // bottom y
            int by = (y != height-1 ? y+1 : 0); // top y
            // printf("l-x: %d, r-x: %d, t-y: %d, b-y: %d\n", lx, rx, ty, by);
            int r_x = get_pixel(im, y, rx, R)-get_pixel(im, y, lx, R);
            int g_x = get_pixel(im, y, rx, G)-get_pixel(im, y, lx, G);
            int b_x = get_pixel(im, y, rx, B)-get_pixel(im, y, lx, B);

            int r_y = get_pixel(im, by, x, R)-get_pixel(im, ty, x, R);
            int g_y = get_pixel(im, by, x, G)-get_pixel(im, ty, x, G);
            int b_y = get_pixel(im, by, x, B)-get_pixel(im, ty, x, B);

            double delta_x = (double)(r_x*r_x + g_x*g_x + b_x*b_x);
            double delta_y = (double)(r_y*r_y + g_y*g_y + b_y*b_y);

            uint8_t energy = (uint8_t)(sqrt(delta_x + delta_y)/10);
            //printf("%d, ", energy);
            set_pixel(*grad, y, x, energy, energy, energy);
        }
        // printf("\n");
    }   
}


//////////////////////////////////////////
// 	     Pt 2 Dynamic seam matrix       //
//////////////////////////////////////////
void dynamic_seam(struct rgb_img *grad, double **best_arr)
{
    int width = (int)(grad->width);
    int height = (int)(grad->height);

    *best_arr = (double *)malloc(sizeof(double)*width*height);

    // SETS FIRST ROW OF BEST ARR TO THE TOP ROW OF GRAD
    for(int x = 0; x < grad->width; x ++)
    {
        //printf("pixel: %g\n",(double)get_pixel(grad, 0, x, R));
        (*best_arr)[x] = (double) get_pixel(grad, 0, x, R);
        // printf("%g, ",(*best_arr)[x]);
    }
    // printf("\n");

    for(int y = 1; y < grad->height; y ++)
    {
        for(int x = 0; x < grad->width; x ++)
        {
            double cur_pixel = (double) get_pixel(grad, y, x, B);
            int left = (x != 0 ? x - 1 : x);
            int center = x; 
            int right = (x != grad->width-1 ? x + 1 : x);

            *(*best_arr + (y*(grad->width) + x)) = MIN3(*(*best_arr + ((y-1)*(grad->width) + left)), 
                                                        *(*best_arr + ((y-1)*(grad->width) + center)), 
                                                        *(*best_arr + ((y-1)*(grad->width) + right)))
                                                        + cur_pixel;
            // printf("%g,  ", *(*best_arr + (y*(grad->width) + x)));                                               
        }
        // printf("\n");
    }
}


//////////////////////////////////////////
// 	     Pt 3: best path recovery       //
//////////////////////////////////////////
void recover_path(double *best, int height, int width, int **path)
{
    // Go to last row and find min
    // in a loop go to prev row selecting the min adjacent
    // stop loop at height of 0 
    int *best_path = (int *)malloc(sizeof(int)*height);

    //Find min in last row
    double cur_min = *(best + (height-1)*width) + 300;
    // best_path[0] = 0; 
    for(int x = 0; x < width; x ++)
    {
        //best_path[0] = 1;
        //cur_min = (*(best + (height-1)*width + x) < cur_min ? *(best + (height-1)*width + x) : cur_min);
        if(*(best + (height-1)*width + x) < cur_min)
        {
            best_path[0] = x;
            cur_min = *(best + (height-1)*width + x);
        }
    }

    //rows above final row:
    int i = 0;
    for(int y = height - 2; y >= 0; y --)
    {
        cur_min = *(best + (height-1)*width) + 300; // set cur_min to first elem in bottom row + an arbitrarilly large number. 
                                                    // All higher up rows should have values smaller or equal to the smallest value in the bottom row 
        int x = (*(best_path + i) != 0 ? *(best_path + i) - 1 : 0);                       // sets starting x to 1 less than last path x
        int x_upper = (*(best_path + i) != width - 1 ? *(best_path + i) + 1 : width - 1); // sets final x to check to 1 more than last path x
        
        i ++; // to update next index of path
        for (; x <= x_upper; x ++)
        {
            if(best[(y*width) + x] < cur_min)
            {
                cur_min = *(best + y*width + x);
                *(best_path + i) = x;
            }
        }
    }
    // best path gives the correct path in reverse order
    // must now reverse its order and store it in path_top_to_bot

    int *path_top_to_bot = (int *)malloc(sizeof(int)*height);

    int j = 0;
    for(int i = height - 1; i >= 0; i --)
    {
        path_top_to_bot[j] = best_path[i];
        j ++;
    }
    
    *path = path_top_to_bot;

    free(best_path);
}

//////////////////////////////////////////
// 	        Pt 4: Seam-removal          //
//////////////////////////////////////////
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path)
{
    //
    create_img(dest, src->height, src->width - 1);
    for(int y = 0; y < src->height; y ++)
    {
        int x_dest = 0;
        for(int x = 0; x < src->width; x ++)
        {
            if(x != path[y])
            {
                int cur_R = (int)get_pixel(src, y, x, R);
                int cur_G = (int)get_pixel(src, y, x, G);
                int cur_B = (int)get_pixel(src, y, x, B);

                set_pixel(*dest, y, x_dest, cur_R, cur_G, cur_B);

                x_dest ++;
            }
        }
    }  
}


int main()
{
	struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;
    char im_name[256];
    read_in_img(&im, SRC_IMG_PATH);


    
    for(int i = 0; i < 201; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        // char filename[200];
        // sprintf(filename, "img%d.bin", i);
        // write_img(cur_im, filename);

        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
        if (i % 10 == 0) {
            sprintf(im_name, "%s/img_out/img%03d.bin", ROOT_DIR, i);
            printf("%s", im_name);
            write_img(im, im_name);
        }
    }
    
    printf("END\n");
    destroy_image(im);
    return 0;
    // struct rgb_img **president_p = (struct rgb_img **)malloc(sizeof(struct rgb_img));
    // struct rgb_img **grad = (struct rgb_img **)malloc(sizeof(struct rgb_img));
    
    // char *filename = "C:/Users/halil/Desktop/ESC190/C C++/president.bin";
    // read_in_img(president_p, filename);
    // create_img(grad, (*president_p)->height, (*president_p)->width);
    // struct rgb_img *image = *president_p;
    // printf("test2\n");
    // calc_energy(image, grad);
    // //print_grad(image);
    // // printf("test3\n");
    // write_img(*grad, "C:/Users/halil/Desktop/ESC190/C C++/president_energy2.bin");
    // // printf("test4\n");
    // destroy_image(image);
    // destroy_image(*grad);
    // free(president_p);
    // free(grad);
   
    // struct rgb_img **six_five_p = (struct rgb_img **)malloc(sizeof(struct rgb_img));
    // struct rgb_img **grad = (struct rgb_img **)malloc(sizeof(struct rgb_img));
    // struct rgb_img **dest = (struct rgb_img **)malloc(sizeof(struct rgb_img));

    // char *filename = "C:/Users/halil/Desktop/ESC190/C C++/ESC190-labs/src/Project 2/6x5.bin";
    // read_in_img(six_five_p, filename);
    // create_img(grad, (*six_five_p)->height, (*six_five_p)->width);
    // struct rgb_img *image = *six_five_p;
    // double **best_arr = (double **)malloc(sizeof(double)*(int)(image->width)*(int)(image->height));
    // int **path = (int **)malloc(sizeof(int)*image->height);

    
    // printf("test2\n");
    // calc_energy(image, grad);
    // //print_grad(image);
    // // write_img(*grad, "C:/Users/halil/Desktop/ESC190/C C++/6x5ENERGY.bin");

    // printf("test3\n");
    // dynamic_seam(*grad, best_arr);
    // printf("GRAD \n");
    // print_grad(*grad);
    // printf("\nBEST ARR\n");
    // print_mat(*best_arr, image->width, image->height);
    // printf("test4\n");
    
    // recover_path(*best_arr, image->height, image->width, path);
    // print_arr(*path, image->height);

    // remove_seam(image, dest, *path);

    // write_img(*grad, "C:/Users/halil/Desktop/ESC190/C C++/6x5_seam_rem.bin");



    // destroy_image(image);
    // destroy_image(*grad);
    // free(six_five_p);
    // free(grad);
    // free(*best_arr);
    // free(best_arr);
    // destroy_image(*dest);
    // free(dest);
}



//////////////////////////////////
//           Old Code           //
//////////////////////////////////
/**
 * Note:
 * 	rgb_img raster pixels are stored as one continuous array
 *		y is given by i / width
 *		x is given by i % width
 * 	where i represents each pixel
 * 	color sub-pixel channels are accessed 
 * 	by indexing in the following way:
 * 		raster[3*i + color], where color = 0, 1, 2 (for r, g, b)
 **/

///////////////////////////////////////////
// Part 1: Dual-Gradient Energy Function //
///////////////////////////////////////////

// /**
//  * TODO: change pixel energy to take a single index value, rather than x and y
//  **/

// // static double pixel_energy(const struct rgb_img *im, int i)
// // {
// // 	int rx, ry, gx, gy, bx, by;
// // 	int width = im->width;
// // 	int height = im->height;
// // 	// NOTE: can also allow the index to increment to the next row (reduces the number of checks)
// // 	/* set the adjacent pixel coordinates, wrapping around to the other
// // 	side of the image if the coordinate is outside of the allowed range */
// // 	int x_1 = i - 1;
// // 	if(i % width == 0)
// // 		x_1 += width;
// // 	int x_2 = i + 1;
// // 	if(x_2 % width == 0)
// // 		x_2 -= width;
// // 	int y_1 = i - width;
// // 	if(y_1 < 0)
// // 		y_1 += width * height;
// // 	int y_2 = i + width;
// // 	if(y_2 >= width * height)
// // 		y_2 -= width * height;

// // 	rx = im->raster[3 * x_2 + RED] - im->raster[3 * x_1 + RED];
// // 	gx = im->raster[3 * x_2 + GREEN] - im->raster[3 * x_1 + GREEN];
// // 	bx = im->raster[3 * x_2 + BLUE] - im->raster[3 * x_1 + BLUE];
// // 	ry = im->raster[3 * y_2 + RED] - im->raster[3 * y_1 + RED];
// // 	gy = im->raster[3 * y_2 + GREEN] - im->raster[3 * y_1 + GREEN];
// // 	by = im->raster[3 * y_2 + BLUE] - im->raster[3 * y_1 + BLUE];

// // 	// TODO: convert function to use integers and return without performing square root
// // 	return sqrt(rx*rx + ry*ry + gx*gx + gy*gy + bx*bx + by*by); // is it even necessary to square-root
// // }

// // void calc_energy(struct rgb_img *im, struct rgb_img **grad)
// // {
// // 	*grad = (struct rgb_img*) malloc(sizeof(struct rgb_img));
// // 	(*grad)->raster = (uint8_t*) malloc(sizeof(uint8_t) * im->width * im->height);
// // 	(*grad)->width = im->width;
// // 	(*grad)->height = im->height;
// // 	for(int i = 0; i < (*grad)->height * (*grad)->width; i++)
// // 	{
// // 		(*grad)->raster[i] = (uint8_t) (0.1 * pixel_energy(im, i));
// // 	}
// // }


// ////////////////////////
// // Part 2: Cost Array //
// ////////////////////////

// void dynamic_seam(struct rgb_img *grad, double **best_arr)
// {
// 	double* partial_seam_energy = (double*) malloc(sizeof(double) * grad->width * grad->height);
// 	// NOTE
// 	// x = i % width;
// 	// y = i / width;
// 	int width = grad->width;
// 	int height = grad->height;

// 	// CASE 1: first row of tiles: seam energy == tile energy
// 	int i = 0;
// 	for(; i < width ; i++)
// 	{
// 		partial_seam_energy[i] = grad->raster[i];
// 	}

// 	int min_prev_seam;
// 	for(; i < width * height; i++)
// 	{
// 		// CASE 2: first column of tiles (no left ) 
// 		if(i % width == 0)
// 		{
// 			// set the tile seam energy to the smaller of the above seam energies + the tile energy
// 			partial_seam_energy[i] = 
// 				partial_seam_energy[i - width] <= partial_seam_energy[i - width + 1] ? 
// 				grad->raster[i] + partial_seam_energy[i - width] :
// 				grad->raster[i] + partial_seam_energy[i - width + 1];
// 			continue;
// 		}

// 		// CASE 3: last column of tiles (no right)
// 		if((i+1) % width == 0)
// 		{
// 			partial_seam_energy[i] = 
// 				partial_seam_energy[i - width - 1] <= partial_seam_energy[i - width] ? 
// 				grad->raster[i] + partial_seam_energy[i - width - 1] :
// 				grad->raster[i] + partial_seam_energy[i - width];
// 			continue;
// 		}

// 		// CASE 4: any other column (all three tiles above are valid comparisons)
// 		min_prev_seam = partial_seam_energy[i - width - 1];
// 		if(min_prev_seam < partial_seam_energy[i - width])
// 			min_prev_seam = partial_seam_energy[i - width];
// 		if(min_prev_seam < partial_seam_energy[i - width + 1])
// 			min_prev_seam = partial_seam_energy[i - width + 1];

// 		partial_seam_energy[i] = grad->raster[i] + min_prev_seam;

// 	}

// 	*best_arr = partial_seam_energy;
// }

// void recover_path(double *best, int height, int width, int **path);
// void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path);

// void print_mat(double *arr, int w, int h)
// {
//     for (int i = 0; i < w; i ++)
//     {
//         for(int j = 0; j < h; j ++)
//         {
//             printf("%d, ", arr[i + j*w]);
//         }
//         printf("\n");
//     }
// }
