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
#pragma once

#include "../../config.h"

#include <sofa/core/behavior/MechanicalState.h>
#include <sofa/core/topology/BaseMeshTopology.h>

#include <sofa/defaulttype/BaseVector.h>
#include <sofa/defaulttype/MapMapSparseMatrix.h>
#include <sofa/defaulttype/Quat.h>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/RigidTypes.h>

#include <vector>
#include <fstream>

namespace nodephysics
{

using namespace sofa;
using namespace sofa::core;
using namespace sofa::defaulttype;
using namespace sofa::core::objectmodel;

/// This class can be overridden if needed for additionnal storage within template specializations.
template <class DataTypes>
class MechanicalObject;

template<class DataTypes>
class MechanicalObjectInternalData
{
public:
    MechanicalObjectInternalData(MechanicalObject<DataTypes>* = nullptr) {}
};

/**
 * @brief MechanicalObject class
 */
template <class DataTypes>
class MechanicalObject : public sofa::core::behavior::MechanicalState<DataTypes>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE(MechanicalObject, DataTypes),
               SOFA_TEMPLATE(sofa::core::behavior::MechanicalState, DataTypes));

    typedef sofa::core::behavior::MechanicalState<DataTypes>      Inherited;
    typedef typename Inherited::VMultiOp    VMultiOp;
    typedef typename Inherited::ForceMask   ForceMask;
    typedef typename DataTypes::Real        Real;
    typedef typename DataTypes::Coord       Coord;
    typedef typename DataTypes::Deriv       Deriv;
    typedef typename DataTypes::VecCoord    VecCoord;
    typedef typename DataTypes::VecDeriv    VecDeriv;
    typedef typename DataTypes::MatrixDeriv						MatrixDeriv;
    typedef typename DataTypes::MatrixDeriv::RowConstIterator	MatrixDerivRowConstIterator;
    typedef typename DataTypes::MatrixDeriv::ColConstIterator	MatrixDerivColConstIterator;
    typedef typename DataTypes::MatrixDeriv::RowIterator		MatrixDerivRowIterator;
    typedef typename DataTypes::MatrixDeriv::ColIterator		MatrixDerivColIterator;

    typedef typename sofa::core::behavior::BaseMechanicalState::ConstraintBlock ConstraintBlock;

    typedef sofa::defaulttype::Vector3 Vector3;

protected:
    MechanicalObject();
public:
    MechanicalObject& operator = ( const MechanicalObject& );
protected:
    virtual ~MechanicalObject();
