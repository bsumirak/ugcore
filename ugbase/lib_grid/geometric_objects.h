//	created by Sebastian Reiter
//	s.b.reiter@googlemail.com
//	y08 m11 d04

#ifndef __H__LIB_GRID__GEOMETRIC_OBJECTS__
#define __H__LIB_GRID__GEOMETRIC_OBJECTS__

#include "grid/geometric_base_objects.h"
#include "common/math/ugmath.h"

namespace ug
{
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	Reference-Object IDs
///	these ids are used to identify the shape of a geometric object.
enum ReferenceObjectID
{
	ROID_INVALID = -1,
	ROID_VERTEX,
	ROID_EDGE,
	ROID_TRIANGLE,
	ROID_QUADRILATERAL,
	ROID_TETRAHEDRON,
	ROID_HEXAHEDRON,
	ROID_PRISM,
	ROID_PYRAMID,
	NUM_REFERENCE_OBJECTS
};

////////////////////////////////////////////////////////////////////////
//	shared pipe sections vertex
///	These numbers define where in the vertex-section-container a vertex will be stored.
enum SharedPipeSectionVertex
{
	SPSVRT_NONE = -1,
	SPSVRT_VERTEX = 0,
	SPSVRT_HANGING_VERTEX = 1
};

////////////////////////////////////////////////////////////////////////
//	shared pipe sections edge
///	These numbers define where in the edge-section-container an edge will be stored.
enum SharedPipeSectionEdge
{
	SPSEDGE_NONE = -1,
	SPSEDGE_EDGE = 0,
	SPSEDGE_CONSTRAINED_EDGE = 1,
	SPSEDGE_CONSTRAINING_EDGE = 2
};

////////////////////////////////////////////////////////////////////////
//	shared pipe sections face
///	These numbers define where in the face-section-container a face will be stored.
enum SharedPipeSectionFace
{
	SPSFACE_NONE = -1,
	SPSFACE_TRIANGLE = 0,
	SPSFACE_QUADRILATERAL = 1,
	SPSFACE_CONSTRAINED_TRIANGLE = 2,
	SPSFACE_CONSTRAINED_QUADRILATERAL = 3,
	SPSFACE_CONSTRAINING_TRIANGLE = 4,
	SPSFACE_CONSTRAINING_QUADRILATERAL = 5
};

////////////////////////////////////////////////////////////////////////
//	shared pipe sections volume
///	These numbers define where in the volume-section-container a volume will be stored.
enum SharedPipeSectionVolume
{
	SPSVOL_NONE = -1,
	SPSVOL_TETRAHEDRON = 0,
	SPSVOL_HEXAHEDRON = 1,
	SPSVOL_PRISM = 2,
	SPSVOL_PYRAMID = 3
};

////////////////////////////////////////////////////////////////////////
//	VERTICES

////////////////////////////////////////////////////////////////////////
//	Vertex
///	A basic vertex-type
/**
 * This type represents the most commonly used vertex.
 *
 * \ingroup GeometricObjects
 */
class Vertex : public VertexBase
{
	friend class Grid;
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Vertex*>(pObj) != NULL;}

		virtual ~Vertex()	{}

		virtual GeometricObject* create_empty_instance() const	{return new Vertex;}

		virtual int shared_pipe_section() const	{return SPSVRT_VERTEX;}
		virtual int base_object_type_id() const	{return VERTEX;}
		virtual int reference_object_id() const {return ROID_VERTEX;}
};

template <>
class geometry_traits<Vertex>
{
	public:
		typedef GenericGeometricObjectIterator<Vertex*, VertexBaseIterator>	iterator;
		typedef VertexBase	geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSVRT_VERTEX,
			BASE_OBJECT_TYPE_ID = VERTEX
		};
};

typedef geometry_traits<Vertex>::iterator VertexIterator;

////////////////////////////////////////////////////////////////////////
//	HangingVertex
///	A vertex appearing on edges or faces.
/**
 * HangingVertices appear during hangin-vertex-refinement.
 * They should be treated with care, since they are often referenced
 * by other elements, as i.e. \sa ConstrainingEdge.
 *
 * \ingroup GeometricObjects
 */
class HangingVertex : public VertexBase
{
	friend class Grid;
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<HangingVertex*>(pObj) != NULL;}

		HangingVertex()	: m_pParent(NULL)	{}
		virtual ~HangingVertex()	{}

		virtual GeometricObject* create_empty_instance() const	{return new HangingVertex;}

		virtual int shared_pipe_section() const	{return SPSVRT_HANGING_VERTEX;}
		virtual int base_object_type_id() const	{return VERTEX;}
		virtual int reference_object_id() const {return ROID_VERTEX;}

		inline void set_parent(GeometricObject* pParent)	{m_pParent = pParent;}
		inline GeometricObject* get_parent()	{return m_pParent;}
		inline int get_parent_base_object_type_id()	{return m_pParent->base_object_type_id();}

		inline const vector2& get_local_coordinates() const	{return m_localCoord;}
		inline number get_local_coordinate_1() const		{return m_localCoord.x;}
		inline number get_local_coordinate_2() const		{return m_localCoord.y;}

		inline void set_local_coordinates(number x, number y)		{m_localCoord.x = x; m_localCoord.y = y;}
		inline void set_local_coordinates(const vector2& coords)	{m_localCoord = coords;}
		inline void set_local_coordinate_1(number coord) 	{m_localCoord.x = coord;}
		inline void set_local_coordinate_2(number coord) 	{m_localCoord.y = coord;}


	protected:
		GeometricObject*	m_pParent;
		vector2				m_localCoord;


};

