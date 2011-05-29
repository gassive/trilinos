/*@HEADER
// ***********************************************************************
// 
//        AztecOO: An Object-Oriented Aztec Linear Solver Package 
//                 Copyright (2002) Sandia Corporation
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
//@HEADER
*/

/*====================================================================
 * ------------------------
 * | CVS File Information |
 * ------------------------
 *
 * $RCSfile$
 *
 * $Author$
 *
 * $Date$
 *
 * $Revision$
 *
 * $Name$
 *====================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "az_aztec.h"
#ifdef AZ_COL_REORDER
/*extern void mc64ad_(int *, int *, int *, int *, int *, double*,
 *                    int *, int *, int *, int *, int *, double*,
 *                    int *, int *);
 */
void AZ_mat_colperm(int n, int bindx[], double val[], int **invp,
                    int name, struct context *context)

/*******************************************************************************

  Use the mc64ad algorithm to permute the columns of a matrix.


  Unresolved issues:
  1.  Similar Aztec modules return invp and delete perm.
  2.  The goal of this module is to increase the number of
      diagonal nonzeros.  This reduces the total number of
      nonzeros in MSR format.  Some effort is required to
      make this consistent with Aztec format.  


  Author:          D. Day

  Return code:     void
  ============

  Parameter list:
  ===============

  bindx :          On input, the nonzero sparsity pattern of the matrix
                   for which we will determine a new ordering.
                   Note: bindx is changed in this routine
  invp:            On output, invp[i] gives the location of row i
*/
{
  int job,nnz,nzdiag,liw,ldw,i,p,row,ki,kf,k,nod;
  char str[80];
  int *mcontrol, *info, *rowptr;
  /*
  double work;
  */
  double *diag;
  if (n==0) return;
  nnz = bindx[n]-1;
  liw = 5*n;
  ldw = 2*n + nnz; /* If job=1, then ldw := n */
  sprintf(str,"invp %s",context->tag);
  *invp = (int *) AZ_manage_memory((n+1)*sizeof(int), AZ_ALLOC, name, str,&i);
  mcontrol = (int *) AZ_allocate(10*sizeof(int));
  info = (int *) AZ_allocate(10*sizeof(int));
  rowptr  = (int *) AZ_allocate(liw*sizeof(int));
  diag    = (double *) AZ_allocate(ldw*sizeof(double));
  if (diag == NULL){
    printf("AZ_col_perm: Error: memory insufficient. Try job=1\n");
    AZ_exit(1);
  }

  /* Echo input matrix
   * printf("AZ_mat_colperm:  bindx[%d] = %d\n", n, bindx[n]);
   * for (row=0;row<n;row++){
   *   printf("%d %d %22.14e \n", row+1, row+1, val[row]);
   *   ki = bindx[row];
   *   kf = bindx[row+1];
   *   for (k=ki;k<kf;k++)
   *    printf("%d %d %22.14e \n", row+1, bindx[k]+1, val[k]);
   * }
   */

  /* msr2csr: retract the diagonal and delete zeros */
  for (row=0;row<n;row++) diag[row] = val[row];
  for (row=0;row<=n;row++) rowptr[row] = bindx[row];
  p=0;
  ki = rowptr[0];
  for (row=0;row<n;row++){
    rowptr[row] += ( row - n - 1);
    kf = rowptr[row+1];
    val[p] = diag[row];
    diag[row] = 0.0;
    bindx[p] = row;
    ++p;
    for (k=ki;k<kf;k++){
      val[p] = val[k];
      bindx[p] = bindx[k];
      ++p;
    }
    ki = kf;
  }
  --rowptr[n];
  p=0;
  ki = rowptr[0];
  for (row=0;row<n;row++){
    rowptr[row] = p;
    kf = rowptr[row+1];
    for (k=ki;k<kf;k++){
      if( val[k] != 0.0 ){
        val[p] = val[k];
        bindx[p] = bindx[k];
        ++p;
      }
    }
    ki = kf;
  }
  rowptr[n] = p;
  nnz = p;
  /* 
   * Convert to standard sparse matrix format with Fortran indexing
   * bindx(1:n+1), bindx(n+2:nnz+n+2), val(1:nnz)
   * bindx[n+1:rowptr[n]+n] := bindx[0:rowptr[n]-1] and then 
   * bindx[0:n] := rowptr[0:n]
   * mcontrol[0:2] := -1 turns off output
   */
  for (k=p-1;k>=0;k--) bindx[k+n+1] = bindx[k]+1;
  for (k=0;k<=n;k++) bindx[k] = rowptr[k]+1;
  for (k=0;k<=n;k++) rowptr[k] = 0;
  job = 4; /* job = 1 may do less violence to symmetric form */
/* for (i=0; i<4; i++) mcontrol[i] = 6; */
  for (i=0; i<3; i++) mcontrol[i] = -1;
  for (i=3; i<10; i++) mcontrol[i] = 0;
  for (i=0; i<10; i++) info[i] = 0;
  MC64AD_F77(&job,&n,&nnz,bindx,&(bindx[n+1]),val,&nzdiag,*invp,&liw,rowptr,&ldw,diag,mcontrol,info);
  /* nzdiag  is the number of zero diagonals in the permuted matrix */
  /*
    +1 structurally singular matrix (iffi nzdiag < n)
    +2 the returned scaling factors are large and may cause
       overflow when used to scale the matrix (for JOB = 5 entry only.)
    -1 JOB < 1 or JOB > 5.  Value of JOB held in INFO(2).
    -2 N < 1.  Value of N held in INFO(2).
    -3 NE < 1. Value of NE held in INFO(2).
    -4 the defined length LIW violates the restriction on LIW.
       Value of LIW required given by INFO(2).
    -5 the defined length LDW violates the restriction on LDW.
       Value of LDW required given by INFO(2).
    -6 entries are found whose row indices are out of range. INFO(2)
       contains the index of a column in which such an entry is found.
    -7 repeated entries are found. INFO(2) contains the index of a
       column in which such entries are found.
  */
  if( info[0] >= 0 ){

    /* convert permutation to C indexing and invert perm */
    for (i = 0;i<  n;i++) (*invp)[i]--; /* 1 2 3 0 */

    /* csr2msr: diag = diag(A P) */
    for (i = 0;i<= n;i++) bindx[i] += n;
    p = bindx[n]; 
    for (i = n+1;i<p;i++) bindx[i]--;
    for (i = n+1;i<p;i++) bindx[i] = (*invp)[bindx[i]];
    for (row=0;row<n;row++) diag[row] = 0.;
    p = n+1;
    for (row=0;row<n;row++){
      ki = bindx[row];
      bindx[row] = p;
      kf = bindx[row+1];
      for (k=ki;k<kf;k++){
        if( row != bindx[k]){
          bindx[p]   = bindx[k];
          val[p-n-1] = val[k-n-1];
          ++p;
        } else {
          diag[row] = val[k-n-1];
        }
      }
    }
    bindx[n] = p;
    /* val[n+1: (n+1) + nod-1] := val[0:nod-1], nod = number off-diagonals */
    nod = p-(n+1);
    /* printf("az_colperm: number of off diagonals is %d\n",nod); */
    for (i=nod ; i>0 ; i-- ) val[n+i] = val[i-1];
    val[n] = 0;
    for (i = 0 ; i < n ; i++ ) val[i] = diag[i];

    /* Sort the colmns to ascend */
    /* This appears unnecessary, though one never can be certain.
    for (row=0;row<n;row++){
      ki = bindx[row];
      kf = bindx[row+1];
      for (p=ki+1;k<kf;k++){
        k = p;
        while ( (k>ki) && (bindx[k-1]>bindx[k]) ){
          work = val[k];
          val[k] = val[k-1];
          val[k-1] = work;
          i = bindx[k];
          bindx[k] = bindx[k-1];
          bindx[k-1] = i;
          --k;
        }
      }
    }
    */
    if( info[0] == 1 ){
      printf("AZ_col_perm: Error: Internal matrix is singular\n");
    }
  }else{

    /* Ideally an error flag would be returned here */
    printf("az_colperm:  Error: info = %d %d\n",info[0],info[1]);
    AZ_exit(1);
  }
  AZ_free(mcontrol);
  AZ_free(info);
  AZ_free(diag);
  AZ_free(rowptr);
  return;
}
#endif 
