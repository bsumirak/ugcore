/*
 * constant_equation_fv1.cpp
 *
 *  Created on: 13.05.2011
 *      Author: andreasvogel
 */

#include "constant_equation.h"

#include "common/util/provider.h"
#include "lib_discretization/spatial_discretization/disc_util/finite_volume_geometry.h"
#include "lib_discretization/spatial_discretization/disc_util/hanging_finite_volume_geometry.h"

namespace ug{


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//  Provide a generic implementation for all elements
//  (since this discretization can be implemented in a generic way)
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
prepare_element_loop()
{
	// all this will be performed outside of the loop over the elements.
	// Therefore it is not time critical.
	static const int refDim = TFVGeom<TElem, dim>::dim;

//	set local positions for rhs
	if(!TFVGeom<TElem, dim>::usesHangingNodes)
	{
		TFVGeom<TElem, dim>& geo = Provider::get<TFVGeom<TElem,dim> >();
		m_imVelocity.template 	set_local_ips<refDim>(geo.scvf_local_ips(),
		                   	                      geo.num_scvf_ips());
		m_imSource.template 		set_local_ips<refDim>(geo.scv_local_ips(),
		               		                      geo.num_scv_ips());
		m_imMassScale.template set_local_ips<refDim>(geo.scv_local_ips(),
		                                           geo.num_scv_ips());
	}

//	we're done
	return true;
}

template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
finish_element_loop()
{
//	nothing to do
	return true;
}

template<typename TDomain>
bool
FVConstantEquationElemDisc<TDomain>::
time_point_changed(number time)
{
//	set new time point at imports
	m_imVelocity.set_time(time);
	m_imSource.set_time(time);
	m_imMassScale.set_time(time);

//	this disc does not need the old time solutions, thus, return false
	return false;
}


template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
prepare_element(TElem* elem, const local_vector_type& u){
//	get dimension of reference element
	static const int refDim = TFVGeom<TElem, dim>::dim;

//	get corners
	m_vCornerCoords = this->template get_element_corners<TElem>(elem);

// 	Update Geometry for this element
	TFVGeom<TElem, dim>& geo = Provider::get<TFVGeom<TElem,dim> >();
	if(!geo.update(elem, &m_vCornerCoords[0], &(this->get_subset_handler())))
	{
		UG_LOG("FVConstantEquationElemDisc::prepare_element:"
				" Cannot update Finite Volume Geometry.\n"); return false;
	}

//	set local positions for rhs
	if(TFVGeom<TElem, dim>::usesHangingNodes)
	{
		m_imVelocity.template 	set_local_ips<refDim>(geo.scvf_local_ips(),
												  geo.num_scvf_ips());
		m_imSource.template 		set_local_ips<refDim>(geo.scv_local_ips(),
												  geo.num_scv_ips());
		m_imMassScale.template set_local_ips<refDim>(geo.scv_local_ips(),
												   geo.num_scv_ips());
	}

//	set global positions for rhs
	m_imVelocity.set_global_ips(geo.scvf_global_ips(), geo.num_scvf_ips());
	m_imSource.set_global_ips(geo.scv_global_ips(), geo.num_scv_ips());
	m_imMassScale.set_global_ips(geo.scv_global_ips(), geo.num_scv_ips());

//	we're done
	return true;
}

template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
assemble_JA(local_matrix_type& J, const local_vector_type& u)
{
//	no own contribution to jacobian (only due to linearized defect)
	return true;
}


template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
assemble_JM(local_matrix_type& J, const local_vector_type& u)
{
//	no own contribution to jacobian (only due to linearized defect)
	return true;
}


template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
assemble_A(local_vector_type& d, const local_vector_type& u)
{
// 	get finite volume geometry
	const static TFVGeom<TElem, dim>& geo	= Provider::get<TFVGeom<TElem,dim> >();

//	check if data given
	if(!m_imVelocity.data_given()) return true;

// 	loop Sub Control Volume Faces (SCVF)
	for(size_t ip = 0; ip < geo.num_scvf(); ++ip)
	{
	// 	get current SCVF
		const typename TFVGeom<TElem, dim>::SCVF& scvf = geo.scvf(ip);

	//	sum up convective flux using convection shapes
		const number conv_flux = VecDot(m_imVelocity[ip], scvf.normal());

	//  add to local defect
		d(_C_, scvf.from()) += conv_flux;
		d(_C_, scvf.to()  ) -= conv_flux;
	}

// 	we're done
	return true;
}


template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
assemble_M(local_vector_type& d, const local_vector_type& u)
{
// 	get finite volume geometry
	const static TFVGeom<TElem, dim>& geo = Provider::get<TFVGeom<TElem,dim> >();

// 	loop Sub Control Volumes (SCV)
	for(size_t ip = 0; ip < geo.num_scv(); ++ip)
	{
	// 	get current SCV
		const typename TFVGeom<TElem, dim>::SCV& scv = geo.scv(ip);

	// 	get associated node
		const int co = scv.node_id();

	//	mass value
		number val = scv.volume();

	//	multiply by scaling
		if(m_imMassScale.data_given())
			val *= m_imMassScale[ip];

	// 	Add to local defect
		d(_C_, co) += val;
	}

// 	we're done
	return true;
}


template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
inline
bool
FVConstantEquationElemDisc<TDomain>::
assemble_f(local_vector_type& d)
{
//	if zero data given, return
	if(!m_imSource.data_given()) return true;

// 	get finite volume geometry
	const static TFVGeom<TElem, dim>& geo
		= Provider::get<TFVGeom<TElem,dim> >();

// 	loop Sub Control Volumes (SCV)
	for(size_t ip = 0; ip < geo.num_scv(); ++ip)
	{
	// 	get current SCV
		const typename TFVGeom<TElem, dim>::SCV& scv = geo.scv(ip);

	// 	get associated node
		const int co = scv.node_id();

	// 	Add to local rhs
		d(_C_, co) += m_imSource[ip] * scv.volume();
	}

// 	we're done
	return true;
}


//	computes the linearized defect w.r.t to the velocity
template<typename TDomain>
template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
bool
FVConstantEquationElemDisc<TDomain>::
lin_defect_velocity(const local_vector_type& u)
{
//  get finite volume geometry
	const static TFVGeom<TElem, dim>& geo = Provider::get<TFVGeom<TElem,dim> >();

//	reset the values for the linearized defect
	m_imVelocity.clear_lin_defect();

//  loop Sub Control Volume Faces (SCVF)
	for(size_t ip = 0; ip < geo.num_scvf(); ++ip)
	{
	// get current SCVF
		const typename TFVGeom<TElem, dim>::SCVF& scvf = geo.scvf(ip);

	//	get derivatives to shapes at ip
		MathVector<dim>* vLinDef = m_imVelocity.lin_defect(ip, _C_);

	//	add parts for both sides of scvf
		vLinDef[scvf.from()] += scvf.normal();
		vLinDef[scvf.to()] -= scvf.normal();
	}

//	we're done
	return true;
}

//	computes the linearized defect w.r.t to the source
template<typename TDomain>
template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
bool
FVConstantEquationElemDisc<TDomain>::
lin_defect_source(const local_vector_type& u)
{
//  get finite volume geometry
	const static TFVGeom<TElem, dim>& geo = Provider::get<TFVGeom<TElem,dim> >();

// 	loop Sub Control Volumes (SCV)
	for(size_t ip = 0; ip < geo.num_scv(); ++ip)
	{
	// 	get current SCV
		const typename TFVGeom<TElem, dim>::SCV& scv = geo.scv(ip);

	// 	get associated node
		const int co = scv.node_id();

	// 	Add to local defect
		m_imSource.lin_defect(ip, _C_, co) = scv.volume();
	}

//	we're done
	return true;
}

//	computes the linearized defect w.r.t to the mass scale
template<typename TDomain>
template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
bool
FVConstantEquationElemDisc<TDomain>::
lin_defect_mass_scale(const local_vector_type& u)
{
// 	get finite volume geometry
	const static TFVGeom<TElem, dim>& geo	= Provider::get<TFVGeom<TElem,dim> >();

//	reset all values
	m_imMassScale.clear_lin_defect();

// 	loop Sub Control Volumes (SCV)
	for(size_t co = 0; co < geo.num_scv(); ++co)
	{
	// 	get current SCV
		const typename TFVGeom<TElem, dim>::SCV& scv = geo.scv(co);

	// 	Check associated node
		UG_ASSERT(co == scv.node_id(), "Only one shape per SCV");

	// 	Add to local defect
		m_imMassScale.lin_defect(co, _C_, co) = scv.volume();
	}

//	we're done
	return true;
}

//	computes the linearized defect w.r.t to the velocity
template<typename TDomain>
template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
bool
FVConstantEquationElemDisc<TDomain>::
ex_concentration(const local_vector_type& u,
                 const MathVector<dim>* vGlobIP,
                 const MathVector<TFVGeom<TElem, dim>::dim>* vLocIP,
                 const size_t nip,
                 number* vValue,
                 bool bDeriv,
                 std::vector<std::vector<std::vector<number> > >& vvvDeriv)
{
//  get finite volume geometry
	const static TFVGeom<TElem, dim>& geo = Provider::get<TFVGeom<TElem,dim> >();

//	number of shape functions
	static const size_t numSh =
			reference_element_traits<TElem>::reference_element_type::num_corners;

//	FV1 SCVF ip
	if(vLocIP	== geo.scvf_local_ips())
	{
	//	Loop Sub Control Volume Faces (SCVF)
		for(size_t ip = 0; ip < geo.num_scvf(); ++ip)
		{
		// 	Get current SCVF
			const typename TFVGeom<TElem, dim>::SCVF& scvf = geo.scvf(ip);

		//	compute concentration at ip
			vValue[ip] = 0.0;
			for(size_t sh = 0; sh < scvf.num_sh(); ++sh)
				vValue[ip] += u(_C_, sh) * scvf.shape(sh);

		//	compute derivative w.r.t. to unknowns iff needed
			if(bDeriv)
				for(size_t sh = 0; sh < scvf.num_sh(); ++sh)
					vvvDeriv[ip][_C_][sh] = scvf.shape(sh);
		}
	}
//	FV1 SCV ip
	else if(vLocIP == geo.scv_local_ips())
	{
	//	solution at ip
		for(size_t sh = 0; sh < numSh; ++sh)
			vValue[sh] = u(_C_, sh);

	//	set derivatives if needed
		if(bDeriv)
			for(size_t sh = 0; sh < numSh; ++sh)
				for(size_t sh2 = 0; sh2 < numSh; ++sh2)
					vvvDeriv[sh][_C_][sh2] = (sh==sh2) ? 1.0 : 0.0;
	}
// 	others not implemented
	else
	{
		UG_LOG("Evaluation not implemented.");
		return false;
	}

//	we're done
	return true;
}

//	computes the linearized defect w.r.t to the velocity
template<typename TDomain>
template <typename TElem, template <class Elem, int WorldDim> class TFVGeom>
bool
FVConstantEquationElemDisc<TDomain>::
ex_concentration_grad(const local_vector_type& u,
                      const MathVector<dim>* vGlobIP,
                      const MathVector<TFVGeom<TElem, dim>::dim>* vLocIP,
                      const size_t nip,
                      MathVector<dim>* vValue,
                      bool bDeriv,
                      std::vector<std::vector<std::vector<MathVector<dim> > > >& vvvDeriv)
{
// 	Get finite volume geometry
	static const TFVGeom<TElem, dim>& geo = Provider::get<TFVGeom<TElem,dim> >();

//	FV1 SCVF ip
	if(vLocIP == geo.scvf_local_ips())
	{
	//	Loop Sub Control Volume Faces (SCVF)
		for(size_t ip = 0; ip < geo.num_scvf(); ++ip)
		{
		// 	Get current SCVF
			const typename TFVGeom<TElem, dim>::SCVF& scvf = geo.scvf(ip);

			VecSet(vValue[ip], 0.0);
			for(size_t sh = 0; sh < scvf.num_sh(); ++sh)
				VecScaleAppend(vValue[ip], u(_C_, sh), scvf.global_grad(sh));

			if(bDeriv)
				for(size_t sh = 0; sh < scvf.num_sh(); ++sh)
					vvvDeriv[ip][_C_][sh] = scvf.global_grad(sh);
		}
	}
// others not implemented
	else
	{
		UG_LOG("Evaluation not implemented.");
		return false;
	}

//	we're done
	return true;
};

////////////////////////////////////////////////////////////////////////////////
//	Constructor
////////////////////////////////////////////////////////////////////////////////

template<typename TDomain>
FVConstantEquationElemDisc<TDomain>::
FVConstantEquationElemDisc()
{
//	register assemling functions
	register_all_fv1_funcs(false);

//	register exports
	register_export(m_exConcentration);
	register_export(m_exConcentrationGrad);

//	register imports
	register_import(m_imVelocity);
	register_import(m_imSource);
	register_import(m_imMassScale);

	m_imMassScale.set_mass_part(true);
}


////////////////////////////////////////////////////////////////////////////////
//	register assemble functions
////////////////////////////////////////////////////////////////////////////////

// register for 1D
template<typename TDomain>
void
FVConstantEquationElemDisc<TDomain>::
register_all_fv1_funcs(bool bHang)
{
//	get all grid element types in this dimension and below
	typedef typename domain_traits<dim>::AllElemList ElemList;

//	switch assemble functions
	if(!bHang) boost::mpl::for_each<ElemList>( RegisterFV1<FV1Geometry>(this) );
	else boost::mpl::for_each<ElemList>( RegisterFV1<HFV1Geometry>(this) );
}

template<typename TDomain>
template<typename TElem, template <class Elem, int WorldDim> class TFVGeom>
void
FVConstantEquationElemDisc<TDomain>::
register_fv1_func()
{
	ReferenceObjectID id = geometry_traits<TElem>::REFERENCE_OBJECT_ID;
	typedef this_type T;
	static const int refDim = reference_element_traits<TElem>::dim;

	set_prep_elem_loop_fct(id, &T::template prepare_element_loop<TElem, TFVGeom>);
	set_prep_elem_fct(	 id, &T::template prepare_element<TElem, TFVGeom>);
	set_fsh_elem_loop_fct( id, &T::template finish_element_loop<TElem, TFVGeom>);
	set_ass_JA_elem_fct(		 id, &T::template assemble_JA<TElem, TFVGeom>);
	set_ass_JM_elem_fct(		 id, &T::template assemble_JM<TElem, TFVGeom>);
	set_ass_dA_elem_fct(		 id, &T::template assemble_A<TElem, TFVGeom>);
	set_ass_dM_elem_fct(		 id, &T::template assemble_M<TElem, TFVGeom>);
	set_ass_rhs_elem_fct(	 id, &T::template assemble_f<TElem, TFVGeom>);

//	set computation of linearized defect w.r.t velocity
	m_imVelocity. reg_lin_defect_fct(id, this, &T::template lin_defect_velocity<TElem, TFVGeom>);
	m_imSource.	  reg_lin_defect_fct(id, this, &T::template lin_defect_source<TElem, TFVGeom>);
	m_imMassScale.reg_lin_defect_fct(id, this, &T::template lin_defect_mass_scale<TElem, TFVGeom>);

//	exports
	m_exConcentration.	  template set_fct<T,refDim>(id, this, &T::template ex_concentration<TElem, TFVGeom>);
	m_exConcentrationGrad.template set_fct<T,refDim>(id, this, &T::template ex_concentration_grad<TElem, TFVGeom>);
}

////////////////////////////////////////////////////////////////////////////////
//	explicit template instantiations
////////////////////////////////////////////////////////////////////////////////

template class FVConstantEquationElemDisc<Domain1d>;
template class FVConstantEquationElemDisc<Domain2d>;
template class FVConstantEquationElemDisc<Domain3d>;


} // namespace ug

