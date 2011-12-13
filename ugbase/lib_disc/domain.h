/*
 * domain.h
 *
 *  Created on: 17.12.2009
 *      Author: andreasvogel
 */

#ifndef __H__UG__LIB_DISC__DOMAIN__
#define __H__UG__LIB_DISC__DOMAIN__

#include "lib_grid/lg_base.h"
#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>

#ifdef UG_PARALLEL
#include "lib_grid/parallelization/distributed_grid.h"
#endif

namespace ug{

/**
 * Domain
 *
 * \defgroup lib_disc_domain Domain
 * \ingroup lib_discretization
 */

/// \ingroup lib_disc_domain
/// @{

/// describes physical domain
/**
 * A Domain collects and exports relevant informations about the
 * physical domain, that is intended to be discretized. It will be used as
 * a template Parameter in several classes to distinguish at compile-time
 * between needed types and parameters. It mainly has a grid and a subset
 * handler for the grid to define different physical subsets. In addition
 * to a grid, that may only contain topological informations a domain always
 * has a Position Attachment holding the physical coodinates.
 * \tparam	d				World Dimension
 * \tparam	TGrid			Grid type
 * \tparam	TSubsetHandler	Subset Handler type
 */
template <int d, typename TGrid, typename TSubsetHandler>
class Domain {
	public:
	// 	World dimension
		static const int dim = d;

	// 	Type of position coordinates
		typedef MathVector<dim> position_type;

	// 	Grid type
		typedef TGrid grid_type;

	// 	Subset Handler type
		typedef TSubsetHandler subset_handler_type;

	// 	Type of Position Attachment
		typedef Attachment<position_type> position_attachment_type;

	// 	Type of Accessor to the Position Data Attachment
		typedef Grid::VertexAttachmentAccessor<position_attachment_type>
					position_accessor_type;

	//	Distributed Grid Manager (for parallel case)
#ifdef UG_PARALLEL
		typedef DistributedGridManager distributed_grid_manager_type;
#endif

	public:
	///	Default constructor
	/**
	 * creates an empty domain. Grid and Subset Handler are set up. The
	 * Distributed Grid Manager is set in the parallel case.
	 * \param[in]	options		Grid Options (optinal)
	 */
		Domain() :
			m_grid(GRIDOPT_NONE), m_sh(m_grid)
#ifdef UG_PARALLEL
		, m_distGridMgr(NULL)
#endif
			{
			//	Depending on the dimesion, we'll activeate different options.
			//	Otherwise we probably would waste memory...
			//	In any case, sides of elements should always be present
				uint gridOpts = GRIDOPT_AUTOGENERATE_SIDES;

			//	Furthermore vertices should store associated elements.
			//	This option depends on the dimension of the domain
				if(dim > 0)
					gridOpts |= VRTOPT_STORE_ASSOCIATED_EDGES;
				if(dim > 1)
					gridOpts |= VRTOPT_STORE_ASSOCIATED_FACES;
				if(dim > 2)
					gridOpts |= VRTOPT_STORE_ASSOCIATED_VOLUMES;

			//	thats it for now. One could think about enabling
			//	FACEOPT_STORE_ASSOCIATED_EDGES, VOLOPT_STORE_ASSOCIATED_EDGES
			//	and VOLOPT_STORE_ASSOCIATED_FACES. However this costs considerably
			//	more memory compared to the performance benefits.
			//	Now set the options
				m_grid.set_options(gridOpts);

			//	get position attachment
				m_aPos = GetDefaultPositionAttachment<position_attachment_type>();

			// 	let position accessor access Vertex Coordinates
				if(!m_grid.template has_attachment<VertexBase>(m_aPos))
					m_grid.template attach_to<VertexBase>(m_aPos);
				m_aaPos.access(m_grid, m_aPos);

#ifdef UG_PARALLEL
			//	create Distributed Grid Manager
				m_distGridMgr = new DistributedGridManager(m_grid);
#endif
			}

	///	Destructor
		~Domain()
		{
#ifdef UG_PARALLEL
			if(m_distGridMgr)
				delete m_distGridMgr;
#endif
		}

	///	World Dimension
		inline int get_dim() const {return dim;}
		
	///	returns Grid
		inline TGrid& grid() {return m_grid;};

	///	const access to Grid
		inline const TGrid& grid() const {return m_grid;};

	///	returns Subset Handler
		inline TSubsetHandler& subset_handler() {return m_sh;};

	///	const access to Subset Handler
		inline const TSubsetHandler& subset_handler() const {return m_sh;};

	///	returns Position Attachment
		inline position_attachment_type& position_attachment(){return m_aPos;}

