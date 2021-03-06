/*
 * Copyright (c) 2012-2015:  G-CSC, Goethe University Frankfurt
 * Author: Martin Scherer
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

#ifndef PERIODIC_IDENTIFIER_H_
#define PERIODIC_IDENTIFIER_H_

#include "lib_grid/grid/grid.h"
#include "lib_grid/multi_grid.h"
#include "lib_grid/grid/grid_base_objects.h"

#include <set>

namespace ug {

/// Interface to match periodic geometric elements
/**
 *
 */
class IIdentifier {
public:
	virtual ~IIdentifier() {}
	virtual bool match(Vertex*, Vertex*) = 0;
	virtual bool match(Edge*, Edge*) = 0;
	virtual bool match(Face*, Face*) = 0;
	virtual bool match(Volume*, Volume*) {UG_THROW("not impled, because volume identification is not supported.")}
};

/// This class matches geometric elements which are parallel translated.
/**
 * Usage: class needs to be instantiated with the position attachment used on the Domain.
 * Before using any match methods, the translation vector needs to be set with set_shift()
 * \tparam <TPosAA>{class needs to be instantiated with the
 * position attachment used on the Domain.}
 */
template<class TPosAA> class ParallelShiftIdentifier: public IIdentifier {
public:
	virtual bool match(Vertex* v1, Vertex* v2) {return match_impl(v1, v2);}
	virtual bool match(Edge* e1, Edge* e2) {return match_impl(e1, e2);}
	virtual bool match(Face* f1, Face* f2) {return match_impl(f1, f2);}

	virtual ~ParallelShiftIdentifier() {}
	typedef typename TPosAA::ValueType AttachmentType;
	ParallelShiftIdentifier(TPosAA& aa) : m_aaPos(aa) {}
	void set_shift(AttachmentType& shift) {m_shift = shift; VecScale(m_shift_opposite, m_shift, -1);}
protected:
	AttachmentType m_shift;
	AttachmentType m_shift_opposite;
	TPosAA& m_aaPos;
	template<class TElem> bool match_impl(TElem*, TElem*) const;
};


//template<class TPosAA, int dim> class TransformationBasedIdentifier : public IIdentifier {
//public:
//	TransformationBasedIdentifier(TPosAA& aa) : m_aaPos(aa) {}
//	void setTransformation(MathMatrix<dim,dim>& T) {this->T = T;}
//
//	virtual bool match(Vertex*, Vertex*);
//	virtual bool match(Edge*, Edge*);
//	virtual bool match(Face*, Face*);
//protected:
//	MathMatrix<dim,dim> T;
//	TPosAA& m_aaPos;
//};

///
/**
 *
 */
class PeriodicBoundaryManager : public GridObserver {
public:

	/**
	 * A Group instance holds a master of type TElem and several children.
	 */
	template <class TElem, class Container = std::vector<TElem*> >
	class Group
	{
		public:
			typedef Container SlaveContainer;
			typedef typename Container::iterator SlaveIterator;
			// the set typedefs are used to check for periodicity after identification
			typedef typename std::pair<TElem*, TElem*> master_slave_pair;
			typedef typename std::set<master_slave_pair> unique_pairs;

			Group(TElem* m = NULL) : m_master(m) {}

			void add_slave(TElem* e) {
				UG_ASSERT(e, "add_slave: slave not valid");
				UG_ASSERT(e != m_master, "element already master!");
				m_slaves.push_back(e); }

			Container& get_slaves() { return m_slaves; }
			TElem* m_master;

		protected:
			Container m_slaves;
	};

	enum PeriodicStatus {
		P_SLAVE, P_SLAVE_MASTER_UNKNOWN, P_MASTER, P_NOT_PERIODIC
	};

	PeriodicBoundaryManager();
	~PeriodicBoundaryManager();

	// sets grid and inits group info attachment accessors
	void set_grid(Grid* g);
	Grid* get_grid() const;

	// sets the subset handler to use for element lookup
//	void set_subset_handler(ISubsetHandler* sh);