template <>
class geometry_traits<HangingVertex>
{
	public:
		typedef GenericGeometricObjectIterator<HangingVertex*, VertexBaseIterator>	iterator;
		typedef VertexBase	geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSVRT_HANGING_VERTEX,
			BASE_OBJECT_TYPE_ID = VERTEX
		};
};

typedef geometry_traits<HangingVertex>::iterator HangingVertexIterator;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	EDGES

////////////////////////////////////////////////////////////////////////
//	Edge
///	Edges connect two vertices.
/**
 * The most commonly used edge-type.
 *
 * \ingroup GeometricObjects
 */
class Edge : public EdgeBase
{
	friend class Grid;
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Edge*>(pObj) != NULL;}

		Edge()	{}
		Edge(VertexBase* v1, VertexBase* v2)
			{
				m_vertices[0] = v1;
				m_vertices[1] = v2;
			}

		Edge(const EdgeDescriptor& descriptor)
			{
				m_vertices[0] = descriptor.vertex(0);
				m_vertices[1] = descriptor.vertex(1);
			}

		virtual ~Edge()	{}

		virtual GeometricObject* create_empty_instance() const	{return new Edge;}

		virtual int shared_pipe_section() const	{return SPSEDGE_EDGE;}
		virtual int base_object_type_id() const	{return EDGE;}
		virtual int reference_object_id() const {return ROID_EDGE;}

	///	virtual refine. Returns pointers to EdgeBase.
	/**
	 * create 2 new edges, connecting the original edges end-points with vrtNew.
	 * \sa EdgeBase::refine.
	 */
		virtual bool refine(std::vector<EdgeBase*>& vNewEdgesOut,
							VertexBase* newVertex,
							VertexBase** pSubstituteVrts = NULL);

//TODO:	Think about this method. It is not safe!
	///	non virtual refine. Returns pointers to Edge.
	/**
	 * create 2 new edges, connecting the original edges end-points with vrtNew.
	 * \sa EdgeBase::refine.
	 */
		bool refine(std::vector<Edge*>& vNewEdgesOut,
					VertexBase* newVertex,
					VertexBase** pSubstituteVrts = NULL);
};

template <>
class geometry_traits<Edge>
{
	public:
		typedef GenericGeometricObjectIterator<Edge*, EdgeBaseIterator>	iterator;
		typedef EdgeDescriptor	Descriptor;
		typedef EdgeBase		geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSEDGE_EDGE,
			BASE_OBJECT_TYPE_ID = EDGE
		};
};

typedef geometry_traits<Edge>::iterator EdgeIterator;


////////////////////////////////////////////////////////////////////////
//	ConstrainedEdge
///	This edge is a sub-edge of a \sa ConstrainingEdge.
/**
 * Edges of this type appear during hanging-vertex-refinement.
 * Treat them with care, since they are referenced by other objects,
 * i.e. \sa ConstrainingEdge
 *
 * \ingroup GeometricObjects
 */
class ConstrainedEdge : public EdgeBase
{
	friend class Grid;
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<ConstrainedEdge*>(pObj) != NULL;}

		ConstrainedEdge() : m_pConstrainingObject(NULL)	{}
		ConstrainedEdge(VertexBase* v1, VertexBase* v2)
			{
				m_vertices[0] = v1;
				m_vertices[1] = v2;
				m_pConstrainingObject = NULL;
			}

		ConstrainedEdge(const EdgeDescriptor& descriptor)
			{
				m_vertices[0] = descriptor.vertex(0);
				m_vertices[1] = descriptor.vertex(1);
				m_pConstrainingObject = NULL;
			}

		virtual ~ConstrainedEdge()	{}

		virtual GeometricObject* create_empty_instance() const	{return new ConstrainedEdge;}

		virtual int shared_pipe_section() const	{return SPSEDGE_CONSTRAINED_EDGE;}
		virtual int base_object_type_id() const	{return EDGE;}
		virtual int reference_object_id() const {return ROID_EDGE;}

	///	virtual refine. Returns pointers to EdgeBase.
	/**
	 * create 2 new constrained edges, connecting the original edges end-points with vrtNew.
	 * please note that the caller is responsible to resolve any conflicts arising with
	 * existing links of constrained/constraining objects.
	 * \sa EdgeBase::refine.
	 */
		virtual bool refine(std::vector<EdgeBase*>& vNewEdgesOut,
							VertexBase* newVertex,
							VertexBase** pSubstituteVrts = NULL);

//TODO:	Think about this method. It is not safe!
	///	non virtual refine. Returns pointers to ConstrainedEdge.
	/**
	 * create 2 new constrained edges, connecting the original edges end-points with vrtNew.
	 * please note that the caller is responsible to resolve any conflicts arising with
	 * existing links of constrained/constraining objects.
	 * \sa EdgeBase::refine
	 */
		bool refine(std::vector<ConstrainedEdge*>& vNewEdgesOut,
					VertexBase* newVertex,
					VertexBase** pSubstituteVrts = NULL);

		inline void set_constraining_object(GeometricObject* pObj)	{m_pConstrainingObject = pObj;}
		inline GeometricObject* get_constraining_object()	{return m_pConstrainingObject;}

	protected:
		GeometricObject*	m_pConstrainingObject;
};

template <>
class geometry_traits<ConstrainedEdge>
{
	public:
		typedef GenericGeometricObjectIterator<ConstrainedEdge*, EdgeBaseIterator>	iterator;
		typedef EdgeDescriptor	Descriptor;
		typedef EdgeBase		geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSEDGE_CONSTRAINED_EDGE,
			BASE_OBJECT_TYPE_ID = EDGE
		};
};

typedef geometry_traits<ConstrainedEdge>::iterator ConstrainedEdgeIterator;

