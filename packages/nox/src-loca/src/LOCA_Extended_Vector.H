// $Id$ 
// $Source$ 

//@HEADER
// ************************************************************************
// 
//            NOX: An Object-Oriented Nonlinear Solver Package
//                 Copyright (2002) Sandia Corporation
// 
//            LOCA: Library of Continuation Algorithms Package
//                 Copyright (2005) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// 
// Questions? Contact Roger Pawlowski (rppawlo@sandia.gov) or 
// Eric Phipps (etphipp@sandia.gov), Sandia National Laboratories.
// ************************************************************************
//  CVS Information
//  $Source$
//  $Author$
//  $Date$
//  $Revision$
// ************************************************************************
//@HEADER

#ifndef LOCA_EXTENDED_VECTOR_H
#define LOCA_EXTENDED_VECTOR_H

#include "NOX_Common.H"  // for <iostream>, <vector>, and <string>
#include "NOX_Abstract_Vector.H"  // Base class
#include "NOX_Abstract_MultiVector.H" // for DenseMatrix

// forward declarations
namespace LOCA {
  class GlobalData;
  namespace Extended {
    class MultiVector;
  }
}

namespace LOCA {

  namespace Extended {

    /*! 
      \brief Implemenatation of the NOX::Abstract::Vector class for extended
      vectors comprised of an arbitrary number of vectors and scalars.
    */
    /*!
      Many continuation and bifurcation calculations can be viewed as the
      solution to an extended set of equations.  For example, calculating 
      a turning point can be viewed as computing a solution to 
      \f$G(z) = 0\f$ where \f$z = [x, n, p]\in\Re^{2n+1}\f$ and 
      \f[
      G(z) = \left[
      \begin{array}{c}
      F(x,p) \\
      Jn \\
      n^Tn-1
      \end{array}
      \right]
      \f]
      The extended vector \f$z\f$ is comprised of the two vectors \f$x\f$ and 
      \f$n\f$ as well as the scalar \f$p\f$.  This class provides
      an implementation of the NOX::Abstract::Vector interface for such 
      extended vectors.  It stores an array of pointers to 
      NOX::Abstract::Vector's as well as an array of scalars using the 
      STL vector class.

      The implementations of the NOX::Abstract::Vector methods are defined
      in terms of the implementations of each stored abstract vector.
    */

    class Vector : public NOX::Abstract::Vector {

      //! Declare LOCA::Extended::MultiVector as a friend class
      friend class LOCA::Extended::MultiVector;

    public:

      //! Copy constructor.
      Vector(const Vector& source, NOX::CopyType type = NOX::DeepCopy);

      //! Vector destructor.
      virtual ~Vector();

      //! Assignment operator
      virtual NOX::Abstract::Vector& 
      operator=(const NOX::Abstract::Vector& y);
  
      //! Assignment operator
      virtual Vector& 
      operator=(const Vector& y);

      //! Clone function.  Applies clone to each stored vector
      virtual Teuchos::RCP<NOX::Abstract::Vector>
      clone(NOX::CopyType type = NOX::DeepCopy) const;

      /*! 
       * \brief Create a MultiVector with \c numVecs+1 columns out of an array 
       * of Vectors.  The vector stored under \c this will be the first 
       * column with the remaining \c numVecs columns given by \c vecs.
       */
      virtual Teuchos::RCP<NOX::Abstract::MultiVector>
      createMultiVector(const NOX::Abstract::Vector* const* vecs,
			int numVecs, NOX::CopyType type = NOX::DeepCopy) const;

      /*! 
       * \brief Create a MultiVector with \c numVecs columns.
       */
      virtual Teuchos::RCP<NOX::Abstract::MultiVector>
      createMultiVector(int numVecs, NOX::CopyType type = NOX::DeepCopy) const;

      /*! 
       * \brief NOX::Abstract::Vector init function. Initializes each 
       * stored vector and scalar
       */
      virtual NOX::Abstract::Vector& 
      init(double gamma);

      //! Initialize every element of this vector with random values
      virtual NOX::Abstract::Vector& 
      random(bool useSeed = false, int seed = 1);

      /*! 
       * \brief NOX::Abstract::Vector abs function.  Compues absolute value 
       * of each stored vector and scalar
       */
      virtual NOX::Abstract::Vector& 
      abs(const NOX::Abstract::Vector& y);

      /*! 
       * \brief NOX::Abstract::Vector reciprocal function.  Computes 
       * reciprocal of each stored vector and scalar
       */
      virtual NOX::Abstract::Vector& 
      reciprocal(const NOX::Abstract::Vector& y);

