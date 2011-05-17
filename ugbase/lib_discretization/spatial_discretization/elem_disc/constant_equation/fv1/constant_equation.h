/*
 * constant_equation_impl.h
 *
 *  Created on: 13.05.2011
 *      Author: andreasvogel
 */

#ifndef __H__LIB_DISCRETIZATION__SPATIAL_DISCRETIZATION__ELEM_DISC__CONSTANT_EQUATION__FV1__CONSTANT_EQUATION__
#define __H__LIB_DISCRETIZATION__SPATIAL_DISCRETIZATION__ELEM_DISC__CONSTANT_EQUATION__FV1__CONSTANT_EQUATION__

// other ug4 modules
#include "common/common.h"
#include "lib_grid/lg_base.h"

// library intern headers
#include "lib_discretization/spatial_discretization/disc_helper/geometry_provider.h"
#include "lib_discretization/spatial_discretization/elem_disc/elem_disc_interface.h"
#include "lib_discretization/common/local_algebra.h"
#include "lib_discretization/spatial_discretization/ip_data/ip_data.h"
#include "lib_discretization/spatial_discretization/ip_data/data_import.h"

namespace ug{

/// \ingroup lib_disc_elem_disc
/// @{

/// Finite Volume Element Discretization for a Constant Equation
/**
 * This class implements the IElemDisc interface to provide element local
 * assemblings for the equation:
 * The Equation has the form
 * \f[
 * 	\partial_t m - \nabla \left( \vec{v} right) = f
 * \f]
 * with
 * <ul>
 * <li>	<b>no</b> unknown solution
 * <li>	\f$ m \equiv m(\vec{x},t) \f$ is the Mass Scaling Term
 * <li>	\f$ v \equiv \vec{v}(\vec{x},t) \f$ is the Velocity Field
 * <li>	\f$ f \equiv f(\vec{x},t) \f$ is a Source Term
 * </ul>
 *
 * \tparam	TDomain		Domain
 * \tparam	TAlgebra	Algebra
 */
template<	typename TDomain,
			typename TAlgebra>
class FVConstantEquationElemDisc
: public IDomainElemDisc<TDomain, TAlgebra>
{
	private:
	///	Base class type
		typedef IDomainElemDisc<TDomain, TAlgebra> base_type;

	public:
	///	Domain type
		typedef typename base_type::domain_type domain_type;

	///	World dimension
		static const int dim = base_type::dim;

	///	Position type
		typedef typename base_type::position_type position_type;

	///	Algebra type
		typedef typename base_type::algebra_type algebra_type;

	///	Local matrix type
		typedef typename base_type::local_matrix_type local_matrix_type;

	///	Local vector type
		typedef typename base_type::local_vector_type local_vector_type;

	///	Local index type
		typedef typename base_type::local_index_type local_index_type;

	public:
	///	Constructor
		FVConstantEquationElemDisc()
		{
		//	register assemling functions
			register_ass_funcs(Int2Type<dim>());

		//	register exports
			register_export(m_exConcentration);
			register_export(m_exConcentrationGrad);

		//	register imports
			register_import(m_imVelocity);
			register_import(m_imSource);
			register_import(m_imMassScale);

			m_imMassScale.set_mass_part(true);
		}

	///	sets the velocity field
	/**
	 * This method sets the Velocity field. If no field is provided a zero
	 * value is assumed.
	 */
		void set_velocity(IPData<MathVector<dim>, dim>& user) {m_imVelocity.set_data(user);}

	///	sets the source / sink term
	/**
	 * This method sets the source/sink value. A zero value is assumed as
	 * default.
	 */
		void set_source(IPData<number, dim>& user)	{m_imSource.set_data(user);}

	///	sets mass scale
	/**
	 * This method sets the mass scale value. A value of 1.0 is assumed as
	 * default.
	 */
		void set_mass_scale(IPData<number, dim>& user)	{m_imMassScale.set_data(user);}

	public:
	///	number of functions used
		virtual size_t num_fct(){return 1;}

	///	type of trial space for each function used
		virtual LocalShapeFunctionSetID local_shape_function_set_id(size_t loc_fct)
		{
			return LocalShapeFunctionSetID(LocalShapeFunctionSetID::LAGRANGE, 1);
		}

	///	switches between non-regular and regular grids
		virtual bool treat_non_regular_grid(bool bNonRegular)
		{
		//	switch, which assemble functions to use.
			if(bNonRegular)
				register_non_regular_ass_funcs(Int2Type<dim>());
			else
				register_ass_funcs(Int2Type<dim>());

		//	this disc supports both grids
			return true;
		}

		virtual bool use_hanging() const {return true;}

	private:
	///	prepares the discretization for time dependent discretization
	/**
	 * This function prepares the discretization for time-dependent problems.
	 * It sets the time in the imports.
	 *
	 * \param[in]	time	new time point
	 * \returns 	true	indicates, that old values are needed
	 */
		virtual bool time_point_changed(number time);

	///	prepares the loop over all elements
	/**
	 * This method prepares the loop over all elements. It resizes the Position
	 * array for the corner coordinates and schedules the local ip positions
	 * at the data imports.
	 */
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		inline bool prepare_element_loop();

	///	prepares the element for assembling
	/**
	 * This methods prepares an element for the assembling. The Positions of
	 * the Element Corners are read and the Finite Volume Geometry is updated.
	 * The global ip positions are scheduled at the data imports.
	 */
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		inline bool prepare_element(TElem* elem,
		                            const local_vector_type& u,
		                            const local_index_type& glob_ind);

	///	finishes the loop over all elements
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		inline bool finish_element_loop();

	///	assembles the local stiffness matrix using a finite volume scheme
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool assemble_JA(local_matrix_type& J, const local_vector_type& u);

	///	assembles the local mass matrix using a finite volume scheme
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool assemble_JM(local_matrix_type& J, const local_vector_type& u);

	///	assembles the stiffness part of the local defect
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool assemble_A(local_vector_type& d, const local_vector_type& u);

	///	assembles the mass part of the local defect
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool assemble_M(local_vector_type& d, const local_vector_type& u);

	///	assembles the local right hand side
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool assemble_f(local_vector_type& d);

	protected:
	///	computes the linearized defect w.r.t to the velocity
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool lin_defect_velocity(const local_vector_type& u);

	///	computes the linearized defect w.r.t to the source term
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool lin_defect_source(const local_vector_type& u);

	///	computes the linearized defect w.r.t to the mass scale term
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool lin_defect_mass_scale(const local_vector_type& u);

	private:
	///	Corner Coordinates
		const position_type* m_vCornerCoords;

	///	abbreviation for the local solution
		static const size_t _C_ = 0;

	///	Data import for the Velocity field
		DataImport<MathVector<dim>, dim, algebra_type > m_imVelocity;

	///	Data import for the right-hand side
		DataImport<number, dim, algebra_type> m_imSource;

	///	Data import for the mass scale
		DataImport<number, dim, algebra_type> m_imMassScale;

	public:
	///	returns the export of the concentration
		IPData<number, dim>& get_concentration() {return m_exConcentration;}

	///	returns the export of gradient of the concentration
		IPData<MathVector<dim>, dim>& get_concentration_grad() {return m_exConcentrationGrad;}

	protected:
	///	computes the concentration
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool compute_concentration_export(const local_vector_type& u, bool compDeriv);

	///	computes the gradient of the concentration
		template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
		bool compute_concentration_grad_export(const local_vector_type& u, bool compDeriv);

	///	Export for the concentration
		DataExport<number, dim, algebra_type> m_exConcentration;

	///	Export for the gradient of concentration
		DataExport<MathVector<dim>, dim, algebra_type> m_exConcentrationGrad;

	private:
		////////////////////////////////////
		//	regular elements
		////////////////////////////////////

	/// register for 1D
		void register_ass_funcs(Int2Type<1>)
		{
			register_all_ass_funcs<Edge>(ROID_EDGE);
		}

	/// register for 2D
		void register_ass_funcs(Int2Type<2>)
		{
			register_ass_funcs(Int2Type<1>());
			register_all_ass_funcs<Triangle>(ROID_TRIANGLE);
			register_all_ass_funcs<Quadrilateral>(ROID_QUADRILATERAL);
		}

	/// register for 3D
		void register_ass_funcs(Int2Type<3>)
		{
			register_ass_funcs(Int2Type<2>());
			register_all_ass_funcs<Tetrahedron>(ROID_TETRAHEDRON);
			register_all_ass_funcs<Pyramid>(ROID_PYRAMID);
			register_all_ass_funcs<Prism>(ROID_PRISM);
			register_all_ass_funcs<Hexahedron>(ROID_HEXAHEDRON);
		}

	///	register all functions for on element type
		template <typename TElem>
		void register_all_ass_funcs(int id)
		{
			typedef FVConstantEquationElemDisc T;

			register_prepare_element_loop_function(	id, &T::template prepare_element_loop<TElem, FV1Geometry>);
			register_prepare_element_function(		id, &T::template prepare_element<TElem, FV1Geometry>);
			register_finish_element_loop_function(	id, &T::template finish_element_loop<TElem, FV1Geometry>);
			register_assemble_JA_function(			id, &T::template assemble_JA<TElem, FV1Geometry>);
			register_assemble_JM_function(			id, &T::template assemble_JM<TElem, FV1Geometry>);
			register_assemble_A_function(			id, &T::template assemble_A<TElem, FV1Geometry>);
			register_assemble_M_function(			id, &T::template assemble_M<TElem, FV1Geometry>);
			register_assemble_f_function(			id, &T::template assemble_f<TElem, FV1Geometry>);

		//	set computation of linearized defect w.r.t velocity
			m_imVelocity.register_lin_defect_func(id, this, &T::template lin_defect_velocity<TElem, FV1Geometry>);
			m_imSource.register_lin_defect_func(id, this, &T::template lin_defect_source<TElem, FV1Geometry>);
			m_imMassScale.register_lin_defect_func(id, this, &T::template lin_defect_mass_scale<TElem, HFV1Geometry>);

		//	exports
			m_exConcentration.register_export_func(id, this, &T::template compute_concentration_export<TElem, FV1Geometry>);
			m_exConcentrationGrad.register_export_func(id, this, &T::template compute_concentration_grad_export<TElem, FV1Geometry>);
		}

	////////////////////////////////////
	//	non-regular elements
	////////////////////////////////////

	/// register for 1D
		void register_non_regular_ass_funcs(Int2Type<1>)
		{
			register_all_non_regular_ass_funcs<Edge>(ROID_EDGE);
		}

	/// register for 2D
		void register_non_regular_ass_funcs(Int2Type<2>)
		{
			register_non_regular_ass_funcs(Int2Type<1>());
			register_all_non_regular_ass_funcs<Triangle>(ROID_TRIANGLE);
			register_all_non_regular_ass_funcs<Quadrilateral>(ROID_QUADRILATERAL);
		}

	/// register for 3D
		void register_non_regular_ass_funcs(Int2Type<3>)
		{
			register_non_regular_ass_funcs(Int2Type<2>());
			register_all_non_regular_ass_funcs<Tetrahedron>(ROID_TETRAHEDRON);
			register_all_non_regular_ass_funcs<Pyramid>(ROID_PYRAMID);
			register_all_non_regular_ass_funcs<Prism>(ROID_PRISM);
			register_all_non_regular_ass_funcs<Hexahedron>(ROID_HEXAHEDRON);
		}

	///	register all functions for on element type
		template <typename TElem>
		void register_all_non_regular_ass_funcs(int id)
		{
			typedef FVConstantEquationElemDisc T;

			register_prepare_element_loop_function(	id, &T::template prepare_element_loop<TElem, HFV1Geometry>);
			register_prepare_element_function(		id, &T::template prepare_element<TElem, HFV1Geometry>);
			register_finish_element_loop_function(	id, &T::template finish_element_loop<TElem, HFV1Geometry>);
			register_assemble_JA_function(			id, &T::template assemble_JA<TElem, HFV1Geometry>);
			register_assemble_JM_function(			id, &T::template assemble_JM<TElem, HFV1Geometry>);
			register_assemble_A_function(			id, &T::template assemble_A<TElem, HFV1Geometry>);
			register_assemble_M_function(			id, &T::template assemble_M<TElem, HFV1Geometry>);
			register_assemble_f_function(			id, &T::template assemble_f<TElem, HFV1Geometry>);

		//	set computation of linearized defect w.r.t velocity
			m_imVelocity.register_lin_defect_func(id, this, &T::template lin_defect_velocity<TElem, HFV1Geometry>);
			m_imSource.register_lin_defect_func(id, this, &T::template lin_defect_source<TElem, HFV1Geometry>);
			m_imMassScale.register_lin_defect_func(id, this, &T::template lin_defect_mass_scale<TElem, HFV1Geometry>);

		//	exports
			m_exConcentration.register_export_func(id, this, &T::template compute_concentration_export<TElem, HFV1Geometry>);
			m_exConcentrationGrad.register_export_func(id, this, &T::template compute_concentration_grad_export<TElem, HFV1Geometry>);
		}
};

/// @}

} // end namespace ug

#include "constant_equation_impl.h"

#endif /*__H__LIB_DISCRETIZATION__SPATIAL_DISCRETIZATION__ELEM_DISC__CONSTANT_EQUATION__FV1__CONSTANT_EQUATION__*/
