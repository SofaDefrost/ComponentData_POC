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
#ifndef SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_INL
#define SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_INL

#include <SofaBoundaryCondition/ConstantForceField.h>
#include <sofa/helper/system/config.h>
#include <sofa/simulation/Simulation.h>
#include <sofa/core/visual/VisualParams.h>
#include <SofaBaseTopology/TopologySubsetData.inl>

#include <math.h>
#include <cassert>
#include <iostream>
#include <numeric>

#include <sofa/core/objectmodel/BaseObjectDescription.h>
using sofa::core::objectmodel::BaseObjectDescription ;

namespace sofa
{

namespace component
{

namespace forcefield
{


template<class DataTypes>
ConstantForceField<DataTypes>::ConstantForceField()
    : d_indices(initData(&d_indices, "indices", "indices where the forces are applied"))
    , d_indexFromEnd(initData(&d_indexFromEnd,bool(false),"indexFromEnd", "Concerned DOFs indices are numbered from the end of the MState DOFs vector. (default=false)"))
    , d_forces(initData(&d_forces, "forces", "applied forces at each point"))
    , d_force(initData(&d_force, "force", "applied force to all points if forces attribute is not specified"))
    , d_totalForce(initData(&d_totalForce, "totalForce", "total force for all points, will be distributed uniformly over points"))
    , d_showArrowSize(initData(&d_showArrowSize,SReal(0.0), "showArrowSize", "Size of the drawn arrows (0->no arrows, sign->direction of drawing. (default=0)"))
    , d_color(initData(&d_color, defaulttype::RGBAColor(0.2f,0.9f,0.3f,1.0f), "showColor", "Color for object display (default: [0.2,0.9,0.3,1.0])"))
    , l_topology(initLink("topology", "link to the topology container"))
{
    d_showArrowSize.setGroup("Visualization");
    d_color.setGroup("Visualization");
}


template<class DataTypes>
void ConstantForceField<DataTypes>::parse(BaseObjectDescription* arg)
{
    /// Now warning the user when using old API
    /// TODO: remove for 20.06
    const char* val1=arg->getAttribute("points",nullptr) ;
    if(val1)
    {
        msg_error() << "The attribute 'points' is no longer valid."
                    << "It has been converted into 'indices' since Sofa 17.06 '" ;

    }
    /// Now warning the user when using old API
    /// TODO: remove for 20.06
    const char* val2=arg->getAttribute("arrowSizeCoef",nullptr) ;
    if(val2)
    {
        msg_error() << "The attribute 'arrowSizeCoef' is no longer valid."
                    << "It has been converted into 'showArrowSize' since Sofa 19.12 '" ;

    }
    Inherit::parse(arg) ;
}


template<class DataTypes>
void ConstantForceField<DataTypes>::init()
{
    this->m_componentstate = core::objectmodel::ComponentState::Invalid;
       
    if (l_topology.empty())
    {
        msg_info() << "link to Topology container should be set to ensure right behavior. First Topology found in current context will be used.";
        l_topology.set(this->getContext()->getMeshTopology());
    }

    // temprory pointer to topology
    sofa::core::topology::BaseMeshTopology* _topology = l_topology.get();    

    if (_topology)
    {
        msg_info() << "Topology path used: '" << l_topology.getLinkedPath() << "'";
        
        // Initialize functions and parameters for topology data and handler
        d_indices.createTopologicalEngine(_topology);
        d_indices.registerTopologicalData();

        m_systemSize = _topology->getNbPoints();
    }
    else
    {
        msg_info() << "No topology component found at path: " << l_topology.getLinkedPath() << ", nor in current context: " << this->getContext()->name;
        core::behavior::BaseMechanicalState* state = this->getContext()->getMechanicalState();
        m_systemSize = state->getSize();
    }


    const VecIndex & indices = d_indices.getValue();
    size_t indicesSize = indices.size();

    if (d_indices.isSet() && indicesSize!=0)
    {
        // check size of vector indices
        if( indicesSize > m_systemSize )
        {
            msg_error() << "Size mismatch: indices > system size";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
        // check each indice of the vector
        for(size_t i=0; i<indicesSize; i++)
        {
            if( indices[i] > m_systemSize )
            {
                msg_error() << "Indices incorrect: indice["<< i <<"] = "<< indices[i] <<" exceeds system size";
                this->m_componentstate = core::objectmodel::ComponentState::Invalid;
                return;
            }
        }
    }
    else
    {
        // initialize with all indices
        VecIndex& indicesEdit = *d_indices.beginEdit();
        indicesEdit.clear();
        indicesEdit.resize(m_systemSize);
        std::iota (std::begin(indicesEdit), std::end(indicesEdit), 0);
        d_indices.endEdit();
    }

    if (d_forces.isSet())
    {
        const VecDeriv &forces = d_forces.getValue();
        if( checkForces(forces) )
        {
            computeForceFromForceVector();
        }
        else
        {
            msg_error() << " Invalid given vector forces";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
        msg_info() << "Input vector forces is used for initialization";
    }
    else if (d_force.isSet())
    {
        const Deriv &force = d_force.getValue();
        if( checkForce(force) )
        {
            computeForceFromSingleForce();
        }
        else
        {
            msg_error() << " Invalid given force";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
        msg_info() << "Input force is used for initialization";
    }
    else if (d_totalForce.isSet())
    {
        const Deriv &totalForce = d_totalForce.getValue();
        if( checkForce(totalForce) )
        {
            computeForceFromTotalForce();
        }
        else
        {
            msg_error() << " Invalid given totalForce";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
        msg_info() << "Input totalForce is used for initialization";
    }

    // init from ForceField
    Inherit::init();

    // add to tracker
    this->trackInternalData(d_indices);
    this->trackInternalData(d_forces);
    this->trackInternalData(d_force);
    this->trackInternalData(d_totalForce);

    // if all init passes, component is valid
    this->m_componentstate = core::objectmodel::ComponentState::Valid;
}


template<class DataTypes>
void ConstantForceField<DataTypes>::reinit()
{
    // Now update is handled through the doInternalUpdate mechanism
    // called at each begin of step through the InternalUpdateDataVisitor
}


template<class DataTypes>
void ConstantForceField<DataTypes>::doUpdateInternal()
{
    if (this->hasDataChanged(d_indices))
    {
        msg_info() << "doUpdateInternal: data indices has changed";

        const VecIndex & indices = d_indices.getValue();
        size_t indicesSize = indices.size();

        this->m_componentstate = core::objectmodel::ComponentState::Valid;

        // check size of vector indices
        if( indicesSize > m_systemSize )
        {
            msg_error() << "Size mismatch: indices > system size";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
        else if( indicesSize==0 )
        {
            msg_warning() << "Size of vector indices is zero";
        }

        // check each indice of the vector
        for(size_t i=0; i<indicesSize; i++)
        {
            if( indices[i] > m_systemSize )
            {
                msg_error() << "Indices incorrect: indice["<< i <<"] = "<< indices[i] <<" exceeds system size";
                this->m_componentstate = core::objectmodel::ComponentState::Invalid;
                return;
            }
        }
    }

    if (this->hasDataChanged(d_forces))
    {
        msg_info() << "doUpdateInternal: data forces has changed";

        const VecDeriv &forces = d_forces.getValue();
        if( checkForces(forces) )
        {
            computeForceFromForceVector();
            this->m_componentstate = core::objectmodel::ComponentState::Valid;
        }
        else
        {
            msg_error() << " Invalid given vector forces";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
    }

    if (this->hasDataChanged(d_force))
    {
        msg_info() << "doUpdateInternal: data force has changed";

        const Deriv &force = d_force.getValue();
        if( checkForce(force) )
        {
            computeForceFromSingleForce();
            this->m_componentstate = core::objectmodel::ComponentState::Valid;
        }
        else
        {
            msg_error() << " Invalid given force";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
    }

    if (this->hasDataChanged(d_totalForce))
    {
        msg_info() << "doUpdateInternal: data totalForce has changed";

        const Deriv &totalForce = d_totalForce.getValue();
        if( checkForce(totalForce) )
        {
            computeForceFromTotalForce();
            this->m_componentstate = core::objectmodel::ComponentState::Valid;
        }
        else
        {
            msg_error() << " Invalid given totalForce";
            this->m_componentstate = core::objectmodel::ComponentState::Invalid;
            return;
        }
    }
}


template<class DataTypes>
bool ConstantForceField<DataTypes>::checkForce(const Deriv& force)
{
    size_t size = Deriv::spatial_dimensions;

    for (size_t i=0; i<size; i++)
    {
        if( std::isnan(force[i]) )
        {
            return false;
        }
    }
    return true;
}

template<class DataTypes>
bool ConstantForceField<DataTypes>::checkForces(const VecDeriv& forces)
{
    for (auto&& i : forces)
    {
        if(! checkForce(i) )
        {
            return false;
        }
    }
    return true;
}

template<class DataTypes>
void ConstantForceField<DataTypes>::computeForceFromForceVector()
{
    const VecDeriv& forces = d_forces.getValue();
    const size_t indicesSize = d_indices.getValue().size();
    Deriv& totalForce = *d_totalForce.beginEdit();
    totalForce.clear();
    if( indicesSize!=forces.size() )
    {
        msg_error() << "Impossible to use the vector forces since its size mismatches with indices size";
        this->m_componentstate = core::objectmodel::ComponentState::Invalid;
        return;
    }
    else
    {
        for(size_t i=0; i<indicesSize; i++)
        {
            totalForce += forces[i];
        }
    }

    d_totalForce.endEdit();
}

template<class DataTypes>
void ConstantForceField<DataTypes>::computeForceFromSingleForce()
{
    const Deriv& singleForce = d_force.getValue();
    const VecIndex & indices = d_indices.getValue();
    VecDeriv& forces = *d_forces.beginEdit();
    size_t indicesSize = indices.size();
    forces.clear();
    forces.resize(indicesSize);

    for(size_t i=0; i<indicesSize; i++)
    {
        forces[i] = singleForce;
    }

    d_totalForce.setValue(singleForce*(static_cast<Real>(indicesSize)));
    d_forces.endEdit();
}

template<class DataTypes>
void ConstantForceField<DataTypes>::computeForceFromTotalForce()
{
    const Deriv& totalForce = d_totalForce.getValue();
    const size_t indicesSize = d_indices.getValue().size();
    Deriv singleForce;
    if( indicesSize!=0 )
    {
        singleForce = totalForce / (static_cast<Real>(indicesSize));
        d_force.setValue(singleForce);
        computeForceFromSingleForce();
    }
    else
    {
        msg_error() << "Impossible to compute force from totalForce since vector indices size is zero";
        this->m_componentstate = core::objectmodel::ComponentState::Invalid;
        return;
    }
}


template<class DataTypes>
void ConstantForceField<DataTypes>::addForce(const core::MechanicalParams* params, DataVecDeriv& f1, const DataVecCoord& x1, const DataVecDeriv& v1)
{
    SOFA_UNUSED(params);
    SOFA_UNUSED(x1);
    SOFA_UNUSED(v1);

    sofa::helper::WriteAccessor< core::objectmodel::Data< VecDeriv > > _f1 = f1;
    const VecIndex& indices = d_indices.getValue();
    const VecDeriv& forces = d_forces.getValue();

    size_t indicesSize = indices.size();
    m_systemSize = _f1.size();

    if (!d_indexFromEnd.getValue())
    {
        for (size_t i=0; i<indicesSize; i++)
        {
            _f1[indices[i]] += forces[i];
        }
    }
    else
    {
        for (size_t i=0; i<indicesSize; i++)
        {
            _f1[m_systemSize - indices[i] - 1] += forces[i];
        }
    }

}

template <class DataTypes>
SReal ConstantForceField<DataTypes>::getPotentialEnergy(const core::MechanicalParams* params, const DataVecCoord& x) const
{
    SOFA_UNUSED(params);
    const VecIndex& indices = d_indices.getValue();
    const VecDeriv& f = d_forces.getValue();
    const VecCoord& _x = x.getValue();
    SReal e = 0;
    unsigned int i = 0;

    if (!d_indexFromEnd.getValue())
    {
        for (; i<indices.size(); i++)
        {
            e -= f[i] * _x[indices[i]];
        }
    }
    else
    {
        for (; i < indices.size(); i++)
        {
            e -= f[i] * _x[_x.size() - indices[i] -1];
        }
    }

    return e;
}


template <class DataTypes>
void ConstantForceField<DataTypes>::setForce(unsigned i, const Deriv& force)
{
    VecIndex& indices = *d_indices.beginEdit();
    VecDeriv& f = *d_forces.beginEdit();
    Deriv& totalf = *d_totalForce.beginEdit();
    indices.push_back(i);
    f.push_back( force );
    totalf += force;
    d_totalForce.endEdit();
    d_indices.endEdit();
    d_forces.endEdit();
}


template<class DataTypes>
void ConstantForceField<DataTypes>::addDForce(const core::MechanicalParams* mparams, DataVecDeriv& d_df , const DataVecDeriv& d_dx)
{
    // Derivative of a constant force is null, no need to compute addKToMatrix nor addDForce
    SOFA_UNUSED(mparams);
    SOFA_UNUSED(d_df);
    SOFA_UNUSED(d_dx);
    mparams->setKFactorUsed(true);
}


template<class DataTypes>
void ConstantForceField<DataTypes>::addKToMatrix(sofa::defaulttype::BaseMatrix * mat, SReal k, unsigned int & offset)
{
    // Derivative of a constant force is null, no need to compute addKToMatrix nor addDForce
    SOFA_UNUSED(mat);
    SOFA_UNUSED(k);
    SOFA_UNUSED(offset);
}


template<class DataTypes>
void ConstantForceField<DataTypes>::addKToMatrix(const sofa::core::behavior::MultiMatrixAccessor* matrix, SReal kFact)
{
    // Derivative of a constant force is null, no need to compute addKToMatrix nor addDForce
    SOFA_UNUSED(matrix);
    SOFA_UNUSED(kFact);
}


template<class DataTypes>
void ConstantForceField<DataTypes>::draw(const core::visual::VisualParams* vparams)
{
    const SReal aSC = d_showArrowSize.getValue();

    if ((!vparams->displayFlags().getShowForceFields() && (aSC==0.0)) || (aSC < 0.0)) return;

    vparams->drawTool()->saveLastState();

    const VecIndex& indices = d_indices.getValue();
    const VecDeriv& f = d_forces.getValue();
    const VecCoord& x = this->mstate->read(core::ConstVecCoordId::position())->getValue();

    if( fabs(aSC)<1.0e-10 )
    {
        std::vector<defaulttype::Vector3> points;
        for (unsigned int i=0; i<indices.size(); i++)
        {
            Real xx = 0.0, xy = 0.0, xz = 0.0, fx = 0.0, fy = 0.0, fz = 0.0;

            if (!d_indexFromEnd.getValue())
            {
                if (indices[i] < x.size())
                {
                    DataTypes::get(xx, xy, xz, x[indices[i]]);
                }
                else
                {
                    msg_error() << "Draw: error in indices values";
                }
            }
            else
            {
                if ((x.size() - indices[i] - 1) < x.size() && (x.size() - indices[i] - 1) >= 0)
                {
                    DataTypes::get(xx, xy, xz, x[x.size() - indices[i] - 1]);
                }
                else
                {
                    msg_error() << "Draw: error in indices values";
                }
            }

            DataTypes::get(fx,fy,fz, f[i] );
            points.push_back(defaulttype::Vector3(xx, xy, xz ));
            points.push_back(defaulttype::Vector3(xx+fx, xy+fy, xz+fz ));
        }
        vparams->drawTool()->drawLines(points, 2, defaulttype::Vec<4,float>(0,1,0,1));
    }
    else
    {

        vparams->drawTool()->setLightingEnabled(true);
        for (unsigned int i=0; i<indices.size(); i++)
        {
            Real xx = 0.0, xy = 0.0, xz = 0.0, fx = 0.0, fy = 0.0, fz = 0.0;

            if (!d_indexFromEnd.getValue())
            {
                if (indices[i] < x.size())
                {
                    DataTypes::get(xx, xy, xz, x[indices[i]]);
                }
                else
                {
                    msg_error() << "Draw: error in indices values";
                }
            }
            else
            {
                if ((x.size() - indices[i] - 1) < x.size() && (x.size() - indices[i] - 1) >= 0)
                {
                    DataTypes::get(xx, xy, xz, x[x.size() - indices[i] - 1]);
                }
                else
                {
                    msg_error() << "Draw: error in indices values";
                }
            }

            DataTypes::get(fx,fy,fz, f[i] );

            defaulttype::Vector3 p1( xx, xy, xz);
            defaulttype::Vector3 p2( aSC*fx+xx, aSC*fy+xy, aSC*fz+xz );

            float norm = static_cast<float>((p2-p1).norm());

            if( aSC > 0.0)
            {
                vparams->drawTool()->drawArrow(p1,p2, norm/20.0f, d_color.getValue());
            }
            else
            {
                vparams->drawTool()->drawArrow(p2,p1, norm/20.0f, d_color.getValue());
            }
        }
    }

    vparams->drawTool()->restoreLastState();
}

template<class DataTypes>
void ConstantForceField<DataTypes>::updateForceMask()
{
    const VecIndex& indices = d_indices.getValue();

    for (size_t i=0; i<indices.size(); i++)
    {
        this->mstate->forceMask.insertEntry(i);
    }
}

} // namespace forcefield

} // namespace component

} // namespace sofa

#endif // SOFA_COMPONENT_FORCEFIELD_CONSTANTFORCEFIELD_INL