	///	const access to Position Attachment
		inline const position_attachment_type& position_attachment() const {return m_aPos;}

	///	get Position Accessor
		inline position_accessor_type& position_accessor() {return m_aaPos;}

	///	const access to Position Accessor
		inline const position_accessor_type& position_accessor() const{return m_aaPos;}

#ifdef UG_PARALLEL
	///	returns Distributed Grid Manager
		inline DistributedGridManager* distributed_grid_manager(){return m_distGridMgr;}
#endif

	///	returns the message hub of the grid
		SPMessageHub message_hub() const {return m_grid.message_hub();}

	protected:
		TGrid m_grid;			///< Grid
		TSubsetHandler m_sh;	///< Subset Handler

		position_attachment_type m_aPos;	///<Position Attachment
		position_accessor_type m_aaPos;		///<Accessor

	//	for parallelization only. NULL in serial mode.
#ifdef UG_PARALLEL
		DistributedGridManager*	m_distGridMgr;	///< Parallel Grid Manager
#endif
};

typedef Domain<1, MultiGrid, MGSubsetHandler> Domain1d;
typedef Domain<2, MultiGrid, MGSubsetHandler> Domain2d;
typedef Domain<3, MultiGrid, MGSubsetHandler> Domain3d;


////////////////////////////////////////////////////////////////////////////////
//	Element types for different dimensions
////////////////////////////////////////////////////////////////////////////////

/**
 * This Class provides boost::mpl::lists storing the type of elements used in
 * for the Domain. It can be used to control dimension dependent builds, where
 * not all template instantiations are available (e.g. a Hexahedron in 1d,2d, etc)
 * While DimElemList returns the Element Types in the dimenion of the domain,
 * the list AllElemList returns all elements contained in the Domain-dimension
 * and the dimensions below.
 */
template <int dim> struct domain_traits;

// 0d
template <> struct domain_traits<0> {
typedef boost::mpl::list<Vertex> DimElemList;
typedef boost::mpl::list<Vertex> AllElemList;

typedef geometry_traits<VertexBase>::const_iterator const_iterator;
typedef geometry_traits<VertexBase>::iterator iterator;

typedef geometry_traits<VertexBase>::geometric_base_object geometric_base_object;

const static size_t MaxNumVerticesOfElem = 1;
};

// 1d
template <> struct domain_traits<1> {
typedef boost::mpl::list<Edge> DimElemList;
typedef boost::mpl::list<Edge> AllElemList;
typedef boost::mpl::list<> ManifoldElemList;

typedef geometry_traits<EdgeBase>::const_iterator const_iterator;
typedef geometry_traits<EdgeBase>::iterator iterator;

typedef geometry_traits<EdgeBase>::geometric_base_object geometric_base_object;

const static size_t MaxNumVerticesOfElem = 2;

typedef MathVector<1> position_type;
typedef Attachment<position_type> position_attachment_type;
typedef Grid::VertexAttachmentAccessor<position_attachment_type> position_accessor_type;

};

// 2d
template <> struct domain_traits<2> {
typedef boost::mpl::list<Triangle, Quadrilateral> DimElemList;
typedef boost::mpl::list<Edge, Triangle, Quadrilateral> AllElemList;
typedef boost::mpl::list<Edge> ManifoldElemList;

typedef geometry_traits<Face>::const_iterator const_iterator;
typedef geometry_traits<Face>::iterator iterator;

typedef geometry_traits<Face>::geometric_base_object geometric_base_object;

const static size_t MaxNumVerticesOfElem = 4;

typedef MathVector<2> position_type;
typedef Attachment<position_type> position_attachment_type;
typedef Grid::VertexAttachmentAccessor<position_attachment_type> position_accessor_type;
};

// 3d
template <> struct domain_traits<3> {
typedef boost::mpl::list<Tetrahedron, Prism, Pyramid, Hexahedron> DimElemList;
typedef boost::mpl::list<Edge, Triangle, Quadrilateral,
								 Tetrahedron, Prism, Pyramid, Hexahedron> AllElemList;
typedef boost::mpl::list<Triangle, Quadrilateral> ManifoldElemList;

typedef geometry_traits<Volume>::const_iterator const_iterator;
typedef geometry_traits<Volume>::iterator iterator;

typedef geometry_traits<Volume>::geometric_base_object geometric_base_object;

const static size_t MaxNumVerticesOfElem = 8;

typedef MathVector<3> position_type;
typedef Attachment<position_type> position_attachment_type;
typedef Grid::VertexAttachmentAccessor<position_attachment_type> position_accessor_type;

};


} // end namespace ug

/// @}

#endif /* __H__UG__LIB_DISC__DOMAIN__ */
