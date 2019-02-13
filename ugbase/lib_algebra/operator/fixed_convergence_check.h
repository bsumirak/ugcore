/*
 * Copyright (c) 2013-2015:  G-CSC, Goethe University Frankfurt
 * Author: Martin Rupp
 * 
 * This file is part of UG4.
 * 
 * UG4 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 3 (as published by the
 * Free Software Foundation) with the following additional attribution
 * requirements (according to LGPL/GPL v3 §7):
 * 
 * (1) The following notice must be displayed in the Appropriate Legal Notices
 * of covered and combined works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (2) The following notice must be displayed at a prominent place in the
 * terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (3) The following bibliography is recommended for citation and must be
 * preserved in all covered files:
 * "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
 *   parallel geometric multigrid solver on hierarchically distributed grids.
 *   Computing and visualization in science 16, 4 (2013), 151-164"
 * "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
 *   flexible software system for simulating pde based models on high performance
 *   computers. Computing and visualization in science 16, 4 (2013), 165-179"
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 */

#ifndef FIXED_CONVERGENCE_CHECK_H_
#define FIXED_CONVERGENCE_CHECK_H_
#include "convergence_check.h"

namespace ug{

template <typename TVector>
class FixedConvergenceCheck : public IConvergenceCheck<TVector>
{
	public:
		FixedConvergenceCheck(int numIterations)
		{
			m_numIterations = numIterations;
		}

		/// sets the given start defect
		virtual void start_defect(number defect)
		{
			m_currentStep=0;
		}

		/// computes the start defect and set it
		virtual void start(const TVector& d)
		{
			m_currentStep=0;
		}

		/// sets the update for the current defect
		virtual void update_defect(number defect)
		{
			m_currentStep++;
		}

		/// computes the defect and sets it a the next defect value
		virtual void update(const TVector& d)
		{
			m_currentStep++;
		}

		/** iteration_ended
		 *
		 *	Checks if the iteration must be ended.
		 *	This can be due to convergence or divergence.
		 *
		 * \return 	true 		if iteration ended
		 * 			false 		if iteration can and must be continued until convergence
		 */
		virtual bool iteration_ended()
		{
			if(step() >= m_numIterations) return true;
			return false;
		}

		/** post
		 *
		 * post-processes the iteration. Some informative outputs of the status of
		 * the iteration after finishing the iteration can be placed here
		 *
		 * \return 	true 		if iteration was successful
		 * 			false 		if iteration did not lead to a satisfying result
		 */
		virtual bool post()
		{
			return true;
		}

		/////////////////////////////////////
		// informations about current status
		/////////////////////////////////////

		/// returns the current defect
		virtual number defect() const { UG_ASSERT(0, "not provided by FixedConvergenceCheck"); return 0;}

		/// returns the current number of steps
		virtual int step() const { return m_currentStep; }

		// returns the current relative reduction
		virtual number reduction() const { UG_ASSERT(0, "not provided by FixedConvergenceCheck");  return 0;}

		// returns the current convergence rate
		virtual number rate() const { return 0;}

		// returns the averaged convergence rate
		virtual number avg_rate() const { UG_ASSERT(0, "not provided by FixedConvergenceCheck");  return 0;}

		virtual void print_line(std::string line)
		{
		}
		////////////////
		// output style
		////////////////

		int get_offset() const {return m_offset;}
		void set_offset(int offset){m_offset = offset;}
		void set_symbol(char symbol){ }
		void set_name(std::string name) {}
		void set_info(std::string info) {}

		/// clone the object
		virtual SmartPtr<IConvergenceCheck<TVector> > clone()
		{
			SmartPtr<FixedConvergenceCheck<TVector> > newInst(new FixedConvergenceCheck<TVector>(m_numIterations));
			return newInst;
		}

		virtual std::string config_string() const
		{
			std::stringstream ss;
			ss << "FixedConvergenceCheck( fix # steps = " << m_numIterations << ")";
			return ss.str();
		}

		/// virtual destructor
		virtual ~FixedConvergenceCheck() {};

	private:
		int m_offset;
		int m_numIterations;
		int m_currentStep;
};


}
#endif /* FIXED_CONVERGENCE_CHECK_H_ */