////////////////////////////////////////////////////////////////////////
//	ConstrainingEdge
///	contains elements of type \sa HangingVertex and \sa ConstrainedEdge
/**
 * Edges of this type appear during hanging-vertex-refinement.
 * Treat with care.
 *
 * \ingroup GeometricObjects
 */
class ConstrainingEdge : public EdgeBase
{
	friend class Grid;
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<ConstrainingEdge*>(pObj) != NULL;}

		ConstrainingEdge()	{}
		ConstrainingEdge(VertexBase* v1, VertexBase* v2)
			{
				m_vertices[0] = v1;
				m_vertices[1] = v2;
			}

		ConstrainingEdge(const EdgeDescriptor& descriptor)
			{
				m_vertices[0] = descriptor.vertex(0);
				m_vertices[1] = descriptor.vertex(1);
			}

		virtual ~ConstrainingEdge()	{}

		virtual GeometricObject* create_empty_instance() const	{return new ConstrainingEdge;}

		virtual int shared_pipe_section() const	{return SPSEDGE_CONSTRAINING_EDGE;}
		virtual int base_object_type_id() const	{return EDGE;}
		virtual int reference_object_id() const {return ROID_EDGE;}

	///	virtual refine. Returns pointers to EdgeBase.
	/**
	 * create 2 new constraining edges, connecting the original edges end-points with vrtNew.
	 * The refine has no effect on constrained objects. The user has to manually copy them.
	 * \sa EdgeBase::refine.
	 */
		virtual bool refine(std::vector<EdgeBase*>& vNewEdgesOut,
							VertexBase* newVertex,
							VertexBase** pSubstituteVrts = NULL);

//TODO:	Think about this method. It is not safe!
	///	non virtual refine. Returns pointers to ConstrainingEdge.
	/**
	 * create 2 new constraining edges, connecting the original edges end-points with vrtNew.
	 * The refine has no effect on constrained objects. The user has to manually copy them.
	 * \sa EdgeBase::refine.
	 */
		bool refine(std::vector<ConstrainingEdge*>& vNewEdgesOut,
						VertexBase* newVertex,
						VertexBase** pSubstituteVrts = NULL);


		inline void add_constrained_object(VertexBase* pObj)
			{
				if(!is_constrained_object(pObj))
					m_lstConstrainedVertices.push_back(pObj);
			}

		inline void add_constrained_object(EdgeBase* pObj)
			{
				if(!is_constrained_object(pObj))
					m_lstConstrainedEdges.push_back(pObj);
			}

		inline VertexBaseIterator constrained_vertices_begin()		{return m_lstConstrainedVertices.begin();}
		inline VertexBaseIterator constrained_vertices_end()		{return m_lstConstrainedVertices.end();}
		inline EdgeBaseIterator constrained_edges_begin()			{return m_lstConstrainedEdges.begin();}
		inline EdgeBaseIterator constrained_edges_end()				{return m_lstConstrainedEdges.end();}

		inline bool is_constrained_object(VertexBase* vrt)
			{
				std::list<VertexBase*>::iterator iter = find(m_lstConstrainedVertices.begin(),
																m_lstConstrainedVertices.end(), vrt);
				return iter != m_lstConstrainedVertices.end();
			}

		inline bool is_constrained_object(EdgeBase* edge)
			{
				std::list<EdgeBase*>::iterator iter = find(m_lstConstrainedEdges.begin(),
																m_lstConstrainedEdges.end(), edge);
				return iter != m_lstConstrainedEdges.end();
			}

		inline void unconstrain_vertex(const VertexBase* vrt)
			{
				std::list<VertexBase*>::iterator iter = find(m_lstConstrainedVertices.begin(),
																m_lstConstrainedVertices.end(), vrt);
				if(iter != m_lstConstrainedVertices.end())
					m_lstConstrainedVertices.erase(iter);
			}

		inline void unconstrain_edge(const EdgeBase* edge)
			{
				std::list<EdgeBase*>::iterator iter = find(m_lstConstrainedEdges.begin(),
																m_lstConstrainedEdges.end(), edge);
				if(iter != m_lstConstrainedEdges.end())
					m_lstConstrainedEdges.erase(iter);
			}

		inline void clear_constrained_vertices()	{m_lstConstrainedVertices.clear();}
		inline void clear_constrained_edges()		{m_lstConstrainedEdges.clear();}
		inline void clear_constrained_objects()
			{
				clear_constrained_vertices();
				clear_constrained_edges();
			}

	///	linear time
		inline uint num_constrained_vertices()	{return m_lstConstrainedVertices.size();}
	///	linear time
		inline uint num_constrained_edges()		{return m_lstConstrainedEdges.size();}

	protected:
		std::list<VertexBase*>	m_lstConstrainedVertices;
		std::list<EdgeBase*>	m_lstConstrainedEdges;
};

template <>
class geometry_traits<ConstrainingEdge>
{
	public:
		typedef GenericGeometricObjectIterator<ConstrainingEdge*, EdgeBaseIterator>	iterator;
		typedef EdgeDescriptor	Descriptor;
		typedef EdgeBase		geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSEDGE_CONSTRAINING_EDGE,
			BASE_OBJECT_TYPE_ID = EDGE
		};
};

typedef geometry_traits<ConstrainingEdge>::iterator ConstrainingEdgeIterator;



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	FACES

////////////////////////////////////////////////////////////////////////
//	CustomFace
///	this class can be used to create a new Face-Type.
template <uint numVrts, uint sharedPipeSectionIndex>
class CustomFace : public Face
{
	public:
		CustomFace()	{Face::set_num_vertices(numVrts);}
		virtual int shared_pipe_section() const	{return sharedPipeSectionIndex;}
		virtual int base_object_type_id() const	{return FACE;}

protected:
	virtual EdgeBase* create_edge(int index)
		{
			return new Edge(m_vertices[index], m_vertices[(index+1) % numVrts]);
		}
};