public:
    void parse ( sofa::core::objectmodel::BaseObjectDescription* arg ) override;

    Data< VecCoord > x; ///< position coordinates of the degrees of freedom
    Data< VecDeriv > v; ///< velocity coordinates of the degrees of freedom
    Data< VecDeriv > f; ///< force vector of the degrees of freedom
    Data< VecCoord > x0; ///< rest position coordinates of the degrees of freedom

    Data< VecDeriv > externalForces; ///< externalForces vector of the degrees of freedom
    Data< VecDeriv > dx; ///< dx vector of the degrees of freedom
    Data< VecCoord > xfree; ///< free position coordinates of the degrees of freedom
    Data< VecDeriv > vfree; ///< free velocity coordinates of the degrees of freedom
    Data< MatrixDeriv > c; ///< constraints applied to the degrees of freedom
    Data< MatrixDeriv > m; ///< mappingJacobian applied to the degrees of freedom
    Data< VecCoord > reset_position; ///< reset position coordinates of the degrees of freedom
    Data< VecDeriv > reset_velocity; ///< reset velocity coordinates of the degrees of freedom


    defaulttype::MapMapSparseMatrix< Deriv > c2;

    Data< SReal > restScale; ///< optional scaling of rest position coordinates (to simulated pre-existing internal tension).(default = 1.0)

    Data< bool >  d_useTopology; ///< Shall this object rely on any active topology to initialize its size and positions

    Data< bool >  showObject; ///< Show objects. (default=false)
    Data< float > showObjectScale; ///< Scale for object display. (default=0.1)
    Data< bool >  showIndices; ///< Show indices. (default=false)
    Data< float > showIndicesScale; ///< Scale for indices display. (default=0.02)
    Data< bool >  showVectors; ///< Show velocity. (default=false)
    Data< float > showVectorsScale; ///< Scale for vectors display. (default=0.0001)
    Data< int > drawMode; ///< The way vectors will be drawn: - 0: Line - 1:Cylinder - 2: Arrow.  The DOFS will be drawn: - 0: point - >1: sphere. (default=0)
    Data< defaulttype::Vec4f > d_color;  ///< drawing color

    void init() override;
    void reinit() override;

    void storeResetState() override;

    void reset() override;

    void writeVec(core::ConstVecId v, std::ostream &out) override;
    void readVec(core::VecId v, std::istream &in) override;
    SReal compareVec(core::ConstVecId v, std::istream &in) override;

    void writeState( std::ostream& out ) override;

    /// @name New vectors access API based on VecId
    /// @{

    Data< VecCoord >* write(core::VecCoordId v) override;
    const Data< VecCoord >* read(core::ConstVecCoordId v) const override;

    Data< VecDeriv >* write(core::VecDerivId v) override;
    const Data< VecDeriv >* read(core::ConstVecDerivId v) const override;

    Data< MatrixDeriv >* write(core::MatrixDerivId v) override;
    const Data< MatrixDeriv >* read(core::ConstMatrixDerivId v) const override;

    /// @}

    void resize( size_t vsize) override;
    virtual void reserve(size_t vsize);

    size_t getSize() const override { return d_size.getValue(); }

    SReal getPX(size_t i) const override { Real x=0.0,y=0.0,z=0.0; DataTypes::get(x,y,z,(read(core::ConstVecCoordId::position())->getValue())[i]); return (SReal)x; }
    SReal getPY(size_t i) const override { Real x=0.0,y=0.0,z=0.0; DataTypes::get(x,y,z,(read(core::ConstVecCoordId::position())->getValue())[i]); return (SReal)y; }
    SReal getPZ(size_t i) const override { Real x=0.0,y=0.0,z=0.0; DataTypes::get(x,y,z,(read(core::ConstVecCoordId::position())->getValue())[i]); return (SReal)z; }

    SReal getVX(size_t i) const { Real x=0.0,y=0.0,z=0.0; DataTypes::get(x,y,z, read(core::ConstVecDerivId::velocity())->getValue()[i]); return (SReal)x; }
    SReal getVY(size_t i) const { Real x=0.0,y=0.0,z=0.0; DataTypes::get(x,y,z, read(core::ConstVecDerivId::velocity())->getValue()[i]); return (SReal)y; }
    SReal getVZ(size_t i) const { Real x=0.0,y=0.0,z=0.0; DataTypes::get(x,y,z, read(core::ConstVecDerivId::velocity())->getValue()[i]); return (SReal)z; }


    std::string getClassName() const override
    {
        return "YOLO.MechanicalObject";
    }

    static std::string className(MechanicalObject* p)
    {
        return "YOLO.MechanicalObject";
    }





    /** \brief Overwrite values at index outputIndex by the ones at inputIndex.
     *
     */
    void replaceValue (const int inputIndex, const int outputIndex);

    /** \brief Exchange values at indices idx1 and idx2.
     *
     */
    void swapValues (const int idx1, const int idx2);

    /** \brief Reorder values according to parameter.
     *
     * Result of this method is :
     * newValue[ i ] = oldValue[ index[i] ];
     */
    void renumberValues( const sofa::helper::vector<unsigned int> &index );

    /** \brief Replace the value at index by the sum of the ancestors values weithed by the coefs.
     *
     * Sum of the coefs should usually equal to 1.0
     */
    void computeWeightedValue( const unsigned int i, const sofa::helper::vector< unsigned int >& ancestors, const sofa::helper::vector< double >& coefs);

    /// Force the position of a point (and force its velocity to zero value)
    void forcePointPosition( const unsigned int i, const sofa::helper::vector< double >& m_x);

    /// @name Initial transformations application methods.
    /// @{

    /// Apply translation vector to the position.
    void applyTranslation (const SReal dx, const SReal dy, const SReal dz) override;

    /// Rotation using Euler Angles in degree.
    void applyRotation (const SReal rx, const SReal ry, const SReal rz) override;

    void applyRotation (const defaulttype::Quat q) override;

    void applyScale (const SReal sx, const SReal sy, const SReal sz) override;

    /// @}

    /// Get the indices of the particles located in the given bounding box
    void getIndicesInSpace(sofa::helper::vector<unsigned>& indices, Real xmin, Real xmax, Real ymin, Real ymax, Real zmin, Real zmax) const override;

    /// update the given bounding box, to include this
    bool addBBox(SReal* minBBox, SReal* maxBBox) override;
    /// Bounding Box computation method.
    void computeBBox(const core::ExecParams* params, bool onlyVisible=false) override;

    /// @name Base Matrices and Vectors Interface
    /// @{

    /// Copy data to a global BaseVector the state stored in a local vector
    /// @param offset the offset in the BaseVector where the scalar values will be used. It will be updated to the first scalar value after the ones used by this operation when this method returns
    void copyToBaseVector(defaulttype::BaseVector* dest, core::ConstVecId src, unsigned int &offset) override;

    /// Copy data to a local vector the state stored in a global BaseVector
    /// @param offset the offset in the BaseVector where the scalar values will be used. It will be updated to the first scalar value after the ones used by this operation when this method returns
    void copyFromBaseVector(core::VecId dest, const defaulttype::BaseVector* src, unsigned int &offset) override;

    /// Add data to a global BaseVector from the state stored in a local vector
    /// @param offset the offset in the BaseVector where the scalar values will be used. It will be updated to the first scalar value after the ones used by this operation when this method returns
    void addToBaseVector(defaulttype::BaseVector* dest, core::ConstVecId src, unsigned int &offset) override;

    /// src and dest must have the same size.
    /// Performs: dest[i][j] += src[offset + i][j] 0<= i < src_entries  0<= j < 3 (for 3D objects) 0 <= j < 2 (for 2D objects)
    /// @param offset the offset in the BaseVector where the scalar values will be used. It will be updated to the first scalar value after the ones used by this operation when this method returns
    void addFromBaseVectorSameSize(core::VecId dest, const defaulttype::BaseVector* src, unsigned int &offset) override;

    /// src size can be smaller or equal to dest size.
    /// Performs: dest[ offset + i ][j] += src[i][j]  0<= i < src_entries  0<= j < 3 (for 3D objects) 0 <= j < 2 (for 2D objects)
    /// @param offset the offset in the MechanicalObject local vector specified by VecId dest. It will be updated to the first scalar value after the ones used by this operation when this method returns.
    void addFromBaseVectorDifferentSize(core::VecId dest, const defaulttype::BaseVector* src, unsigned int &offset ) override;


    /// @}

    /// Express the matrix L in term of block of matrices, using the indices of the lines in the MatrixDeriv container
    virtual std::list<ConstraintBlock> constraintBlocks( const std::list<unsigned int> &indices) const override;
    SReal getConstraintJacobianTimesVecDeriv( unsigned int line, core::ConstVecId id) override;

    /// @name Initial transformations accessors.
    /// @{

    void setTranslation(SReal dx, SReal dy, SReal dz) {translation.setValue(Vector3(dx,dy,dz));}
    void setRotation(SReal rx, SReal ry, SReal rz) {rotation.setValue(Vector3(rx,ry,rz));}
    void setScale(SReal sx, SReal sy, SReal sz) {scale.setValue(Vector3(sx,sy,sz));}

    virtual Vector3 getTranslation() const {return translation.getValue();}
    virtual Vector3 getRotation() const {return rotation.getValue();}
    Vector3 getScale() const override {return scale.getValue();}

    /// @}

    /// @name Integration related methods
    /// @{

    void beginIntegration(SReal dt) override;

    void endIntegration(const core::ExecParams* params, SReal dt) override;

    void accumulateForce(const core::ExecParams* params, core::VecDerivId f = core::VecDerivId::force()) override; // see BaseMechanicalState::accumulateForce(const ExecParams*, VecId) override

    /// Increment the index of the given VecCoordId, so that all 'allocated' vectors in this state have a lower index
    void vAvail(const core::ExecParams* params, core::VecCoordId& v) override;
    /// Increment the index of the given VecDerivId, so that all 'allocated' vectors in this state have a lower index
    void vAvail(const core::ExecParams* params, core::VecDerivId& v) override;
    /// Increment the index of the given MatrixDerivId, so that all 'allocated' vectors in this state have a lower index
    //virtual void vAvail(core::MatrixDerivId& v);

    /// Allocate a new temporary vector
    void vAlloc(const core::ExecParams* params, core::VecCoordId v) override;
    /// Allocate a new temporary vector
    void vAlloc(const core::ExecParams* params, core::VecDerivId v) override;
    /// Allocate a new temporary vector
    //virtual void vAlloc(core::MatrixDerivId v);

    /// Reallocate a new temporary vector
    void vRealloc(const core::ExecParams* params, core::VecCoordId v) override;
    /// Reallocate a new temporary vector
    void vRealloc(const core::ExecParams* params, core::VecDerivId v) override;


    /// Free a temporary vector
    void vFree(const core::ExecParams* params, core::VecCoordId v) override;
    /// Free a temporary vector
    void vFree(const core::ExecParams* params, core::VecDerivId v) override;
    /// Free a temporary vector
    //virtual void vFree(core::MatrixDerivId v);

    /// Initialize an unset vector
    void vInit(const core::ExecParams* params, core::VecCoordId v, core::ConstVecCoordId vSrc) override;
    /// Initialize an unset vector
    void vInit(const core::ExecParams* params, core::VecDerivId v, core::ConstVecDerivId vSrc) override;
    /// Initialize an unset vector
    //virtual void vInit(const core::ExecParams* params, core::MatrixDerivId v, core::ConstMatrixDerivId vSrc);

    void vOp(const core::ExecParams* params, core::VecId v, core::ConstVecId a = core::ConstVecId::null(), core::ConstVecId b = core::ConstVecId::null(), SReal f=1.0) override;

    void vMultiOp(const core::ExecParams* params, const VMultiOp& ops) override;

    void vThreshold(core::VecId a, SReal threshold ) override;

    SReal vDot(const core::ExecParams* params, core::ConstVecId a, core::ConstVecId b) override;

    /// Sum of the entries of state vector a at the power of l>0. This is used to compute the l-norm of the vector.
    SReal vSum(const core::ExecParams* params, core::ConstVecId a, unsigned l) override;

    /// Maximum of the absolute values of the entries of state vector a. This is used to compute the infinite-norm of the vector.
    SReal vMax(const core::ExecParams* params, core::ConstVecId a) override;

    size_t vSize( const core::ExecParams* params, core::ConstVecId v ) override;

    void resetForce(const core::ExecParams* params, core::VecDerivId f = core::VecDerivId::force()) override;

    void resetAcc(const core::ExecParams* params, core::VecDerivId a = core::VecDerivId::dx()) override;

    void resetConstraint(const core::ConstraintParams* cparams) override;

    void getConstraintJacobian(const core::ConstraintParams* cparams, sofa::defaulttype::BaseMatrix* J,unsigned int & off) override;

    void buildIdentityBlocksInJacobian(const sofa::helper::vector<unsigned int>& list_n, core::MatrixDerivId &mID) override;
    /// @}

    /// @name Debug
    /// @{

    void printDOF(core::ConstVecId, std::ostream& =std::cerr, int firstIndex=0, int range=-1 ) const override;
    unsigned printDOFWithElapsedTime(core::ConstVecId, unsigned =0, unsigned =0, std::ostream& =std::cerr ) override;

    void draw(const core::visual::VisualParams* vparams) override;

    /// @}

    // handle state changes
    void handleStateChange() override;

    /// Find mechanical particles hit by the given ray.
    /// A mechanical particle is defined as a 2D or 3D, position or rigid DOF
    /// Returns false if this object does not support picking
    bool pickParticles(const core::ExecParams* params, double rayOx, double rayOy, double rayOz, double rayDx, double rayDy, double rayDz, double radius0, double dRadius,
            std::multimap< double, std::pair<sofa::core::behavior::BaseMechanicalState*, int> >& particles) override;


   /// if this mechanical object stores independent dofs (in opposition to mapped dofs)
   bool isIndependent() const;


   const std::string templateName()
   {

   }
