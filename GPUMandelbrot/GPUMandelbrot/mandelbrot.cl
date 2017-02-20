typedef struct {
	unsigned char blue, green, red;
} mandelbrot_color;

__kernel void mandelbrot
(__global int* PARAMS,
__global mandelbrot_color* BITMAP,
__global mandelbrot_color* COLOR
)
{
	int windowPosX = get_global_id(0);
	int windowPosY = get_global_id(1);

	float x0 = (float)PARAMS[2];
	float y0 = (float)PARAMS[3];
	float stepsize = (float)1 / (float)PARAMS[4];
	unsigned int max_iterations = PARAMS[5];
	unsigned int window_width = PARAMS[0];
	unsigned int window_height = PARAMS[1];
	

	float center_X = -(stepsize*window_width / 2);
	float center_Y = (stepsize*window_height / 2);
	const float stepPosX = center_X - x0 + (windowPosX * stepsize);
	const float stepPosY = center_Y + y0 - (windowPosY * stepsize);
	
	// Variables for the calculation
	local float x = 0.0;
	local float y = 0.0;
	local float xSqr = 0.0;
	local float ySqr = 0.0;
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
	BITMAP[(window_width * windowPosY + windowPosX)] = (iterations == max_iterations) ? black : COLOR[iterations];

}