////////////////////////////////////////////////////////////////////////
//	TriangleDescriptor
///	only used to initialize a triangle. for all other tasks you should use FaceDescriptor.
class TriangleDescriptor
{
	public:
		TriangleDescriptor()	{}
		TriangleDescriptor(const TriangleDescriptor& td);
		TriangleDescriptor(VertexBase* v1, VertexBase* v2, VertexBase* v3);

		inline uint num_vertices() const					{return 3;}
		inline void set_vertex(uint index, VertexBase* v)	{m_vertex[index] = v;}
		inline VertexBase* vertex(uint index) const			{return m_vertex[index];}

	protected:
		VertexBase*	m_vertex[3];
};

////////////////////////////////////////////////////////////////////////
//	CustomTriangle
///	Concrete types share this base-type. It is not intended for direct use.
/**
 * BaseClass has to be derived from Face (or simply should be Face).
 * The ConcreteTriangleType is used in methods like refine, etc. as the type
 * of newly created objects.
 */
template <class ConcreteTriangleType, class BaseClass>
class CustomTriangle : public BaseClass
{
	public:
		CustomTriangle()	{BaseClass::set_num_vertices(3);}
		CustomTriangle(const TriangleDescriptor& td);
		CustomTriangle(VertexBase* v1, VertexBase* v2, VertexBase* v3);

		virtual GeometricObject* create_empty_instance() const	{return new ConcreteTriangleType;}
		virtual int reference_object_id() const {return ROID_TRIANGLE;}

	///	Refines a Triangle by inserting new vertices. \sa Face::refine.
		virtual bool refine(std::vector<Face*>& vNewFacesOut,
							VertexBase** newFaceVertexOut,
							VertexBase** newEdgeVertices,
							VertexBase* newFaceVertex = NULL,
							VertexBase** pSubstituteVertices = NULL);

	///	Performs a regular refine on the triangle. \sa Face::refine_regular.
		virtual bool refine_regular(std::vector<Face*>& vNewFacesOut,
										VertexBase** newFaceVertexOut,
										std::vector<VertexBase*>& vNewEdgeVertices,
										VertexBase* newFaceVertex,
										const VertexBase& prototypeVertex,
										std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edge(std::vector<Face*>& vNewFacesOut,
								int edgeIndex, VertexBase* newVertex,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edges(std::vector<Face*>& vNewFacesOut,
								std::vector<VertexBase*>& vNewEdgeVertices,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

//	BEGIN Depreciated
		virtual void create_faces_by_edge_split(int splitEdgeIndex,
							VertexBase* newVertex,
							std::vector<Face*>& vNewFacesOut,
							std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual int base_object_type_id() const	{return FACE;}
};

////////////////////////////////////////////////////////////////////////
//	Triangle
///	the most simple form of a face
/**
 *
 * \ingroup GeometricObjects
 */
class Triangle : public CustomTriangle<Triangle, Face>
{
	typedef CustomTriangle<Triangle, Face> BaseClass;
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Triangle*>(pObj) != NULL;}

		Triangle() : BaseClass()	{}
		Triangle(const TriangleDescriptor& td) : BaseClass(td)	{}
		Triangle(VertexBase* v1, VertexBase* v2, VertexBase* v3) : BaseClass(v1, v2, v3)	{}

		virtual int shared_pipe_section() const	{return SPSFACE_TRIANGLE;}

	protected:
		virtual EdgeBase* create_edge(int index)
			{
				return new Edge(m_vertices[index], m_vertices[(index+1) % 3]);
			}
};

template <>
class geometry_traits<Triangle>
{
	public:
		typedef GenericGeometricObjectIterator<Triangle*, FaceIterator>	iterator;
		typedef TriangleDescriptor Descriptor;	///< Faces can't be created directly
		typedef Face	geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSFACE_TRIANGLE,
			BASE_OBJECT_TYPE_ID = FACE
		};
};

typedef geometry_traits<Triangle>::iterator	TriangleIterator;

////////////////////////////////////////////////////////////////////////
//	ConstrainingFace
///	This class is used to store constrained geometric objects.
/**
 * Please note, that the user is has to link and unlink constrained
 * objects manually.
 */
class ConstrainingFace : public Face
{

	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<ConstrainingFace*>(pObj) != NULL;}

		virtual ~ConstrainingFace()	{}

		inline void add_constrained_object(VertexBase* pObj)
			{
				if(!is_constrained_object(pObj))
					m_lstConstrainedVertices.push_back(pObj);
			}

		inline void add_constrained_object(EdgeBase* pObj)
			{
				if(!is_constrained_object(pObj))
					m_lstConstrainedEdges.push_back(pObj);
			}

		inline void add_constrained_object(Face* pObj)
			{
				if(!is_constrained_object(pObj))
					m_lstConstrainedFaces.push_back(pObj);
			}

		inline VertexBaseIterator constrained_vertices_begin()		{return m_lstConstrainedVertices.begin();}
		inline VertexBaseIterator constrained_vertices_end()		{return m_lstConstrainedVertices.end();}
		inline EdgeBaseIterator constrained_edges_begin()			{return m_lstConstrainedEdges.begin();}
		inline EdgeBaseIterator constrained_edges_end()				{return m_lstConstrainedEdges.end();}
		inline FaceIterator constrained_faces_begin()				{return m_lstConstrainedFaces.begin();}
		inline FaceIterator constrained_faces_end()					{return m_lstConstrainedFaces.end();}

		inline bool is_constrained_object(VertexBase* vrt)
			{
				std::list<VertexBase*>::iterator iter = find(m_lstConstrainedVertices.begin(),
																m_lstConstrainedVertices.end(), vrt);
				return iter != m_lstConstrainedVertices.end();
			}

		inline bool is_constrained_object(EdgeBase* edge)
			{
				std::list<EdgeBase*>::iterator iter = find(m_lstConstrainedEdges.begin(),
																m_lstConstrainedEdges.end(), edge);
				return iter != m_lstConstrainedEdges.end();
			}

		inline bool is_constrained_object(Face* face)
			{
				std::list<Face*>::iterator iter = find(m_lstConstrainedFaces.begin(),
																m_lstConstrainedFaces.end(), face);
				return iter != m_lstConstrainedFaces.end();
			}

		inline void unconstrain_object(const VertexBase* vrt)
			{
				std::list<VertexBase*>::iterator iter = find(m_lstConstrainedVertices.begin(),
																m_lstConstrainedVertices.end(), vrt);
				if(iter != m_lstConstrainedVertices.end())
					m_lstConstrainedVertices.erase(iter);
			}

		inline void unconstrain_object(const EdgeBase* edge)
			{
				std::list<EdgeBase*>::iterator iter = find(m_lstConstrainedEdges.begin(),
																m_lstConstrainedEdges.end(), edge);
				if(iter != m_lstConstrainedEdges.end())
					m_lstConstrainedEdges.erase(iter);
			}

		inline void unconstrain_object(const Face* face)
			{
				std::list<Face*>::iterator iter = find(m_lstConstrainedFaces.begin(),
																m_lstConstrainedFaces.end(), face);
				if(iter != m_lstConstrainedFaces.end())
					m_lstConstrainedFaces.erase(iter);
			}

		inline void clear_constrained_vertices()	{m_lstConstrainedVertices.clear();}
		inline void clear_constrained_edges()		{m_lstConstrainedEdges.clear();}
		inline void clear_constrained_faces()		{m_lstConstrainedFaces.clear();}
		inline void clear_constrained_objects()
			{
				clear_constrained_vertices();
				clear_constrained_edges();
				clear_constrained_faces();
			}

	///	linear time
		inline uint num_constrained_vertices()	{return m_lstConstrainedVertices.size();}
	///	linear time
		inline uint num_constrained_edges()		{return m_lstConstrainedEdges.size();}
	///	linear time
		inline uint num_constrained_faces()		{return m_lstConstrainedFaces.size();}

	protected:
		std::list<VertexBase*>	m_lstConstrainedVertices;
		std::list<EdgeBase*>	m_lstConstrainedEdges;
		std::list<Face*>		m_lstConstrainedFaces;
};

////////////////////////////////////////////////////////////////////////
//	ConstrainedFace
///	This class stores the constraining object.
/**
 * Please note, that the user is has to link and unlink constraining
 * objects manually.
 */
class ConstrainedFace : public Face
{
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<ConstrainedFace*>(pObj) != NULL;}

