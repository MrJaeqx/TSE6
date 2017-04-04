typedef struct {
	float topLeftx, topLefty, topRightx, topRighty;
} pythagoras_coords;

__kernel void pythagoras_single(
	__global pythagoras_coords *INPUT,
	__global pythagoras_coords *OUTPUT_L,
	__global pythagoras_coords *OUTPUT_R
)
{
	int x = get_global_id(0);

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

	OUTPUT_L[x].topLeftx = Q4x;
	OUTPUT_L[x].topLefty = Q4y;
	OUTPUT_L[x].topRightx = newx;
	OUTPUT_L[x].topRighty = newy;

	OUTPUT_R[x].topLeftx = newx;
	OUTPUT_R[x].topLefty = newy;
	OUTPUT_R[x].topRightx = Q3x;
	OUTPUT_R[x].topRighty = Q3y;
}

__kernel void pythagoras_multiple(
	__global pythagoras_coords *INPUT
)
{
	int x = get_global_id(0) * 2;

	//1st point						top left corner
	float Q1x = INPUT[get_global_id(0)].topLeftx;
	float Q1y = INPUT[get_global_id(0)].topLefty;

	//2nd point						top right corner
	float Q2x = INPUT[get_global_id(0)].topRightx;
	float Q2y = INPUT[get_global_id(0)].topRighty;

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

	barrier(CLK_GLOBAL_MEM_FENCE);

	INPUT[x].topLeftx = Q4x;
	INPUT[x].topLefty = Q4y;
	INPUT[x].topRightx = newx;
	INPUT[x].topRighty = newy;

	INPUT[x+1].topLeftx = newx;
	INPUT[x+1].topLefty = newy;
	INPUT[x+1].topRightx = Q3x;
	INPUT[x+1].topRighty = Q3y;
}
