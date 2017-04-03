typedef struct {
	float topLeftx, topLefty, topRightx, topRighty;
} pythagoras_coords;

__kernel void pythagoras(
	__global pythagoras_coords *INPUT,
	__global pythagoras_coords *OUTPUT_L,
	__global pythagoras_coords *OUTPUT_R
	)
{
	//1st point						top left corner
	float Q1x = INPUT->topLeftx;
	float Q1y = INPUT->topLefty;

	//2nd point						top right corner
	float Q2x = INPUT->topRightx;
	float Q2y = INPUT->topRighty;

	//3rd point						bottom right corner
	float Q3x = Q2x - (Q1y - Q2y);
	float Q3y = Q2y + (Q1x - Q2x);

	//4th point						bottom left corner
	float Q4x = (Q1x - Q2x) + Q3x;
	float Q4y = (Q1y - Q2y) + Q3y;

	//length of a side of the square
	float dist = sqrt(((Q1x - Q2x)*(Q1x - Q2x)) + ((Q1y - Q2y)*(Q1y - Q2y)));

	//calculate begining of next square
	float ACx = (Q3x - Q1x) / 2;	//half of the diagonal
	float ACy = (Q3y - Q1y) / 2;
	float newx = ACx + Q4x;		// point of iscoceles right triangle
	float newy = ACy + Q4y;		// off of the bottom of the square

	OUTPUT_L->topLeftx = Q4x;
	OUTPUT_L->topLefty = Q4y;
	OUTPUT_L->topRightx = newx;
	OUTPUT_L->topRighty = newy;

	OUTPUT_R->topLeftx = newx;
	OUTPUT_R->topLefty = newy;
	OUTPUT_R->topRightx = Q3x;
	OUTPUT_R->topRighty = Q3y;
	/*OUTPUT->bottomRightx = Q3x,
	OUTPUT->bottomRighty = Q3y,
	OUTPUT->bottomLeftx = Q4x,
	OUTPUT->bottomLefty = Q4y,
	OUTPUT->newX = newx;
	OUTPUT->newY = newy;*/
}