		virtual ~ConstrainedFace()	{}

		inline void set_constraining_object(GeometricObject* pObj)	{m_pConstrainingObject = pObj;}
		inline GeometricObject* get_constraining_object()			{return m_pConstrainingObject;}

	protected:
		GeometricObject*	m_pConstrainingObject;
};

////////////////////////////////////////////////////////////////////////
//	ConstrainedTriangle
///	a triangle constrained by another object.
/**
 * \ingroup GeometricObjects
 */
class ConstrainedTriangle : public CustomTriangle<ConstrainedTriangle, ConstrainedFace>
{
	typedef CustomTriangle<ConstrainedTriangle, ConstrainedFace> BaseTriangle;

	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<ConstrainedTriangle*>(pObj) != NULL;}

		ConstrainedTriangle() : BaseTriangle()	{}
		ConstrainedTriangle(const TriangleDescriptor& td) : BaseTriangle(td)	{}
		ConstrainedTriangle(VertexBase* v1, VertexBase* v2, VertexBase* v3) : BaseTriangle(v1, v2, v3)	{}

		virtual int shared_pipe_section() const	{return SPSFACE_CONSTRAINED_TRIANGLE;}

	protected:
		virtual EdgeBase* create_edge(int index)
			{
				return new ConstrainedEdge(m_vertices[index], m_vertices[(index+1) % 3]);
			}
};

template <>
class geometry_traits<ConstrainedTriangle>
{
	public:
		typedef GenericGeometricObjectIterator<ConstrainedTriangle*, TriangleIterator>	iterator;
		typedef TriangleDescriptor Descriptor;	///< Faces can't be created directly
		typedef Face	geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSFACE_CONSTRAINED_TRIANGLE,
			BASE_OBJECT_TYPE_ID = FACE
		};
};

typedef geometry_traits<ConstrainedTriangle>::iterator	ConstrainedTriangleIterator;

////////////////////////////////////////////////////////////////////////
//	ConstrainedTriangle
///	a triangle constraining other objects.
/**
 * \ingroup GeometricObjects
 */
class ConstrainingTriangle : public CustomTriangle<ConstrainingTriangle, ConstrainingFace>
{
	typedef CustomTriangle<ConstrainingTriangle, ConstrainingFace> BaseTriangle;

	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<ConstrainingTriangle*>(pObj) != NULL;}

		ConstrainingTriangle() : BaseTriangle()	{}
		ConstrainingTriangle(const TriangleDescriptor& td) : BaseTriangle(td)	{}
		ConstrainingTriangle(VertexBase* v1, VertexBase* v2, VertexBase* v3) : BaseTriangle(v1, v2, v3)	{}

		virtual int shared_pipe_section() const	{return SPSFACE_CONSTRAINING_TRIANGLE;}

	protected:
		virtual EdgeBase* create_edge(int index)
			{
				return new Edge(m_vertices[index], m_vertices[(index+1) % 3]);
			}
};