protected :

    /// @name Initial geometric transformations
    /// @{

    Data< Vector3 > translation; ///< Translation of the DOFs
    Data< Vector3 > rotation; ///< Rotation of the DOFs
    Data< Vector3 > scale; ///< Scale of the DOFs in 3 dimensions
    Data< Vector3 > translation2; ///< Translation of the DOFs, applied after the rest position has been computed
    Data< Vector3 > rotation2; ///< Rotation of the DOFs, applied the after the rest position has been computed

    /// @}

    //int vsize; ///< Number of elements to allocate in vectors
    Data< int > d_size; ///< Size of the vectors

    SingleLink< MechanicalObject<DataTypes>, core::topology::BaseMeshTopology,BaseLink::FLAG_STRONGLINK|BaseLink::FLAG_STOREPATH> l_topology;

    Data< int > f_reserve; ///< Size to reserve when creating vectors. (default=0)

    bool m_initialized;

    /// @name Integration-related data
    /// @{

    sofa::helper::vector< Data< VecCoord >		* > vectorsCoord;		///< Coordinates DOFs vectors table (static and dynamic allocated)
    sofa::helper::vector< Data< VecDeriv >		* > vectorsDeriv;		///< Derivates DOFs vectors table (static and dynamic allocated)
    sofa::helper::vector< Data< MatrixDeriv >	* > vectorsMatrixDeriv; ///< Constraint vectors table

    /**
     * @brief Inserts VecCoord DOF coordinates vector at index in the vectorsCoord container.
     */
    void setVecCoord(unsigned int /*index*/, Data< VecCoord >* /*vCoord*/);

    /**
     * @brief Inserts VecDeriv DOF derivates vector at index in the vectorsDeriv container.
     */
    void setVecDeriv(unsigned int /*index*/, Data< VecDeriv >* /*vDeriv*/);

    /**
     * @brief Inserts MatrixDeriv DOF  at index in the MatrixDeriv container.
     */
    void setVecMatrixDeriv(unsigned int /*index*/, Data< MatrixDeriv> * /*mDeriv*/);


    /// @}

    /**
    * @brief Internal function : Draw indices in 3d coordinates.
    */
    void drawIndices(const core::visual::VisualParams* vparams);

    /**
    * @brief Internal function : Draw vectors
    */
    void drawVectors(const core::visual::VisualParams* vparams);

    /// Given the number of a constraint Equation, find the index in the MatrixDeriv C, where the constraint is actually stored

    MechanicalObjectInternalData<DataTypes> data;

    friend class MechanicalObjectInternalData<DataTypes>;
};