	///
	/**
	 * ATTENTION: method assumes, that e1 and e2 are matching!
	 * identifies element e1 with e2. If element type has sides, it will recursively
	 * ascend to the lowest dimension type (vertex) and identifies them.
	 * @param e1
	 * @param e2
	 * @param i identifier to use to perform geometrical matching of elements
	 */
	template <class TElem> void identify(TElem* e1, TElem* e2, IIdentifier& i);

	template <class TElem> bool is_periodic(TElem* e) const;
	template <class TElem> bool is_slave(TElem*) const;
	template <class TElem> bool is_master(TElem*) const;
	template <class TElem> TElem* master(TElem* e) const;
	template <class TElem> typename Group<TElem>::SlaveContainer* slaves(
			TElem* e) const;
	template <class TElem> void print_identification() const;


	/// grid observation methods
	virtual void grid_to_be_destroyed(Grid* grid);
	virtual void vertex_created(Grid* grid, Vertex* vrt,
										GridObject* pParent = NULL,
										bool replacesParent = false);

	virtual void edge_created(Grid* grid, Edge* e,
								GridObject* pParent = NULL,
								bool replacesParent = false);

	virtual void face_created(Grid* grid, Face* f,
								GridObject* pParent = NULL,
								bool replacesParent = false);

	virtual void vertex_to_be_erased(Grid* grid, Vertex* vrt,
									 Vertex* replacedBy = NULL);

	virtual void edge_to_be_erased(Grid* grid, Edge* e,
									 Edge* replacedBy = NULL);

	virtual void face_to_be_erased(Grid* grid, Face* f,
									 Face* replacedBy = NULL);

	/// checks that all elements of given gocs are periodic (called after identification)
	bool check_periodicity(const GridObjectCollection& goc1,
							  const GridObjectCollection& goc2,
							  ISubsetHandler* sh = NULL);

	/**	makes sure that all master/slave identifications are correct and that
	 * no unconnected masters or slaves exist.
	 * This method iterates over all elements of the grid and is thus rather expensive.*/
	void validity_check();

	/**
	 * sets the identifier instance to use for subset index si
	 * @param i identifier pointer
	 * @param si
	 */
//	void set_identifier(SmartPtr<IIdentifier> i, size_t si);

protected:
	// no copy construction allowed
	PeriodicBoundaryManager(const PeriodicBoundaryManager&)	{}

	/// grid instance we operate on
	MultiGrid* m_pGrid;

	/// store subset handler of domain to lookup element subset ids
//	ISubsetHandler* m_pSH;

	/// identifier mapping to subset indices
	/**
	 * stores IIdentifier pointers to subset index position to look them up
	 * in constant time.
	 */
//	std::vector<SmartPtr<IIdentifier> > m_vIdentifier;

	/// attachment accessors for Groups
	Grid::AttachmentAccessor<Vertex, Attachment<Group<Vertex>* > > m_aaGroupVRT;
	Grid::AttachmentAccessor<Edge, Attachment<Group<Edge>* > > m_aaGroupEDG;
	Grid::AttachmentAccessor<Face, Attachment<Group<Face>* > > m_aaGroupFCE;

	/// attachment accessors for PeriodicStatus
	Grid::AttachmentAccessor<Vertex, Attachment<PeriodicStatus> > m_aaPeriodicStatusVRT;
	Grid::AttachmentAccessor<Edge, Attachment<PeriodicStatus> > m_aaPeriodicStatusEDG;
	Grid::AttachmentAccessor<Face, Attachment<PeriodicStatus> > m_aaPeriodicStatusFCE;

	/// make element e slave of group g
	template <class TElem> void make_slave(Group<TElem>* g, TElem* e);
	template <class TElem> bool remove_slave(TElem* slave);

	// make e a master of g
	template <class TElem> void make_master(Group<TElem>* g, TElem* e);

	template <class TElem> void merge_groups(Group<TElem>* g0, Group<TElem>* g1);

	// get typed attachment accessor for group attachment
	template <class TElem>
	const Grid::AttachmentAccessor<TElem, Attachment<Group<TElem>* > >&
	get_group_accessor() const;