template <>
class geometry_traits<ConstrainingTriangle>
{
	public:
		typedef GenericGeometricObjectIterator<ConstrainingTriangle*, TriangleIterator>	iterator;
		typedef TriangleDescriptor Descriptor;	///< Faces can't be created directly
		typedef Face	geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSFACE_CONSTRAINING_TRIANGLE,
			BASE_OBJECT_TYPE_ID = FACE
		};
};

typedef geometry_traits<ConstrainingTriangle>::iterator	ConstrainingTriangleIterator;



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	QUADRILATERAL

////////////////////////////////////////////////////////////////////////
//	QuadrilateralDescriptor
///	only used to initialize a quadrilateral. for all other tasks you should use FaceDescriptor.
class QuadrilateralDescriptor
{
	public:
		QuadrilateralDescriptor()	{}
		QuadrilateralDescriptor(const QuadrilateralDescriptor& qd);
		QuadrilateralDescriptor(VertexBase* v1, VertexBase* v2, VertexBase* v3, VertexBase* v4);

		inline uint num_vertices() const					{return 4;}
		inline void set_vertex(uint index, VertexBase* v)	{m_vertex[index] = v;}
		inline VertexBase* vertex(uint index) const			{return m_vertex[index];}

	protected:
		VertexBase*	m_vertex[4];
};

////////////////////////////////////////////////////////////////////////
//	Quadrilateral
///	a face with four points.
/**
 * \ingroup GeometricObjects
 */
