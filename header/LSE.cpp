
#include "LSE.h"
#include "vertex.h"
#include "gvector.h"
#include "utils.h"


using namespace std;

extern vertex Zero;



//https://www.geeksforgeeks.org/gaussian-elimination/

void SetMat(double mat[3][4], face Faces[3])
{
	for(int i=0; i<3; i++)
	{
		face &Face = Faces[i];
		vertex Zero;
		double d = GetAdjaLen(GetVector(Zero,Face.Vertices[0]),Face.Normal);
		mat[i][0] = Face.Normal.x;
		mat[i][1] = Face.Normal.y;
		mat[i][2] = Face.Normal.z;
		mat[i][3] = d;
	}
}

void SetMat(double mat[3][4], face &F1, face &F2, face &F3)
{
	for(int i=0; i<3; i++)
	{
		face *Face_Ptr;
		if(i==0) Face_Ptr = &F1;
		else if(i==1) Face_Ptr = &F2;
		else if(i==2) Face_Ptr = &F3;
		face &Face = *Face_Ptr;
		vertex Zero;
		double d = GetAdjaLen(GetVector(Zero,Face.Vertices[0]),Face.Normal);
		mat[i][0] = Face.Normal.x;
		mat[i][1] = Face.Normal.y;
		mat[i][2] = Face.Normal.z;
		mat[i][3] = d;
	}
}


// function to get matrix content 
bool gaussianElimination(double mat[N][N+1], vertex &Isect)
{ 
    /* reduction into r.e.f. */
    int singular_flag;
    for (int i=0; i<5; i++)
    {
    	singular_flag = forwardElim(mat);
    	if (singular_flag == -1) break;
    	else
    	{
    		if(i % 2 == 0)
    		for(int j=0;j<4;j++)
    			swap(mat[1][j] , mat[2][j]);
    		else
    		for(int j=0;j<4;j++)
    			swap(mat[0][j] , mat[2][j]);
    	}
    }
  
    /* if matrix is singular */
    if (singular_flag != -1)
    {
		#if DEBUG > 0
    	bool dev = 0;
    	
        if(dev)printf("Singular Matrix.\n"); 
		#endif
  
        /* if the RHS of equation corresponding to 
           zero row  is 0, * system has infinitely 
           many solutions, else inconsistent*/

		#if DEBUG > 0
        if(dev)
		if (mat[singular_flag][N]) 
            printf("Inconsistent System."); 
        else
            printf("May have infinitely many "
                   "solutions.");
		#endif
		
    	return 0;
    } 
  
    /* get solution to system and print it using 
       backward substitution */
	backSub(mat, Isect);
    
    return 1;
}


// function to calculate the values of the unknowns
void backSub(double mat[N][N+1], vertex &Isect)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
    double x[N];  // An array to store solution
    
    /* Start calculating from last equation up to the
       first */
    for (int i = N-1; i >= 0; i--)
    {
        /* start with the RHS of the equation */
        x[i] = mat[i][N]; 
  
        /* Initialize j to i+1 since matrix is upper 
           triangular*/
        for (int j=i+1; j<N; j++) 
        { 
            /* subtract all the lhs values 
             * except the coefficient of the variable 
             * whose value is being calculated */
            x[i] -= mat[i][j]*x[j]; 
        } 
  
        /* divide the RHS by the coefficient of the 
           unknown being calculated */
        x[i] = x[i]/mat[i][i]; 
    }
    
	#if DEBUG > 0
    if(dev) {
    printf("\nSolution for the system:\n"); 
    for (int i=0; i<N; i++) 
        printf("%lf\n", x[i]);
    }
	#endif
    
    Isect.setall(x[0],x[1],x[2]);
}


// function for elementary operation of swapping two rows 
void swap_row(double mat[N][N+1], int i, int j) 
{ 
    //printf("Swapped rows %d and %d\n", i, j); 
  
    for (int k=0; k<=N; k++) 
    { 
        double temp = mat[i][k]; 
        mat[i][k] = mat[j][k]; 
        mat[j][k] = temp; 
    } 
} 

// function to print matrix content at any stage 
void print(double mat[N][N+1]) 
{ 
    for (int i=0; i<N; i++, printf("\n")) 
        for (int j=0; j<=N; j++) 
            printf("%lf ", mat[i][j]); 
  
    printf("\n"); 
} 

// function to reduce matrix to r.e.f. 
int forwardElim(double mat[N][N+1]) 
{ 
    for (int k=0; k<N; k++) 
    { 
        // Initialize maximum value and index for pivot 
        int i_max = k; 
        int v_max = mat[i_max][k]; 
  
        /* find greater amplitude for pivot if any */
        for (int i = k+1; i < N; i++) 
            if (abs(mat[i][k]) > v_max) 
                v_max = mat[i][k], i_max = i; 
  
        /* if a prinicipal diagonal element  is zero, 
         * it denotes that matrix is singular, and 
         * will lead to a division-by-zero later. */
        if (!mat[k][i_max]) 
            return k; // Matrix is singular 
  
        /* Swap the greatest value row with current row */
        if (i_max != k) 
            swap_row(mat, k, i_max); 
  
  
        for (int i=k+1; i<N; i++) 
        { 
            /* factor f to set current row kth element to 0, 
             * and subsequently remaining kth column to 0 */
            double f = mat[i][k]/mat[k][k]; 
  
            /* subtract fth multiple of corresponding kth 
               row element*/
            for (int j=k+1; j<=N; j++) 
                mat[i][j] -= mat[k][j]*f; 
  
            /* filling lower triangular matrix with zeros*/
            mat[i][k] = 0; 
        } 
  
        //print(mat);        //for matrix state 
    } 
    //print(mat);            //for matrix state 
    return -1; 
} 