	template <class TElem>
	Grid::AttachmentAccessor<TElem, Attachment<Group<TElem>* > >&
	get_group_accessor();

	// get typed attachment accessor for periodic status attachment
	template <class TElem>
	const Grid::AttachmentAccessor<TElem, Attachment<PeriodicStatus> >&
	get_periodic_status_accessor() const;

	template <class TElem>
	Grid::AttachmentAccessor<TElem, Attachment<PeriodicStatus> >&
	get_periodic_status_accessor();

	// gets group of given element e
	template <class TElem> Group<TElem>* group(TElem* e) const;
	// set group attachment to element e
	template <class TElem> void set_group(Group<TElem>* g, TElem* e);
	template <class TElem> void remove_group(Group<TElem>* g);

	///	replaces all group occurrances of pParent by the specified elem
	template <class TElem>
	void replace_parent(TElem* e, TElem* pParent);

	/// handles creation of element type
	template <class TElem, class TParent>
	void handle_creation(TElem* e, TParent* pParent);

	/// handles deletion of element type
	template <class TElem>
	void handle_deletion(TElem* e, TElem* replacedBy);

	/// casts parent pointer to exact type before calling handle_creation
	template <class TElem>
	void handle_creation_cast_wrapper(TElem* e, GridObject* parent, bool replacesParent);

	template <class TElem, class TIterator>
	void check_elements_periodicity(
			TIterator begin,
			TIterator end,
			typename Group<TElem>::unique_pairs& s,
			ISubsetHandler* sh);

	template <class TElem>
	void validity_check();

};

///	Accesses attachements with consideration to periodic boundaries
/**	Slave element redirects to related master element
 *  also works in case of no periodic boundary conditions
 *  (but is then probably slower than standard attachment accessor)
 */
template <class TElem,class TAttachment>
class PeriodicAttachmentAccessor
{
	public:
		typedef typename TAttachment::ValueType	ValueType;
		typedef typename attachment_value_traits<ValueType>::reference RefType;
		typedef typename attachment_value_traits<ValueType>::const_reference ConstRefType;

		PeriodicAttachmentAccessor() : m_pbm(NULL)	{}

		PeriodicAttachmentAccessor(Grid& g, TAttachment& a) : m_pbm(NULL)
		{
			access(g, a);
		}

		PeriodicAttachmentAccessor(PeriodicBoundaryManager& pbm) : m_pbm(&pbm) {}

		bool access(Grid& g, TAttachment& a)
		{
			if (!(m_pbm)) m_pbm = g.periodic_boundary_manager();
			return m_aa.access(g, a);
		}

		RefType operator[](TElem* e)	{
			if(m_pbm && m_pbm->is_slave(e))
				return m_aa[m_pbm->master(e)];
			return m_aa[e];
		}

		ConstRefType operator[](TElem* e) const	{
			if(m_pbm && m_pbm->is_slave(e))
				return m_aa[m_pbm->master(e)];
			return m_aa[e];
		}

	private:
		Grid::AttachmentAccessor<TElem, TAttachment>	m_aa;
		PeriodicBoundaryManager* m_pbm;
};


/**
 * \brief identifies subset 1 with subset 2. If the grid of given domain has no
 * periodic boundary manager attached, one will be created.
 *
 * \param dom Domain the periodic boundary should be defined on
 * \param sInd1 subset index which elements should be identified with elements from
 * those of sInd2
 * \param sInd2 \see{sInd1}
 */
template <class TDomain>
void IdentifySubsets(TDomain& dom, int sInd1, int sInd2);

/**
 * \brief identifies subset 1 with subset 2. If the grid of given domain has no
 * periodic boundary manager attached, one will be created.
 *
 * \param dom Domain the periodic boundary should be defined on
 * \param sName1 subset name which elements should be identified with elements from
 * those of sName2
 * \param sName2 \see {sName1}
 */
template <class TDomain>
void IdentifySubsets(TDomain& dom, const char* sName1, const char* sName2);

} // end of namespace ug

// include implementation
#include "./periodic_boundary_manager_impl.hpp"

#endif /* PERIODIC_IDENTIFIER_H_ */