class Quadrilateral : public CustomFace<4, SPSFACE_QUADRILATERAL>
{
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Quadrilateral*>(pObj) != NULL;}

		Quadrilateral() : CustomFace<4, SPSFACE_QUADRILATERAL>()	{}
		Quadrilateral(const QuadrilateralDescriptor& td);
		Quadrilateral(VertexBase* v1, VertexBase* v2, VertexBase* v3, VertexBase* v4);

		virtual GeometricObject* create_empty_instance() const	{return new Quadrilateral;}
		virtual int reference_object_id() const {return ROID_QUADRILATERAL;}

	///	Refines a Quad by inserting new vertices. \sa Face::refine.
		virtual bool refine(std::vector<Face*>& vNewFacesOut,
							VertexBase** newFaceVertexOut,
							VertexBase** newEdgeVertices,
							VertexBase* newFaceVertex = NULL,
							VertexBase** pSubstituteVertices = NULL);

	///	Performs regular refine on a Quad by inserting new vertices. \sa Face::refine_regular.
		virtual bool refine_regular(std::vector<Face*>& vNewFacesOut,
									VertexBase** newFaceVertexOut,
									std::vector<VertexBase*>& vNewEdgeVertices,
									VertexBase* newFaceVertex,
									const VertexBase& prototypeVertex,
									std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edge(std::vector<Face*>& vNewFacesOut,
								int edgeIndex, VertexBase* newVertex,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edges(std::vector<Face*>& vNewFacesOut,
								std::vector<VertexBase*>& vNewEdgeVertices,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

//	BEGIN Depreciated
		virtual void create_faces_by_edge_split(int splitEdgeIndex,
							VertexBase* newVertex,
							std::vector<Face*>& vNewFacesOut,
							std::vector<VertexBase*>* pvSubstituteVertices = NULL);
//	END Depreciated
};

template <>
class geometry_traits<Quadrilateral>
{
	public:
		typedef GenericGeometricObjectIterator<Quadrilateral*, FaceIterator>	iterator;
		typedef QuadrilateralDescriptor Descriptor;	///< Faces can't be created directly
		typedef Face	geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSFACE_QUADRILATERAL,
			BASE_OBJECT_TYPE_ID = FACE
		};
};

typedef geometry_traits<Quadrilateral>::iterator	QuadrilateralIterator;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	VOLUMES

////////////////////////////////////////////////////////////////////////
//	TetrahedronDescriptor
///	only used to initialize a tetrahedron. for all other tasks you should use VolumeDescripor.
/**
 * please be sure to pass the vertices in the correct order:
 * v1, v2, v3: bottom-vertices in counterclockwise order (if viewed from the top).
 * v4: top
 */
class TetrahedronDescriptor
{
	public:
		TetrahedronDescriptor()	{}
		TetrahedronDescriptor(const TetrahedronDescriptor& td);
		TetrahedronDescriptor(VertexBase* v1, VertexBase* v2, VertexBase* v3, VertexBase* v4);

		inline uint num_vertices() const	{return 4;}
		inline VertexBase* vertex(uint index) const	{return m_vertex[index];}

	protected:
		VertexBase*	m_vertex[4];
};

////////////////////////////////////////////////////////////////////////
//	Tetrahedron
///	the most simple volume-element.
/**
 * order of vertices should be the same as described in \sa TetrahedronDescriptor
 *
 * \ingroup GeometricObjects
 */
class Tetrahedron : public Volume
{
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Tetrahedron*>(pObj) != NULL;}

		Tetrahedron()	{m_vertices.resize(4);}
		Tetrahedron(const TetrahedronDescriptor& td);
		Tetrahedron(VertexBase* v1, VertexBase* v2, VertexBase* v3, VertexBase* v4);

		virtual GeometricObject* create_empty_instance() const	{return new Tetrahedron;}

		virtual EdgeDescriptor edge(int index) const;
		virtual void edge(int index, EdgeDescriptor& edOut) const;
		virtual uint num_edges() const;

		virtual FaceDescriptor face(int index) const;
		virtual void face(int index, FaceDescriptor& fdOut) const;
		virtual uint num_faces() const;

	///	see Volume::refine for a detailed description.
		virtual bool refine(std::vector<Volume*>& vNewVolumesOut,
							VertexBase** ppNewVertexOut,
							std::vector<VertexBase*>& vNewEdgeVertices,
							std::vector<VertexBase*>& vNewFaceVertices,
							VertexBase* newVolumeVertex,
							const VertexBase& prototypeVertex,
							std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edge(std::vector<Volume*>& vNewVolumesOut,
								int edgeIndex, VertexBase* newVertex,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual int shared_pipe_section() const	{return SPSVOL_TETRAHEDRON;}
		virtual int base_object_type_id() const	{return VOLUME;}
		virtual int reference_object_id() const {return ROID_TETRAHEDRON;}

	protected:
		virtual EdgeBase* create_edge(int index);	///< create the edge with index i and return it.
		virtual Face* create_face(int index);		///< create the face with index i and return it.
};

template <>
class geometry_traits<Tetrahedron>
{
	public:
		typedef GenericGeometricObjectIterator<Tetrahedron*, VolumeIterator>	iterator;
		typedef TetrahedronDescriptor Descriptor;
		typedef Volume 		geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSVOL_TETRAHEDRON,
			BASE_OBJECT_TYPE_ID = VOLUME
		};
};

typedef geometry_traits<Tetrahedron>::iterator	TetrahedronIterator;


////////////////////////////////////////////////////////////////////////
//	HexahedronDescriptor
///	only used to initialize a hexahedron. for all other tasks you should use VolumeDescripor.
/**
 * please be sure to pass the vertices in the correct order:
 * v1, v2, v3, v4: bottom-vertices in counterclockwise order (if viewed from the top).
 * v5, v6, v7, v8: top-vertices in counterclockwise order (if viewed from the top).
 */
class HexahedronDescriptor
{
	public:
		HexahedronDescriptor()	{}
		HexahedronDescriptor(const HexahedronDescriptor& td);
		HexahedronDescriptor(VertexBase* v1, VertexBase* v2, VertexBase* v3, VertexBase* v4,
							VertexBase* v5, VertexBase* v6, VertexBase* v7, VertexBase* v8);

		inline uint num_vertices() const	{return 8;}
		inline VertexBase* vertex(uint index) const	{return m_vertex[index];}

	protected:
		VertexBase*	m_vertex[8];
};

////////////////////////////////////////////////////////////////////////
//	Hexahedron
///	A volume element with 6 quadrilateral sides.
/**
 * Order of vertices should be the same as described in \sa HexahedronDescriptor
 *
 * \ingroup GeometricObjects
 */
class Hexahedron : public Volume
{
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Hexahedron*>(pObj) != NULL;}

		Hexahedron()	{m_vertices.resize(8);}
		Hexahedron(const HexahedronDescriptor& td);
		Hexahedron(VertexBase* v1, VertexBase* v2, VertexBase* v3, VertexBase* v4,
					VertexBase* v5, VertexBase* v6, VertexBase* v7, VertexBase* v8);

		virtual GeometricObject* create_empty_instance() const	{return new Hexahedron;}

		virtual EdgeDescriptor edge(int index) const;
		virtual void edge(int index, EdgeDescriptor& edOut) const;
		virtual uint num_edges() const;

		virtual FaceDescriptor face(int index) const;
		virtual void face(int index, FaceDescriptor& fdOut) const;
		virtual uint num_faces() const;

	///	see Volume::refine for a detailed description.
		virtual bool refine(std::vector<Volume*>& vNewVolumesOut,
							VertexBase** ppNewVertexOut,
							std::vector<VertexBase*>& vNewEdgeVertices,
							std::vector<VertexBase*>& vNewFaceVertices,
							VertexBase* newVolumeVertex,
							const VertexBase& prototypeVertex,
							std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edge(std::vector<Volume*>& vNewVolumesOut,
								int edgeIndex, VertexBase* newVertex,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual int shared_pipe_section() const	{return SPSVOL_HEXAHEDRON;}
		virtual int base_object_type_id() const	{return VOLUME;}
		virtual int reference_object_id() const {return ROID_HEXAHEDRON;}

	protected:
		virtual EdgeBase* create_edge(int index);	///< create the edge with index i and return it.
		virtual Face* create_face(int index);		///< create the face with index i and return it.
};

template <>
class geometry_traits<Hexahedron>
{
	public:
		typedef GenericGeometricObjectIterator<Hexahedron*, VolumeIterator>	iterator;
		typedef HexahedronDescriptor Descriptor;
		typedef Volume 		geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSVOL_HEXAHEDRON,
			BASE_OBJECT_TYPE_ID = VOLUME
		};
};

typedef geometry_traits<Hexahedron>::iterator	HexahedronIterator;


////////////////////////////////////////////////////////////////////////
//	PrismDescriptor
///	only used to initialize a prism. for all other tasks you should use VolumeDescripor.
/**
 * please be sure to pass the vertices in the correct order:
 * v1, v2, v3: bottom-vertices in counterclockwise order (if viewed from the top).
 * v4, v5, v6: top-vertices in counterclockwise order (if viewed from the top).
 */
class PrismDescriptor
{
	public:
		PrismDescriptor()	{}
		PrismDescriptor(const PrismDescriptor& td);
		PrismDescriptor(VertexBase* v1, VertexBase* v2, VertexBase* v3,
						VertexBase* v4, VertexBase* v5, VertexBase* v6);

		inline uint num_vertices() const	{return 6;}
		inline VertexBase* vertex(uint index) const	{return m_vertex[index];}

	protected:
		VertexBase*	m_vertex[6];
};

////////////////////////////////////////////////////////////////////////
//	Prism
///	A volume element with 2 triangle and 3 quadrilateral sides.
/**
 * order of vertices should be the same as described in \sa PrismDescriptor
 *
 * \ingroup GeometricObjects
 */
class Prism : public Volume
{
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Prism*>(pObj) != NULL;}

		Prism()	{m_vertices.resize(6);}
		Prism(const PrismDescriptor& td);
		Prism(VertexBase* v1, VertexBase* v2, VertexBase* v3,
				VertexBase* v4, VertexBase* v5, VertexBase* v6);

		virtual GeometricObject* create_empty_instance() const	{return new Prism;}

		virtual EdgeDescriptor edge(int index) const;
		virtual void edge(int index, EdgeDescriptor& edOut) const;
		virtual uint num_edges() const;

		virtual FaceDescriptor face(int index) const;
		virtual void face(int index, FaceDescriptor& fdOut) const;
		virtual uint num_faces() const;

	///	see Volume::refine for a detailed description.
		virtual bool refine(std::vector<Volume*>& vNewVolumesOut,
							VertexBase** ppNewVertexOut,
							std::vector<VertexBase*>& vNewEdgeVertices,
							std::vector<VertexBase*>& vNewFaceVertices,
							VertexBase* newVolumeVertex,
							const VertexBase& prototypeVertex,
							std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edge(std::vector<Volume*>& vNewVolumesOut,
								int edgeIndex, VertexBase* newVertex,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual int shared_pipe_section() const	{return SPSVOL_PRISM;}
		virtual int base_object_type_id() const	{return VOLUME;}
		virtual int reference_object_id() const {return ROID_PRISM;}

	protected:
		virtual EdgeBase* create_edge(int index);	///< create the edge with index i and return it.
		virtual Face* create_face(int index);		///< create the face with index i and return it.
};

template <>
class geometry_traits<Prism>
{
	public:
		typedef GenericGeometricObjectIterator<Prism*, VolumeIterator>	iterator;
		typedef PrismDescriptor Descriptor;
		typedef Volume 		geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSVOL_PRISM,
			BASE_OBJECT_TYPE_ID = VOLUME
		};
};

typedef geometry_traits<Prism>::iterator	PrismIterator;


////////////////////////////////////////////////////////////////////////
//	PyramidDescriptor
///	only used to initialize a pyramids. for all other tasks you should use VolumeDescripor.
/**
 * please be sure to pass the vertices in the correct order:
 * v1, v2, v3, v4: bottom-vertices in counterclockwise order (if viewed from the top).
 * v5: top-vertex.
 */
class PyramidDescriptor
{
	public:
		PyramidDescriptor()	{}
		PyramidDescriptor(const PyramidDescriptor& td);
		PyramidDescriptor(VertexBase* v1, VertexBase* v2, VertexBase* v3,
						VertexBase* v4, VertexBase* v5);

		inline uint num_vertices() const	{return 5;}
		inline VertexBase* vertex(uint index) const	{return m_vertex[index];}

	protected:
		VertexBase*	m_vertex[5];
};

////////////////////////////////////////////////////////////////////////
//	Pyramid
///	A volume element with 4 triangle and 1 quadrilateral sides.
/**
 * order of vertices should be the same as described in \sa PyramidDescriptor
 *
 * \ingroup GeometricObjects
 */
class Pyramid : public Volume
{
	public:
		inline static bool type_match(GeometricObject* pObj)	{return dynamic_cast<Pyramid*>(pObj) != NULL;}

		Pyramid()	{m_vertices.resize(5);}
		Pyramid(const PyramidDescriptor& td);
		Pyramid(VertexBase* v1, VertexBase* v2, VertexBase* v3,
				VertexBase* v4, VertexBase* v5);

		virtual GeometricObject* create_empty_instance() const	{return new Pyramid;}

		virtual EdgeDescriptor edge(int index) const;
		virtual void edge(int index, EdgeDescriptor& edOut) const;
		virtual uint num_edges() const;

		virtual FaceDescriptor face(int index) const;
		virtual void face(int index, FaceDescriptor& fdOut) const;
		virtual uint num_faces() const;

	///	see Volume::refine for a detailed description.
		virtual bool refine(std::vector<Volume*>& vNewVolumesOut,
							VertexBase** ppNewVertexOut,
							std::vector<VertexBase*>& vNewEdgeVertices,
							std::vector<VertexBase*>& vNewFaceVertices,
							VertexBase* newVolumeVertex,
							const VertexBase& prototypeVertex,
							std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual bool collapse_edge(std::vector<Volume*>& vNewVolumesOut,
								int edgeIndex, VertexBase* newVertex,
								std::vector<VertexBase*>* pvSubstituteVertices = NULL);

		virtual int shared_pipe_section() const	{return SPSVOL_PYRAMID;}
		virtual int base_object_type_id() const	{return VOLUME;}
		virtual int reference_object_id() const {return ROID_PYRAMID;}

	protected:
		virtual EdgeBase* create_edge(int index);	///< create the edge with index i and return it.
		virtual Face* create_face(int index);		///< create the face with index i and return it.
};

template <>
class geometry_traits<Pyramid>
{
	public:
		typedef GenericGeometricObjectIterator<Pyramid*, VolumeIterator>	iterator;
		typedef PyramidDescriptor Descriptor;
		typedef Volume 		geometric_base_object;

		enum
		{
			SHARED_PIPE_SECTION = SPSVOL_PYRAMID,
			BASE_OBJECT_TYPE_ID = VOLUME
		};
};

typedef geometry_traits<Pyramid>::iterator	PyramidIterator;

}//	end of namespace

#endif