template<> SOFA_BASE_MECHANICS_API
void MechanicalObject<defaulttype::Rigid3Types>::applyRotation (const defaulttype::Quat q);

template<> SOFA_BASE_MECHANICS_API
void MechanicalObject<defaulttype::Rigid3Types>::addFromBaseVectorSameSize(core::VecId dest, const defaulttype::BaseVector* src, unsigned int &offset );


template<> SOFA_BASE_MECHANICS_API
void MechanicalObject<defaulttype::Rigid3Types>::addFromBaseVectorDifferentSize(core::VecId dest, const defaulttype::BaseVector* src, unsigned int &offset );


template<> SOFA_BASE_MECHANICS_API
void MechanicalObject<defaulttype::Rigid3Types>::draw(const core::visual::VisualParams* vparams);




#if  !defined(SOFA_NODEPHYSICS_CPP)
extern template class SOFA_BASE_MECHANICS_API MechanicalObject<defaulttype::Vec3Types>;
extern template class SOFA_BASE_MECHANICS_API MechanicalObject<defaulttype::Vec2Types>;
extern template class SOFA_BASE_MECHANICS_API MechanicalObject<defaulttype::Vec1Types>;
extern template class SOFA_BASE_MECHANICS_API MechanicalObject<defaulttype::Vec6Types>;
extern template class SOFA_BASE_MECHANICS_API MechanicalObject<defaulttype::Rigid3Types>;
extern template class SOFA_BASE_MECHANICS_API MechanicalObject<defaulttype::Rigid2Types>;

#endif


} // namespace sofa
