/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2019 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_H
#define SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_H
#include "config.h"

#include <sofa/core/behavior/ForceField.h>

#include <SofaBaseTopology/TopologySubsetData.h>
#include <sofa/defaulttype/RGBAColor.h>

namespace sofa
{

namespace component
{

namespace forcefield
{

/// Apply constant forces to given degrees of freedom.
template<class DataTypes>
class ConstantForceField : public core::behavior::ForceField<DataTypes>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(ConstantForceField, DataTypes), SOFA_TEMPLATE(core::behavior::ForceField, DataTypes));

    typedef core::behavior::ForceField<DataTypes> Inherit;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename DataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;
    typedef typename Coord::value_type Real;
    typedef helper::vector<unsigned int> VecIndex;
    typedef core::objectmodel::Data<VecCoord> DataVecCoord;
    typedef core::objectmodel::Data<VecDeriv> DataVecDeriv;

    typedef sofa::component::topology::PointSubsetData< VecIndex > SetIndex;

public:
    /// indices of the points the force applies to
    SetIndex d_indices;

    /// Concerned DOFs indices are numbered from the end of the MState DOFs vector
    Data< bool > d_indexFromEnd;

    /// Per-point forces.
    Data< VecDeriv > d_forces;

    /// Force applied at each point, if per-point forces are not specified
    Data< Deriv > d_force;

    /// Sum of the forces applied at each point, if per-point forces are not specified
    Data< Deriv > d_totalForce;

    /// S for drawing. The sign changes the direction, 0 doesn't draw arrow
    Data< SReal > d_showArrowSize;

    /// display color
    Data< defaulttype::RGBAColor > d_color;

    /// Concerned DOFs indices are numbered from the end of the MState DOFs vector
    Data< bool > indexFromEnd;

    /// Link to be set to the topology container in the component graph.
    SingleLink<ConstantForceField<DataTypes>, sofa::core::topology::BaseMeshTopology, BaseLink::FLAG_STOREPATH | BaseLink::FLAG_STRONGLINK> l_topology;

public:
    /// Init function
    void init() override;

    /// Re-init function
    void reinit() override;

    /// Add the forces
    void addForce (const core::MechanicalParams* params, DataVecDeriv& f, const DataVecCoord& x, const DataVecDeriv& v) override;

    /// Constant force has null variation
    void addDForce(const core::MechanicalParams* mparams, DataVecDeriv& d_df , const DataVecDeriv& d_dx) override;

    /// Constant force has null variation
    void addKToMatrix(sofa::defaulttype::BaseMatrix *mat, SReal k, unsigned int &offset) override;

    /// Constant force has null variation
    virtual void addKToMatrix(const sofa::core::behavior::MultiMatrixAccessor* /*matrix*/, SReal /*kFact*/) ;

    SReal getPotentialEnergy(const core::MechanicalParams* params, const DataVecCoord& x) const override;

    void draw(const core::visual::VisualParams* vparams) override;

    void updateForceMask() override;

    /// Update data and internal vectors
    void doUpdateInternal() override;

    /// Set a force to a given particle
    void setForce( unsigned i, const Deriv& f );

    /// Parse function (to be removed after v19.12)
    void parse(sofa::core::objectmodel::BaseObjectDescription* arg) override;

    using Inherit::addAlias ;
    using Inherit::addKToMatrix;


protected:
    ConstantForceField();

    /// Functions checking inputs before update
    bool checkForce(const Deriv&  force);
    bool checkForces(const VecDeriv& forces);

    /// Functions computing and updating the constant force vector
    void computeForceFromSingleForce();
    void computeForceFromForceVector();
    void computeForceFromTotalForce();

    /// Save system size for update of indices (doUpdateInternal)
    size_t m_systemSize;
};

template <>
SReal ConstantForceField<defaulttype::Rigid3Types>::getPotentialEnergy(const core::MechanicalParams*, const DataVecCoord& ) const;
template <>
SReal ConstantForceField<defaulttype::Rigid2Types>::getPotentialEnergy(const core::MechanicalParams*, const DataVecCoord& ) const;



#if  !defined(SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_CPP)
extern template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<sofa::defaulttype::Vec3Types>;
extern template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<sofa::defaulttype::Vec2Types>;
extern template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<sofa::defaulttype::Vec1Types>;
extern template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<sofa::defaulttype::Vec6Types>;
extern template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<sofa::defaulttype::Rigid3Types>;
extern template class SOFA_BOUNDARY_CONDITION_API ConstantForceField<sofa::defaulttype::Rigid2Types>;

#endif

} // namespace forcefield

} // namespace component

} // namespace sofa

#endif // SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_H
