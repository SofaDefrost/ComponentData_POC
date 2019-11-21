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
#define SOFA_COMPONENT_MASS_UNIFORMMASS_CPP
#include <SofaBaseMechanics/UniformMass.inl>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/helper/system/FileRepository.h>
#include <sofa/helper/system/Locale.h>
using sofa::helper::system::TemporaryLocale ;

#include <sstream>

using std::string ;
using std::ostringstream ;

using sofa::component::mass::Vec3d ;
using sofa::helper::system::DataRepository ;

using namespace sofa::defaulttype;

namespace sofa
{

namespace component
{

namespace mass
{

static void skipToEOL(FILE* f)
{
    int	ch;
    while ((ch = fgetc(f)) != EOF && ch != '\n')
        ;
}


Mat3x3d MatrixFromEulerXYZ(double thetaX, double thetaY, double thetaZ)
{
    Quatd q=Quatd::fromEuler(thetaX, thetaY, thetaZ) ;
    Mat3x3d m;
    q.toMatrix(m);
    return m;
}


template <class RigidTypes, class MassType>
template <class T>
void UniformMass<RigidTypes, MassType>::loadFromFileRigidImpl(const string& filename)
{
    TemporaryLocale locale(LC_ALL, "C") ;

    if (!filename.empty())
    {
        MassType m = getVertexMass();
        string unconstingFilenameQuirck = filename ;
        if (!DataRepository.findFile(unconstingFilenameQuirck))
            msg_error(this) << "cannot find file '" << filename << "'.\n"  ;
        else
        {
            char	cmd[64];
            FILE*	file;
            if ((file = fopen(filename.c_str(), "r")) == nullptr)
            {
                msg_error(this) << "cannot open file '" << filename << "'.\n" ;
            }
            else
            {
                {
                    skipToEOL(file);
                    ostringstream cmdScanFormat;
                    cmdScanFormat << "%" << (sizeof(cmd) - 1) << "s";
                    while (fscanf(file, cmdScanFormat.str().c_str(), cmd) != EOF)
                    {
                        if (!strcmp(cmd,"inrt"))
                        {
                            for (int i = 0; i < 3; i++){
                                for (int j = 0; j < 3; j++){
                                    double tmp = 0;
                                    if( fscanf(file, "%lf", &(tmp)) < 1 ){
                                        msg_error(this) << "error while reading file '" << filename << "'.\n";
                                    }
                                    m.inertiaMatrix[i][j]=tmp;
                                }
                            }
                        }
                        else if (!strcmp(cmd,"cntr") || !strcmp(cmd,"center") )
                        {
                            Vec3d center;
                            for (int i = 0; i < 3; ++i)
                            {
                                if( fscanf(file, "%lf", &(center[i])) < 1 ){
                                    msg_error(this) << "error reading file '" << filename << "'.";
                                }
                            }
                        }
                        else if (!strcmp(cmd,"vertexMass"))
                        {
                            double mass;
                            if( fscanf(file, "%lf", &mass) > 0 )
                            {
                                m.mass = mass;
                                if (!this->d_vertexMass.isSet())
                                {
                                    this->d_vertexMass.forceSet();
                                    this->d_totalMass.unset();
                                }
                            }
                            else
                                msg_error(this) << "error reading file '" << filename <<  "'." << msgendl
                                                << "Unable to decode command 'mass'";
                        }
                        else if (!strcmp(cmd,"volm"))
                        {
                            double tmp;
                            if( fscanf(file, "%lf", &(tmp)) < 1 )
                                msg_error(this) << "error reading file '" << filename << "'." << msgendl
                                                << "Unable to decode command 'volm'." << msgendl;
                            m.volume = tmp ;
                        }
                        else if (!strcmp(cmd,"frme"))
                        {
                            Quatd orient;
                            for (int i = 0; i < 4; ++i)
                            {
                                if( fscanf(file, "%lf", &(orient[i])) < 1 )
                                    msg_error(this) << "error reading file '" << filename << "'." << msgendl
                                                    << "Unable to decode command 'frme' at index " << i ;
                            }
                            orient.normalize();
                        }
                        else if (!strcmp(cmd,"grav"))
                        {
                            Vec3d gravity;
                            if( fscanf(file, "%lf %lf %lf\n", &(gravity.x()), &(gravity.y()), &(gravity.z())) < 3 )
                                msg_warning(this) << "error reading file '" << filename << "'." << msgendl
                                                  << " Unable to decode command 'gravity'.";
                        }
                        else if (!strcmp(cmd,"visc"))
                        {
                            double viscosity = 0;
                            if( fscanf(file, "%lf", &viscosity) < 1 )
                                msg_warning(this) << "error reading file '" << filename << "'.\n"
                                                     " Unable to decode command 'visc'. \n";
                        }
                        else if (!strcmp(cmd,"stck"))
                        {
                            double tmp;
                            if( fscanf(file, "%lf", &tmp) < 1 ) //&(MSparams.default_stick));
                                msg_warning(this) << "error reading file '" << filename << "'.\n"
                                                  << "Unable to decode command 'stck'. \n";

                        }
                        else if (!strcmp(cmd,"step"))
                        {
                            double tmp;
                            if( fscanf(file, "%lf", &tmp) < 1 ) //&(MSparams.default_dt));
                                msg_warning(this) << "error reading file '" << filename << "'.\n"
                                                  << "Unable to decode command 'step'. \n";
                        }
                        else if (!strcmp(cmd,"prec"))
                        {
                            double tmp;
                            if( fscanf(file, "%lf", &tmp) < 1 ) //&(MSparams.default_prec));
                            {
                                msg_warning(this) << "error reading file '" << filename << "'.\n"
                                                  << "Unable to decode command 'prec'. \n" ;
                            }
                        }
                        else if (cmd[0] == '#')	// it's a comment
                        {
                            skipToEOL(file);
                        }
                        else		// it's an unknown keyword
                        {
                            msg_warning(this) << "error reading file '" << filename << "'. \n"
                                              << "Unable to decode an unknow command '"<< cmd << "'. \n" ;
                            skipToEOL(file);
                        }
                    }
                }
                fclose(file);
            }
        }
        setMass(m);
    }
    else if (d_totalMass.getValue()>0 && mstate!=nullptr) d_vertexMass.setValue((Real)d_totalMass.getValue() / mstate->getSize());
}


template <class RigidTypes, class MassType>
template <class T>
void UniformMass<RigidTypes, MassType>::drawRigid2DImpl(const VisualParams* vparams)
{
    if (!vparams->displayFlags().getShowBehaviorModels())
        return;

    const VecCoord& x =mstate->read(core::ConstVecCoordId::position())->getValue();
    ReadAccessor<Data<vector<int> > > indices = d_indices;
    defaulttype::Vec3d len;

    len[0] = len[1] = sqrt(d_vertexMass.getValue().inertiaMatrix);
    len[2] = 0;

    for (unsigned int i=0; i<indices.size(); i++)
    {
        Quat orient(Vec3d(0,0,1), x[indices[i]].getOrientation());
        Vec3d center; center = x[indices[i]].getCenter();

        vparams->drawTool()->drawFrame(center, orient, len*d_showAxisSize.getValue() );
    }
}

template <class RigidTypes, class MassType>
template <class T>
void UniformMass<RigidTypes, MassType>::drawRigid3DImpl(const VisualParams* vparams)
{
    if (!vparams->displayFlags().getShowBehaviorModels())
        return;

    const VecCoord& x =mstate->read(core::ConstVecCoordId::position())->getValue();
    ReadAccessor<Data<vector<int> > > indices = d_indices;
    typename RigidTypes::Vec3 gravityCenter;
    defaulttype::Vec3d len;

    // The moment of inertia of a box is:
    //   m->_I(0,0) = M/REAL(12.0) * (ly*ly + lz*lz);
    //   m->_I(1,1) = M/REAL(12.0) * (lx*lx + lz*lz);
    //   m->_I(2,2) = M/REAL(12.0) * (lx*lx + ly*ly);
    // So to get lx,ly,lz back we need to do
    //   lx = sqrt(12/M * (m->_I(1,1)+m->_I(2,2)-m->_I(0,0)))
    // Note that RigidMass inertiaMatrix is already divided by M
    double m00 = d_vertexMass.getValue().inertiaMatrix[0][0];
    double m11 = d_vertexMass.getValue().inertiaMatrix[1][1];
    double m22 = d_vertexMass.getValue().inertiaMatrix[2][2];
    len[0] = sqrt(m11+m22-m00);
    len[1] = sqrt(m00+m22-m11);
    len[2] = sqrt(m00+m11-m22);

    for (unsigned int i=0; i<indices.size(); i++)
    {
        if (getContext()->isSleeping())
            vparams->drawTool()->drawFrame(x[indices[i]].getCenter(), x[indices[i]].getOrientation(), len*d_showAxisSize.getValue(), Vec4f(0.5,0.5,0.5,1) );
        else
            vparams->drawTool()->drawFrame(x[indices[i]].getCenter(), x[indices[i]].getOrientation(), len*d_showAxisSize.getValue() );
        gravityCenter += (x[indices[i]].getCenter());
    }

    if (d_showInitialCenterOfGravity.getValue())
    {
        const VecCoord& x0 = mstate->read(core::ConstVecCoordId::restPosition())->getValue();

        for (unsigned int i=0; i<indices.size(); i++)
            vparams->drawTool()->drawFrame(x0[indices[i]].getCenter(), x0[indices[i]].getOrientation(), len*d_showAxisSize.getValue());
    }

    if(d_showCenterOfGravity.getValue())
    {
        gravityCenter /= x.size();
        const sofa::defaulttype::Vec4f color(1.0,1.0,0.0,1.0);

        vparams->drawTool()->drawCross(gravityCenter, d_showAxisSize.getValue(), color);
    }
}

template <class Vec6Types, class MassType>
template <class T>
void UniformMass<Vec6Types, MassType>::drawVec6Impl(const core::visual::VisualParams* vparams)
{
    if (!vparams->displayFlags().getShowBehaviorModels())
        return;
    const VecCoord& x =mstate->read(core::ConstVecCoordId::position())->getValue();
    const VecCoord& x0 = mstate->read(core::ConstVecCoordId::restPosition())->getValue();
    ReadAccessor<Data<vector<int> > > indices = d_indices;

    Mat3x3d R; R.identity();

    std::vector<Vector3> vertices;
    std::vector<sofa::defaulttype::Vec4f> colors;

    const sofa::defaulttype::Vec4f red(1.0,0.0,0.0,1.0);
    const sofa::defaulttype::Vec4f green(0.0,1.0,0.0,1.0);
    const sofa::defaulttype::Vec4f blue(0.0,0.0,1.0,1.0);

    sofa::defaulttype::Vec4f colorSet[3];
    colorSet[0] = red;
    colorSet[1] = green;
    colorSet[2] = blue;

    for (unsigned int i=0; i<indices.size(); i++)
    {
        defaulttype::Vec3d len(1,1,1);
        int a = (i<indices.size()-1)?i : i-1;
        int b = a+1;
        defaulttype::Vec3d dp; dp = x0[b]-x0[a];
        defaulttype::Vec3d p; p = x[indices[i]];
        len[0] = dp.norm();
        len[1] = len[0];
        len[2] = len[0];
        R = R * MatrixFromEulerXYZ(x[indices[i]][3], x[indices[i]][4], x[indices[i]][5]);

        for(unsigned int j=0 ; j<3 ; j++)
        {
            vertices.push_back(p);
            vertices.push_back(p + R.col(j)*len[j]);
            colors.push_back(colorSet[j]);
        }
    }

    vparams->drawTool()->drawLines(vertices, 1, colors);
}


template <class RigidTypes, class MassType>
template <class T>
Vector6 UniformMass<RigidTypes,MassType>::getMomentumRigid3DImpl( const MechanicalParams*,
                                                                  const DataVecCoord& d_x,
                                                                  const DataVecDeriv& d_v ) const
{
    ReadAccessor<DataVecDeriv> v = d_v;
    ReadAccessor<DataVecCoord> x = d_x;
    ReadAccessor<Data<vector<int> > > indices = d_indices;

    Real m = d_vertexMass.getValue().mass;
    const typename MassType::Mat3x3& I = d_vertexMass.getValue().inertiaMassMatrix;

    defaulttype::Vec6d momentum;

    for ( unsigned int i=0 ; i<indices.size() ; i++ )
    {
        typename RigidTypes::Vec3 linearMomentum = m*v[indices[i]].getLinear();
        for( int j=0 ; j< 3 ; ++j ) momentum[j] += linearMomentum[j];

        typename RigidTypes::Vec3 angularMomentum = cross( x[indices[i]].getCenter(), linearMomentum ) + ( I * v[indices[i]].getAngular() );
        for( int j=0 ; j< 3 ; ++j ) momentum[3+j] += angularMomentum[j];
    }

    return momentum;
}

template <class Vec3Types, class MassType>
template <class T>
Vector6 UniformMass<Vec3Types, MassType>::getMomentumVec3DImpl ( const MechanicalParams*,
                                                                 const DataVecCoord& d_x,
                                                                 const DataVecDeriv& d_v ) const
{
    ReadAccessor<DataVecDeriv> v = d_v;
    ReadAccessor<DataVecCoord> x = d_x;
    ReadAccessor<Data<vector<int> > > indices = d_indices;

    const MassType& m = d_vertexMass.getValue();
    defaulttype::Vec6d momentum;

    for ( unsigned int i=0 ; i<indices.size() ; i++ )
    {
        Deriv linearMomentum = m*v[indices[i]];
        for( int j=0 ; j<3 ; ++j ) momentum[j] += linearMomentum[j];

        Deriv angularMomentum = cross( x[indices[i]], linearMomentum );
        for( int j=0 ; j<3 ; ++j ) momentum[3+j] += angularMomentum[j];
    }

    return momentum;
}

template <class VecTypes, class MassType>
template <class T>
SReal UniformMass<VecTypes, MassType>::getPotentialEnergyRigidImpl(const core::MechanicalParams* mparams,
                                                                    const DataVecCoord& p_x) const
{
    SOFA_UNUSED(mparams) ;
    SReal e = 0;
    ReadAccessor< DataVecCoord > x = p_x;
    ReadAccessor<Data<vector<int> > > indices = d_indices;

    typename Coord::Pos g ( getContext()->getGravity() );
    for (unsigned int i=0; i<indices.size(); i++)
        e -= g*d_vertexMass.getValue().mass*x[indices[i]].getCenter();

    return e;
}

template <class VecTypes, class MassType>
template <class T>
void UniformMass<VecTypes, MassType>::addMDxToVectorVecImpl(defaulttype::BaseVector *resVect,
                                                     const VecDeriv* dx,
                                                     SReal mFact,
                                                     unsigned int& offset)
{
    unsigned int derivDim = (unsigned)Deriv::size();
    double m = d_vertexMass.getValue();

    ReadAccessor<Data<vector<int> > > indices = d_indices;

    const SReal* g = getContext()->getGravity().ptr();

    for (unsigned int i=0; i<indices.size(); i++)
        for (unsigned int j=0; j<derivDim; j++)
        {
            if (dx != nullptr)
                resVect->add(offset + indices[i] * derivDim + j, mFact * m * g[j] * (*dx)[indices[i]][0]);
            else
                resVect->add(offset + indices[i] * derivDim + j, mFact * m * g[j]);
        }
}


template<> SOFA_BASE_MECHANICS_API
void UniformMass<Rigid3Types, Rigid3Mass>::constructor_message()
{
    d_filenameMass.setDisplayed(true) ;
    d_filenameMass.setReadOnly(true) ;
    d_filenameMass.setValue("unused") ;
}

template<> SOFA_BASE_MECHANICS_API
void UniformMass<Rigid3Types, Rigid3Mass>::init()
{
    initDefaultImpl() ;

    // Call recalc from RigidTypes:
    // computes inertiaMassMatrix, invInertiaMatrix and invInertiaMassMatrix
    d_vertexMass.beginEdit()->recalc();
    d_vertexMass.endEdit();
}


template<>
SOFA_BASE_MECHANICS_API
void UniformMass<Rigid3Types, Rigid3Mass>::loadRigidMass(const string& filename)
{
    loadFromFileRigidImpl<Rigid3Types>(filename) ;
}

template <> SOFA_BASE_MECHANICS_API
void UniformMass<Rigid3Types, Rigid3Mass>::draw(const VisualParams* vparams)
{
    drawRigid3DImpl<Rigid3Types>(vparams) ;
}

template <> SOFA_BASE_MECHANICS_API
void UniformMass<Rigid2Types, Rigid2Mass>::draw(const VisualParams* vparams)
{
    drawRigid2DImpl<Rigid3Types>(vparams) ;
}

template <> SOFA_BASE_MECHANICS_API
SReal UniformMass<Rigid3Types,Rigid3Mass>::getPotentialEnergy( const MechanicalParams* params,
                                                                 const DataVecCoord& d_x ) const
{
    return getPotentialEnergyRigidImpl<Rigid3Types>(params, d_x) ;
}

template <> SOFA_BASE_MECHANICS_API
SReal UniformMass<Rigid2Types,Rigid2Mass>::getPotentialEnergy( const MechanicalParams* params,
                                                                 const DataVecCoord& vx ) const
{
    return getPotentialEnergyRigidImpl<Rigid2Types>(params, vx) ;
}

template <> SOFA_BASE_MECHANICS_API
void UniformMass<Vec6Types, double>::draw(const core::visual::VisualParams* vparams)
{
    drawVec6Impl<Vec6Types>(vparams) ;
}

template <> SOFA_BASE_MECHANICS_API
void UniformMass<Vec3Types, double>::addMDxToVector(defaulttype::BaseVector *resVect,
                                                     const VecDeriv* dx,
                                                     SReal mFact,
                                                     unsigned int& offset)
{
    addMDxToVectorVecImpl<Vec3Types>(resVect, dx,mFact,offset) ;
}

template <> SOFA_BASE_MECHANICS_API
Vector6 UniformMass<Vec3Types, double>::getMomentum ( const MechanicalParams* params,
                                                       const DataVecCoord& d_x,
                                                       const DataVecDeriv& d_v ) const
{
    return getMomentumVec3DImpl<Vec3Types>(params, d_x, d_v) ;
}

template <> SOFA_BASE_MECHANICS_API
Vector6 UniformMass<Rigid3Types,Rigid3Mass>::getMomentum ( const MechanicalParams* params,
                                                             const DataVecCoord& d_x,
                                                             const DataVecDeriv& d_v ) const
{
    return getMomentumRigid3DImpl<Rigid3Types>(params, d_x, d_v);
}




//////////////////////////////////////////// REGISTERING TO FACTORY /////////////////////////////////////////
/// Registering the component
/// see: https://www.sofa-framework.org/community/doc/programming-with-sofa/components-api/the-objectfactory/
/// 1-SOFA_DECL_CLASS(componentName) : Set the class name of the component
/// 2-RegisterObject("description") + .add<> : Register the component
/// 3-.add<>(true) : Set default template
// Register in the Factory
int UniformMassClass = core::RegisterObject("Define the same mass for all the particles")

