typedef struct {
	float blue, green, red;
} mandelbrot_color;

__kernel void mandelbrot
(__global float* PARAMS,
__write_only image2d_t BITMAP,
__global mandelbrot_color* COLOR,
__global float* STPSIZE
)
{
	int windowPosX = get_global_id(0);
	int windowPosY = get_global_id(1);

	double x0 = (double)PARAMS[2];
	double y0 = (double)PARAMS[3];
	double stepsize = (double)*STPSIZE;
	unsigned int max_iterations = (unsigned int)PARAMS[4];
	unsigned int window_width = (unsigned int)PARAMS[0];
	unsigned int window_height = (unsigned int)PARAMS[1];
	

	double center_X = -(stepsize*window_width / 2);
	double center_Y = (stepsize*window_height / 2);
	const double stepPosX = center_X - x0 + (windowPosX * stepsize);
	const double stepPosY = center_Y + y0 - (windowPosY * stepsize);
	
	// Variables for the calculation
	__local double x, y, xSqr, ySqr;
	x = 0.0;
	y = 0.0;
	xSqr = 0.0;
	ySqr = 0.0;
	unsigned int iterations = 0;

	// Perform up to the maximum number of iterations to solve
	// the current pixel's position in the image
	while ((xSqr + ySqr < 4.0) && (iterations < max_iterations))
	{
		// Perform the current iteration
		xSqr = x*x;
		ySqr = y*y;

		y = 2 * x*y + stepPosY;
		x = xSqr - ySqr + stepPosX;

		// Increment iteration count
		iterations++;
	}

	// Output black if we never finished, and a color from the look up table otherwise
	mandelbrot_color black = { 0,0,0 };

	mandelbrot_color output = (iterations == max_iterations) ? black : COLOR[iterations];

	int2 coord = (int2)(windowPosX, windowPosY);
	float4 color_yellow = (float4)(output.red / 255, output.green / 255, output.blue / 255, 1.0);
	write_imagef(BITMAP, coord, color_yellow);


}
