//**************************************************************************
//
//                                 NOTICE
//
// This software is a result of the research described in the report
//
// " A comparison of algorithms for modal analysis in the absence 
//   of a sparse direct method", P. Arbenz, R. Lehoucq, and U. Hetmaniuk,
//  Sandia National Laboratories, Technical report SAND2003-1028J.
//
// It is based on the Epetra, AztecOO, and ML packages defined in the Trilinos
// framework ( http://software.sandia.gov/trilinos/ ).
//
// The distribution of this software follows also the rules defined in Trilinos.
// This notice shall be marked on any reproduction of this software, in whole or
// in part.
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Code Authors: U. Hetmaniuk (ulhetma@sandia.gov), R. Lehoucq (rblehou@sandia.gov)
//
//**************************************************************************

#ifndef BLOCK_PCG_SOLVER_H
#define BLOCK_PCG_SOLVER_H

#include "Epetra_ConfigDefs.h"

#include "AztecOO.h"

#include "Epetra_BLAS.h"
#include "Epetra_Comm.h"
#include "Epetra_LAPACK.h"
#include "Epetra_Map.h"
#include "Epetra_MultiVector.h"
#include "Epetra_Operator.h"
#include "Epetra_RowMatrix.h"

#include "FortranRoutines.h"


class BlockPCGSolver : public virtual Epetra_Operator {

  private:

    const Epetra_Comm &MyComm;
    const Epetra_BLAS callBLAS;
    const Epetra_LAPACK callLAPACK;
    const FortranRoutines callFortran;

    const Epetra_Operator *K;
    Epetra_Operator *Prec;

    mutable AztecOO *vectorPCG;

    double tolCG;
    int iterMax;

    int verbose;

    mutable double *workSpace;
    mutable int lWorkSpace;

    mutable int numSolve;
    mutable int maxIter;
    mutable int sumIter; 
    mutable int minIter;

    // Don't define these functions
    BlockPCGSolver(const BlockPCGSolver &ref);
    BlockPCGSolver& operator=(const BlockPCGSolver &ref);

  public:

    BlockPCGSolver(const Epetra_Comm& _Com, const Epetra_Operator *KK,
                   double _tol = 0.0, int _iMax = 0, int _verb = 0);

    BlockPCGSolver(const Epetra_Comm& _Com, const Epetra_Operator *KK,
                   Epetra_Operator *PP, 
                   double _tol = 0.0, int _iMax = 0, int _verb = 0);

    ~BlockPCGSolver();

    char * Label() const { return "Epetra_Operator for Block PCG solver"; };

    bool UseTranspose() const { return (false); };
    int SetUseTranspose(bool UseTranspose_in) { return 0; };

    bool HasNormInf() const { return (false); };
    double NormInf() const  { return (-1.0); };

    int Apply(const Epetra_MultiVector &X, Epetra_MultiVector &Y) const;
    int ApplyInverse(const Epetra_MultiVector &X, Epetra_MultiVector &Y) const;

    const Epetra_Comm& Comm() const { return MyComm; };

    const Epetra_Map& OperatorDomainMap() const { return K->OperatorDomainMap(); };
    const Epetra_Map& OperatorRangeMap() const { return K->OperatorRangeMap(); };

    int Solve(const Epetra_MultiVector &X, Epetra_MultiVector &Y) const;
    int Solve(const Epetra_MultiVector &X, Epetra_MultiVector &Y, int blkSize) const;

    const Epetra_Operator* getPreconditioner() const { return Prec; };
    void setPreconditioner(Epetra_Operator *PP);

    void setIterMax(int _iMax) { iterMax = (_iMax > 0) ? _iMax : 0; };

    int getMaxIter() const { return maxIter; };
    double getAvgIter() const { return sumIter/((double) numSolve); };
    int getMinIter() const { return minIter; };

};

#endif