        .add< UniformMass<Vec3Types,double> >()
        .add< UniformMass<Vec2Types,double> >()
        .add< UniformMass<Vec1Types,double> >()
        .add< UniformMass<Vec6Types,double> >()
        .add< UniformMass<Rigid3Types,Rigid3Mass> >()
        .add< UniformMass<Rigid2Types,Rigid2Mass> >()

        ;
////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////// TEMPLATE INITIALIZATION /////////////////////////////////////////////////
/// Force template specialization for the most common sofa type.
/// This goes with the extern template declaration in the .h. Declaring extern template
/// avoid the code generation of the template for each compilation unit.
/// see: http://www.stroustrup.com/C++11FAQ.html#extern-templates

template class SOFA_BASE_MECHANICS_API UniformMass<Vec3Types,double>;
template class SOFA_BASE_MECHANICS_API UniformMass<Vec2Types,double>;
template class SOFA_BASE_MECHANICS_API UniformMass<Vec1Types,double>;
template class SOFA_BASE_MECHANICS_API UniformMass<Vec6Types,double>;
template class SOFA_BASE_MECHANICS_API UniformMass<Rigid3Types,Rigid3Mass>;
template class SOFA_BASE_MECHANICS_API UniformMass<Rigid2Types,Rigid2Mass>;


////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace mass

} // namespace component

} // namespace sofa
