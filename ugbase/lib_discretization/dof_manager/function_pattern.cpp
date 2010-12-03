/*
 * function_pattern.cpp
 *
 *  Created on: 03.12.2010
 *      Author: andreasvogel
 */

#include "function_pattern.h"
#include "lib_discretization/domain_util.h"

namespace ug{

bool
FunctionPattern::
add_discrete_function(const char* name, LocalShapeFunctionSetID id, int dim)
{
// 	if already locked, return false
	if(m_bLocked)
	{
		UG_LOG("Already fixed. Cannot change Distributor.\n");
		return false;
	}

//	check that subset handler exists
	if(m_pSH == NULL)
	{
		UG_LOG("ERROR in 'FunctionPattern::add_discrete_function': "
				"SubsetHandler not set.\n");
		return false;
	}

//	if no dimension passed, try to get dimension
	if(dim == -1)
		dim = DimensionOfSubsets(*m_pSH);

//	if still no dimension available, return false
	if(dim == -1)
	{
		UG_LOG("ERROR in 'FunctionPattern::add_discrete_function': "
				"Cannot find dimension for new function.\n");
		return false;
	}

//	create temporary subset group
	SubsetGroup tmpSubsetGroup;
	tmpSubsetGroup.set_subset_handler(*m_pSH);
	tmpSubsetGroup.add_all_subsets();

// 	add to function list, everywhere = true, copy SubsetGroup
	m_vFunction.push_back(Function(name, dim, id, true, tmpSubsetGroup));

//	we're done
	return true;
}

bool
FunctionPattern::
add_discrete_function(const char* name,
                      LocalShapeFunctionSetID id,
                      const SubsetGroup& SubsetIndices,
                      int dim)
{
// 	if already locked, return false
	if(m_bLocked)
	{
		UG_LOG("Already fixed. Cannot change Distributor.\n");
		return false;
	}

//	check that subset handler exists
	if(m_pSH == NULL)
	{
		UG_LOG("ERROR in 'FunctionPattern::add_discrete_function': "
				"SubsetHandler not set.\n");
		return false;
	}

//	check that subset handler are equal
	if(m_pSH != SubsetIndices.get_subset_handler())
	{
		UG_LOG("ERROR in 'FunctionPattern::add_discrete_function': "
				"SubsetHandler of SubsetGroup does "
				"not match SubsetHandler of FunctionPattern.\n");
		return false;
	}

//	if no dimension passed, try to get dimension
	if(dim == -1)
	{
		dim = SubsetIndices.get_highest_subset_dimension();
	}

//	if still no dimension available, return false
	if(dim == -1)
	{
		UG_LOG("ERROR in 'FunctionPattern::add_discrete_function': "
				"Cannot find dimension for new function.\n");
		return false;
	}


// 	add to function list, everywhere = false, copy SubsetGroup as given
	m_vFunction.push_back(Function(name, dim, id, false, SubsetIndices));

	return true;
}

} // end namespace ug