      /*! 
       * \brief NOX::Abstract::Vector scale function.  Scales each stored 
       * vector and scalar
       */
      virtual NOX::Abstract::Vector& 
      scale(double gamma);

      /*! 
       * \brief NOX::Abstract::Vector scale function.  Scales each stored 
       * vector and scalar
       */
      virtual NOX::Abstract::Vector& 
      scale(const NOX::Abstract::Vector& a);

      /*! 
       * \brief NOX::Abstract::Vector update function.  Applies vector 
       * update to each stored vector and scalar
       */
      virtual NOX::Abstract::Vector& 
      update(double alpha, const NOX::Abstract::Vector& a,
	     double gamma = 0.0);
    
      /*! 
       * \brief NOX::Abstract::Vector update function.  Applies vector 
       * update to each stored vector and scalar
       */
      virtual NOX::Abstract::Vector& 
      update(double alpha, const NOX::Abstract::Vector& a, 
	     double beta, const NOX::Abstract::Vector& b, 
	     double gamma = 0.0);

      /*! 
       * \brief NOX::Abstract::Vector norm function.  Computes norm of 
       * each stored vector and combines to compute appropriate norm
       */
      virtual double 
      norm(NormType type = TwoNorm) const;

      /*! 
       * \brief NOX::Abstract::Vector weighted norm function.  Computes 
       * weighted norm of each stored vector and combines to compute 
       * appropriate norm
       */
      virtual double 
      norm(const NOX::Abstract::Vector& weights) const;

      /*! 
       * \brief NOX::Abstract::Vector innerProduct function.
       *  Computes inner product * of each stored vector and combines to
       *  compute inner product
       */
      virtual double 
      innerProduct(const NOX::Abstract::Vector& y) const;
  
      /*! 
       * \brief NOX::Abstract::Vector length function.  Computes sum of 
       * lengths of stored vectors plus number of scalars
       */
      virtual int 
      length() const;
    
      /*! 
       * \brief NOX::Abstract::Vector print function.  For debugging purposes
       */
      virtual void 
      print(std::ostream& stream) const;

      //! Sets the ith vector
      virtual void 
      setVector(int i, const NOX::Abstract::Vector& v);

      //! Sets the ith vector as a view
      virtual void 
      setVectorView(int i, 
		    const Teuchos::RCP<NOX::Abstract::Vector>& v);

      //! Sets the ith scalar
      virtual void 
      setScalar(int i, double s);

      //! Sets the scalar array
      virtual void
      setScalarArray(double *sv);

      //! Returns const ref-count pointer to the ith vector
      virtual Teuchos::RCP<const NOX::Abstract::Vector>
      getVector(int i) const;

      //! Returns ref-count pointer to the ith vector
      virtual Teuchos::RCP<NOX::Abstract::Vector>
      getVector(int i);

      //! Returns copy of the ith scalar
      virtual double 
      getScalar(int i) const;

      //! Returns reference to the ith scalar
      virtual double& 
      getScalar(int i);

      //! Returns array of scalars
      virtual 
      Teuchos::RCP<const NOX::Abstract::MultiVector::DenseMatrix>
      getScalars() const;

      //! Returns array of scalars
      virtual Teuchos::RCP<NOX::Abstract::MultiVector::DenseMatrix>
      getScalars();

      //! Returns number of scalars
      virtual int getNumScalars() const;

      //! Returns number of vectors
      virtual int getNumVectors() const;

    protected:

      //! %Vector constructor.
      Vector(const Teuchos::RCP<LOCA::GlobalData>& global_data,
	     int nvecs, int nscalars);

      //! generate an extended multi-vector
      /*!
       * Extended vectors derived from this class should implement this
       * method and return a multivector of the appropriate type so that
       * the createMultiVectors method returns a multivector of the 
       * appropriate type.
       */
      virtual Teuchos::RCP<LOCA::Extended::MultiVector>
      generateMultiVector(int nColumns, int nVectorRows, 
			  int nScalarRows) const;

    protected:

      //! Global data
      Teuchos::RCP<LOCA::GlobalData> globalData;

      //! Array of vector pointers
      vector< Teuchos::RCP<NOX::Abstract::Vector> > vectorPtrs;

      //! Array indicating if each vector is a view
      vector< bool > isView;

      //! Number of scalars
      int numScalars;

      //! Scalars
      Teuchos::RCP<NOX::Abstract::MultiVector::DenseMatrix> scalarsPtr;

    }; // class Vector
  } // namspace Extended
} // namespace LOCA

#endif
