// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Stokhos Package
//                 Copyright (2009) Sandia Corporation
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
// Questions? Contact Eric T. Phipps (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef STOKHOS_SGINVERSEMODELEVALUATOR_HPP
#define STOKHOS_SGINVERSEMODELEVALUATOR_HPP

#include "EpetraExt_ModelEvaluator.h"
#include "Teuchos_RCP.hpp"
#include "Teuchos_Array.hpp"
#include "Stokhos_OrthogPolyBasis.hpp"

namespace Stokhos {

  //! Nonlinear, inverse stochastic Galerkin ModelEvaluator
  /*!
   * SGInverseModelEvaluator is an implementation of EpetraExt::ModelEvaluator
   * that does the inverse of SGModelEvalutor, namely it takes SG versions of
   * the p InArgs and g and dg/dp OutArgs, and converts them to block vectors
   * that are passed to the underlying model evaluator.  This allows block
   * nonlinear problems to appear to SG problems.
   */
  class SGInverseModelEvaluator : public EpetraExt::ModelEvaluator {
  public:

    // Constructor
    SGInverseModelEvaluator(
      const Teuchos::RCP<EpetraExt::ModelEvaluator>& me,
      const Teuchos::RCP<const Stokhos::OrthogPolyBasis<int,double> >& sg_basis,
      const Teuchos::Array<int>& sg_p_index,
      const Teuchos::Array<int>& non_sg_p_index,
      const Teuchos::Array<int>& sg_g_index,
      const Teuchos::Array<int>& non_sg_g_index,
      const Teuchos::Array< Teuchos::RCP<const Epetra_Map> >& base_p_maps,
      const Teuchos::Array< Teuchos::RCP<const Epetra_Map> >& base_g_maps);

    /** \name Overridden from EpetraExt::ModelEvaluator . */
    //@{

    //! Return solution vector map
    Teuchos::RCP<const Epetra_Map> get_x_map() const;

    //! Return residual vector map
    Teuchos::RCP<const Epetra_Map> get_f_map() const;

    //! Return parameter vector map
    Teuchos::RCP<const Epetra_Map> get_p_map(int l) const;

    //! Return parameter vector map
    Teuchos::RCP<const Epetra_Map> get_p_sg_map(int l) const;

    //! Return response map
    Teuchos::RCP<const Epetra_Map> get_g_map(int l) const;

     //! Return observation vector map
    Teuchos::RCP<const Epetra_Map> get_g_sg_map(int l) const;

    //! Return array of parameter names
    Teuchos::RCP<const Teuchos::Array<std::string> > 
    get_p_names(int l) const;

    //! Return array of parameter names
    Teuchos::RCP<const Teuchos::Array<std::string> > 
    get_p_sg_names(int l) const;

    //! Return initial parameters
    Teuchos::RCP<const Epetra_Vector> get_p_init(int l) const;

    //! Return initial parameters
    Teuchos::RCP<const Stokhos::EpetraVectorOrthogPoly> get_p_sg_init(int l) const;

    //! Create InArgs
    InArgs createInArgs() const;

    //! Create OutArgs
    OutArgs createOutArgs() const;

    //! Evaluate model on InArgs
    void evalModel(const InArgs& inArgs, const OutArgs& outArgs) const;

    //@}

  protected:

    //! Underlying model evaluator
    Teuchos::RCP<EpetraExt::ModelEvaluator> me;

    //! Stochastic Galerkin basis
    Teuchos::RCP<const Stokhos::OrthogPolyBasis<int, double> > sg_basis;

    //! Index of stochastic parameters
    Teuchos::Array<int> sg_p_index;

    //! Index of non-stochastic parameters
    Teuchos::Array<int> non_sg_p_index;

    //! Index of stochastic responses
    Teuchos::Array<int> sg_g_index;

    //! Index of non-stochastic responses
    Teuchos::Array<int> non_sg_g_index;

    //! Base maps of block p vectors
    Teuchos::Array< Teuchos::RCP<const Epetra_Map> > base_p_maps;

    //! Base maps of block g vectors
    Teuchos::Array< Teuchos::RCP<const Epetra_Map> > base_g_maps;

    //! Number of stochastic blocks
    int num_sg_blocks;

    //! Number of stochastic parameter vectors
    int num_p_sg;

    //! Number of non-stochastic parameter vectors
    int num_p;

    //! Number of stochastic response vectors
    int num_g_sg;

    //! Number of non-stochastic response vectors
    int num_g;

    //! Block SG p vectors
    Teuchos::Array< Teuchos::RCP< Epetra_Vector > > block_p;

    //! Block SG g vectors
    mutable Teuchos::Array< Teuchos::RCP< Epetra_Vector > > block_g;

    //! Block SG dg/dp vectors
    mutable Teuchos::Array< Teuchos::Array< Teuchos::RCP< Epetra_MultiVector > > > block_dgdp;

  };

}

#endif // STOKHOS_SGMODELEVALUATOR_HPP
