#include "SundancePartitionedRectangleMesher.hpp"
#include "SundanceOut.hpp"

using namespace Sundance;
using namespace Sundance;

using namespace Teuchos;
using namespace Sundance;

PartitionedRectangleMesher::PartitionedRectangleMesher(const ParameterList& params)
  : MeshSourceBase(params),
    ax_(params.get<double>("ax")),
    bx_(params.get<double>("bx")),
    nx_(params.get<int>("nx")),
    npx_(1),
    ay_(params.get<double>("ay")),
    by_(params.get<double>("by")),
    ny_(params.get<int>("ny")),
    npy_(1)
{
  if (params.isParameter("npx"))
    {
      npx_ = params.get<int>("npx");
    }
  if (params.isParameter("npy"))
    {
      npy_ = params.get<int>("npy");
    }
}


void PartitionedRectangleMesher::balanceXY(int n, int* npx, int* npy)
{
  int m = (int) floor(sqrt((double)n));
  for (int i=m; i>=1; i--)
  {
    if (n % i == 0) 
    {
      *npx = i;
      *npy = n/i;
      return ;
    }
  }

  *npx = n;
  *npy = 1;
}


Mesh PartitionedRectangleMesher::fillMesh() const
{
  SUNDANCE_OUT(this->verb() > 0,
               "PartitionedRectangleMesher::fillLocalMesh() is meshing "
               "rectangle [" << ax_ << ", " << bx_ << "] by ["
               << ay_ << ", " << by_ << "]");

  SUNDANCE_OUT(this->verb() >= 3,
               "PartitionedRectangleMesher::fillLocalMesh() starting creation "
               "of empty mesh");

  Mesh mesh = createMesh(2);

  SUNDANCE_OUT(this->verb() >= 3,
               "PartitionedRectangleMesher::fillLocalMesh() done creation of "
               "empty mesh");
  
  /* compute number of points per proc */

  int np = nProc();
	int rank = myRank();

	TEST_FOR_EXCEPTION(npx_ * npy_ != np, RuntimeError,
                     "PartitionedRectangleMesher::fillLocalMesh(): product "
                     "of npx=" << npx_ << " and npy=" << npy_
                     << " is not equal to np=" << np);

	/* compute number of points per proc */

	int nppx = nx_;
	int nppy = ny_;
  int nxTot = nx_*npx_;
  int nyTot = ny_*npy_;

	int px = rank/npy_;
	int py = rank % npy_;

	int lowestVisiblePtX = px*nppx-1;
	int lowestVisiblePtY = py*nppy-1;
	if (lowestVisiblePtX < 0) lowestVisiblePtX = 0;
	if (lowestVisiblePtY < 0) lowestVisiblePtY = 0;

	
	int highestVisiblePtX = lowestVisiblePtX + nppx + 1;
	int highestVisiblePtY = lowestVisiblePtY + nppy + 1;
	if (highestVisiblePtX > nxTot) highestVisiblePtX = nxTot;
	if (highestVisiblePtY > nyTot) highestVisiblePtY = nyTot;


	Array<Array<int> > pts(highestVisiblePtX-lowestVisiblePtX+1);
	for (int i=0; i<pts.size(); i++) pts[i].resize(highestVisiblePtY-lowestVisiblePtY+1);

	int globalIndex = 0;

	/* add the visible points into the mesh */
  for (int i=0; i<=nxTot; i++)
    {
      for (int j=0; j<=nyTot; j++, globalIndex++)
        {
					if (i < lowestVisiblePtX || i > highestVisiblePtX) continue;
					if (j < lowestVisiblePtY || j > highestVisiblePtY) continue;

					int ip = i/nppx;
					if (i==nxTot) ip--;
					int jp = j/nppy;
					if (j==nyTot) jp--;
					int pointOwner = ip*npy_ + jp;

          Point x( ax_ + ((double) i)*(bx_-ax_)/((double) nxTot) ,
                   ay_ + ((double) j)*(by_-ay_)/((double) nyTot));
          SUNDANCE_OUT(this->verb() > 1, "adding point GID=" 
                       << globalIndex << " x=" << x << " owner=" 
                       << pointOwner);
          int lid = mesh.addVertex(globalIndex, x, pointOwner, 0);
          pts[i-lowestVisiblePtX][j-lowestVisiblePtY] = globalIndex;
          
          SUNDANCE_OUT(this->verb() >=  3,
                       "point " << x << " registered with LID=" << lid);
        }
    }

	/* add the visible cells to the mesh */
	globalIndex = 0 ;

  for (int i=0; i<nxTot; i++)
    {
      for (int j=0; j<nyTot; j++, globalIndex+=2)
				{
					if (i < lowestVisiblePtX || i >= highestVisiblePtX) continue;
					if (j < lowestVisiblePtY || j >= highestVisiblePtY) continue;

					int a = pts[i-lowestVisiblePtX][j-lowestVisiblePtY];
					int b = pts[i+1-lowestVisiblePtX][j-lowestVisiblePtY];
					int c = pts[i+1-lowestVisiblePtX][j+1-lowestVisiblePtY];
					int d = pts[i-lowestVisiblePtX][j+1-lowestVisiblePtY];

					int ip = i/nppx;
					int jp = j/nppy;
					int cellOwner = ip*npy_ + jp;
          Array<int> tri1;
          Array<int> tri2;
					if (i%2 == j%2)
						{
              tri1 = tuple(a,b,c);
              tri2 = tuple(a,c,d);
						}
					else
						{
              tri1 = tuple(a,b,d);
              tri2 = tuple(b,c,d);
						}
          int lid1 = mesh.addElement(globalIndex, tri1, cellOwner, 0);
          SUNDANCE_OUT(this->verb() >=  3,
                       "elem " << tri1 
                       << " registered with LID=" << lid1);
          int lid2 = mesh.addElement(globalIndex+1, tri2, cellOwner, 0);
          SUNDANCE_OUT(this->verb() >=  3,
                       "elem " << tri2 
                       << " registered with LID=" << lid2);
          
				}
    }
  return mesh;
}
