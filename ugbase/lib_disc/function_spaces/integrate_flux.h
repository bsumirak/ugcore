/*
 * integrate_flux.h
 *
 *  Created on: 18.06.2012
 *      Author: andreasvogel
 */

#ifndef __H__UG__LIB_DISC__FUNCTION_SPACES__INTEGRATE_FLUX__
#define __H__UG__LIB_DISC__FUNCTION_SPACES__INTEGRATE_FLUX__

#include <cmath>

#include "common/common.h"
#include "lib_disc/common/subset_group.h"
#include "lib_disc/common/function_group.h"
#include "lib_disc/common/groups_util.h"

#include "lib_disc/assemble_interface.h"
#include "lib_disc/spatial_disc/elem_disc/elem_disc_interface.h"
#include "lib_disc/spatial_disc/constraints/constraint_interface.h"

namespace ug{

template <typename TGridFunction, int dim>
void IntegrateDiscFlux(std::vector<number>& vValue,
                       const TGridFunction& rDefect,
                       const FunctionGroup& fctGrp,
                       int si)
{
//	get iterators for all elems on subset
	typedef typename TGridFunction::template dim_traits<dim>::const_iterator const_iterator;
	typedef typename TGridFunction::template dim_traits<dim>::geometric_base_object element_type;
	const_iterator iter = rDefect.template begin<element_type>(si);
	const_iterator iterEnd = rDefect.template end<element_type>(si);

//	return if no dofs on this element types
	if(rDefect.max_dofs(dim) == 0) return;

//	check sizes
	if(vValue.size() != fctGrp.size())
		UG_THROW("IntegrateDiscFlux: number of values and number of function mismatch.");

//	allocate memory for indices
	std::vector<std::vector<MultiIndex<2> > > vInd(fctGrp.size());

//	loop elements of subset
	for( ; iter != iterEnd; ++iter)
	{
	//	get element
		element_type* elem = *iter;

	//	get multi-indices of elem
		for(size_t f = 0; f < fctGrp.size(); ++f)
			rDefect.inner_multi_indices(elem, fctGrp[f], vInd[f]);

	//	sum values
		for(size_t f = 0; f < fctGrp.size(); ++f)
			for(size_t i = 0; i < vInd[f].size(); ++i)
				vValue[f] += BlockRef( rDefect[ vInd[f][i][0] ], vInd[f][i][1]);
	}
}


template <typename TGridFunction>
number IntegrateDiscFlux(SmartPtr<IAssemble<typename TGridFunction::algebra_type> > spAssemble,
                         SmartPtr<TGridFunction> spGridFct,
                         const char* pCmps,
                         const char* subsets)
{
//	read subsets
	SubsetGroup ssGrp(spGridFct->domain()->subset_handler());
	if(subsets != NULL)
	{
		ConvertStringToSubsetGroup(ssGrp, subsets);
		if(!SameDimensionsInAllSubsets(ssGrp))
			UG_THROW("IntegrateDiscFlux: Subsets '"<<subsets<<"' do not have same dimension."
					 "Can not integrate on subsets of different dimensions.");
	}
	else
	{
	//	add all subsets and remove lower dim subsets afterwards
		ssGrp.add_all();
		RemoveLowerDimSubsets(ssGrp);
	}

//	read functions
	FunctionGroup fctGrp;
	try{
		ConvertStringToFunctionGroup(fctGrp, spGridFct->function_pattern(), pCmps);
	}
	UG_CATCH_THROW("IntegrateDiscFlux: Cannot parse functions for p.");

//	check if something to do
	if(fctGrp.size() == 0) return 0;

//	create vector
	TGridFunction Defect(*spGridFct);

//	remember enabled-flags
	const int ElemTypesEnabled = spAssemble->elem_discs_enabled();
	const int ConstraintTypesEnabled = spAssemble->constraints_enabled();

//	remove bnd components
	spAssemble->enable_elem_discs(ElemTypesEnabled & (~EDT_BND));
	spAssemble->enable_constraints(ConstraintTypesEnabled & (~CT_DIRICHLET));

//	compute defect
	spAssemble->assemble_defect(Defect, *spGridFct);

//	reset flags to original status
	spAssemble->enable_elem_discs(ElemTypesEnabled);
	spAssemble->enable_constraints(ConstraintTypesEnabled);

//	reset value
	std::vector<number> vValue(fctGrp.size(), 0);

//	loop subsets
	for(size_t i = 0; i < ssGrp.num_subsets(); ++i)
	{
	//	get subset index
		const int si = ssGrp[i];

	//	error if function is not defined in subset
		for(size_t f = 0; f < fctGrp.size(); ++f)
		{
			if(!spGridFct->is_def_in_subset(fctGrp[f], si))
				UG_THROW("IntegrateDiscFlux: Function "<<fctGrp[f]<<" not defined "
						 "on subset "<<si<<".");
		}

	//	check dimension
		if(ssGrp.dim(i) > TGridFunction::dim)
			UG_THROW("IntegrateDiscFlux: Dimension of subset is "<<ssGrp.dim(i)<<", but "
					 " World Dimension is "<<TGridFunction::dim<<". Cannot integrate this.");

	//	integrate elements of subset
		try{
		for(int d = 0; d < ssGrp.dim(i); ++d)
		{
			if(d==0) 	  IntegrateDiscFlux<TGridFunction, 0>(vValue, Defect, fctGrp, si);
			else if(d==1) IntegrateDiscFlux<TGridFunction, 1>(vValue, Defect, fctGrp, si);
			else if(d==2) IntegrateDiscFlux<TGridFunction, 2>(vValue, Defect, fctGrp, si);
			else if(d==3) IntegrateDiscFlux<TGridFunction, 3>(vValue, Defect, fctGrp, si);
			else UG_THROW("IntegrateDiscFlux: Dimension "<<d<<" not supported. "
							  " World dimension is "<<TGridFunction::dim<<".");
		}
		}
		UG_CATCH_THROW("IntegrateSubsets: Integration failed on subset "<<si);

	}

#ifdef UG_PARALLEL
	// sum over processes
	if(pcl::GetNumProcesses() > 1)
	{
		pcl::ProcessCommunicator com;
		for(size_t f = 0; f < vValue.size(); ++f)
		{
			number local = vValue[f];
			com.allreduce(&local, &vValue[f], 1, PCL_DT_DOUBLE, PCL_RO_SUM);
		}
	}
#endif

//	return the result
	if(vValue.size() == 1) return vValue[0];
	else
	{
		number sum = 0;
		for(size_t f = 0; f < vValue.size(); ++f){
			UG_LOG("Integral for '"<<fctGrp.name(f)<<"': "<<vValue[f]<<"\n");
			sum += vValue[f];
		}
		return sum;
	}
}

} // end namespace ug

#endif /* __H__UG__LIB_DISC__FUNCTION_SPACES__INTEGRATE_FLUX__ */
