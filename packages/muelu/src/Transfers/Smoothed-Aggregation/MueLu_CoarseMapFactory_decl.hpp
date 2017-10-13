// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
/*
 * MueLu_CoarseMapFactory_decl.hpp
 *
 *  Created on: Oct 12, 2012
 *      Author: wiesner
 */

#ifndef MUELU_COARSEMAPFACTORY_DECL_HPP_
#define MUELU_COARSEMAPFACTORY_DECL_HPP_

#include "Xpetra_StridedMapFactory_fwd.hpp"

#include "MueLu_ConfigDefs.hpp"
#include "MueLu_SingleLevelFactoryBase.hpp"
#include "MueLu_CoarseMapFactory_fwd.hpp"

#include "MueLu_Level_fwd.hpp"
#include "MueLu_Aggregates_fwd.hpp"
#include "MueLu_Exceptions.hpp"

namespace MueLu {

/*!
     @class CoarseMapFactory class.
     @brief Factory for generating coarse level map. Used by TentativePFactory.

     Factory is used by TentativePFactory to generate the coarse level map (=domain map) of
     the prolongation operator. The number of DOFs is calculated by the following formula
     \f[
     n_{Aggs}\cdot d_{blocksize}
     \f] where \f$ n_{Aggs} \f$ denotes the number of aggregates (generated by the AggregationFactory) and
     \f$ d_{blocksize}\f$ denotes the number of DOFs per node on the coarse level.
     For determing the blocksize the CoarseMapFactory supports strided map information.
     Per default stridedBlockId_ is -1 and the number of nullspace vectors (from "Nullspace") is used.
     Otherwise the block size is calculated using the striding information.

     For each level we can declare an own offset value for the global id's of the prolongator domain map
     by setting the "Domain GID offsets" parameter. If the number of entries in that list is smaller than
     the number of levels we use the default offset 0 for the remaining coarse levels.

     @ingroup MueLuTransferClasses

     ## Input/output of CoarseMapFactory ##

     ### User parameters of CoarseMapFactory ###
     Parameter | type | default | master.xml | validated | requested | description
     ----------|------|---------|:----------:|:---------:|:---------:|------------
     Aggregates | Factory | null | | * | * | Generating factory for aggregates. We basically need only the number of aggregates in this factory.
     Nullspace  | Factory | null |   | * | * | Generating factory of the fine nullspace vectors (of type "MultiVector"). In the default case the same instance of the TentativePFactory is also the generating factory for the null space vectors (on the next coarser levels). Therefore, it is recommended to declare the TentativePFactory to be the generating factory of the "Nullspace" variable globally using the FactoryManager object! For defining the near null space vectors on the finest level one should use the NullspaceFactory.
     Striding info | std::string | {} |  | * | | String containing the striding information, e.g. {2,1} for a fixed block size of 3 with sub blocks of size 2 and 1 in each node.
     Strided block id | LocalOrdinal | -1 |  | * | | Strided block id in strided information vector or -1 (=default) for the full block information.
     Domain GID offsets | std::string | {0} |  | * | | String describing a std::vector with offsets for GIDs for each level. If no offset GID value is given for the level we use 0 as default.

     The * in the @c master.xml column denotes that the parameter is defined in the @c master.xml file.<br>
     The * in the @c validated column means that the parameter is declared in the list of valid input parameters (see CoarseMapFactory::GetValidParameters).<br>
     The * in the @c requested column states that the data is requested as input with all dependencies (see CoarseMapFactory::DeclareInput).

     ### Variables provided by CoarseMapFactory ###

     After CoarseMapFactory::Build the following data is available (if requested)

     Parameter | generated by | description
     ----------|--------------|------------
     | CoarseMap | CoarseMapFactory | Map containing the coarse map used as domain map in the tentative prolongation operator

  */

  template <class Scalar = double, class LocalOrdinal = int, class GlobalOrdinal = LocalOrdinal, class Node = KokkosClassic::DefaultNode::DefaultNodeType>
  class CoarseMapFactory : public SingleLevelFactoryBase {
#undef MUELU_COARSEMAPFACTORY_SHORT
#include "MueLu_UseShortNames.hpp"

  public:

    //! @name Constructors/Destructors.
    //@{

    //! Constructor
    CoarseMapFactory();

    //! Destructor
    virtual ~CoarseMapFactory();

    RCP<const ParameterList> GetValidParameterList() const;

    //@}

    //! @name Input
    //@{
    /*! @brief Specifies the data that this class needs, and the factories that generate that data.

        If the Build method of this class requires some data, but the generating factory is not specified in DeclareInput, then this class
        will fall back to the settings in FactoryManager.
    */

    void DeclareInput(Level &currentLevel) const;

    //@}

    //! @name Build methods.
    //@{

    //! Build an object with this factory.
    void Build(Level &currentLevel) const;

    //@}


    /*! @brief getFixedBlockSize
     * returns the full block size (number of DOFs per node) of the domain DOF map (= coarse map).
     * This is the sum of all entries in the striding vector.
     * e.g. for 2 velocity dofs and 1 pressure dof the return value is 3.
     */
    virtual size_t getFixedBlockSize() const {
      // sum up size of all strided blocks (= number of dofs per node)
      size_t blkSize = 0;
      std::vector<size_t>::const_iterator it;
      for(it = stridingInfo_.begin(); it != stridingInfo_.end(); ++it) {
        blkSize += *it;
      }
      return blkSize;
    }

    //! @name Get/Set functions

    /*! @brief getStridingData
     * returns vector with size of striding blocks in the domain DOF map (= coarse map).
     * e.g. for 2 velocity dofs and 1 pressure dof the vector is (2,1)
     */
    virtual std::vector<size_t> getStridingData() const { return stridingInfo_; }

    /*! @brief setStridingData
     * set striding vector for the domain DOF map (= coarse map),
     * e.g. (2,1) for 2 velocity dofs and 1 pressure dof
     */
    virtual void setStridingData(std::vector<size_t> stridingInfo);

    /*! @brief getStridedBlockId
     * returns strided block id for the domain DOF map of Ptent (= coarse map)
     * or -1 if full strided map is stored in the domain map of Ptent (= coarse map)
     */
    virtual LocalOrdinal getStridedBlockId() const {
      const ParameterList & pL = GetParameterList();
      return pL.get<LocalOrdinal>("Strided block id");
    }

    /*! @brief setStridedBlockId
     * set strided block id for the domain DOF map of Ptent (= coarse map)
     * or -1 if full strided map is stored in the domain map of Ptent (= coarse map)
     */
    virtual void setStridedBlockId(LocalOrdinal stridedBlockId) {
      SetParameter("Strided block id", ParameterEntry(stridedBlockId));
    }

    //@}

  protected:

    mutable std::vector<size_t> stridingInfo_;   // vector with size of strided blocks (dofs)
    //LocalOrdinal stridedBlockId_;        // member variable denoting which dofs are stored in map
                                         // stridedBlock == -1: the full map (with all strided block dofs)
                                         // stridedBlock >  -1: only dofs of strided block with index "stridedBlockId" are stored in this map


  }; //class CoarseMapFactory

} //namespace MueLu

#define MUELU_COARSEMAPFACTORY_SHORT
#endif /* MUELU_COARSEMAPFACTORY_DECL_HPP_ */
