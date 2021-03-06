//                                                                                 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA                                                                                
// Questions? Contact Kevin Long (krlong@sandia.gov), 
// Sandia National Laboratories, Livermore, California, USA
// 
// ************************************************************************
/* @HEADER@ */

#include "SundanceIQI_VecLF_Vi_Facet.hpp"

using namespace SundanceUtils;
using namespace SundanceStdMesh;
using namespace SundanceStdMesh::Internal;
using namespace SundanceStdFwk;
using namespace SundanceStdFwk::Internal;
using namespace SundanceCore;
using namespace SundanceCore::Internal;
using namespace Teuchos;

 IQI_VecLF_Vi_Facet:: IQI_VecLF_Vi_Facet( int spatialDim ,
					    const CellType & maxCellType ,
					    int dim ,
					    const CellType &cellType ,
					    const BasisFamily &testBasis ,
					    int component ,
					    const QuadratureFamily &quad ,
					    const ParameterList& verbParams ):
  ElementIntegralLinearFormFacet( spatialDim ,
				  maxCellType ,
				  dim ,
				  cellType ,
				  testBasis ,
				  quad ,
				  verbParams )
{
  TEST_FOR_EXCEPTION(true,
		     InternalError,
		     "IQI_HdivLF_Vi_Facet is not implemented" );
}

void IQI_VecLF_Vi_Facet::evaluate( CellJacobianBatch& JTrans,
				    CellJacobianBatch& JVol ,
				    const Array<int>& facetIndex ,
				    const double* const coeff,
				    RefCountPtr<Array<double> >& A) const
{
  TEST_FOR_EXCEPTION(true,
		     InternalError,
		     "IQI_HdivLF_Vi_Facet is not implemented" );
}

