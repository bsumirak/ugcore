// created by Sebastian Reiter
// s.b.reiter@googlemail.com
// 11.01.2011 (m,d,y)
 
#include <vector>
#include "hanging_node_refiner_grid.h"
#include "lib_grid/algorithms/geom_obj_util/geom_obj_util.h"

using namespace std;

namespace ug{

HangingNodeRefiner_Grid::
HangingNodeRefiner_Grid(IRefinementCallback* refCallback) :
	HangingNodeRefinerBase(refCallback),
	m_pGrid(NULL),
	m_aVertex(false)
{

}

HangingNodeRefiner_Grid::
HangingNodeRefiner_Grid(Grid& grid,
						IRefinementCallback* refCallback) :
	HangingNodeRefinerBase(refCallback),
	m_pGrid(NULL),
	m_aVertex(false)
{
	set_grid(&grid);
}

HangingNodeRefiner_Grid::
~HangingNodeRefiner_Grid()
{
	set_grid(NULL);
}

void HangingNodeRefiner_Grid::
grid_to_be_destroyed(Grid* grid)
{
	set_grid(NULL);
}

void HangingNodeRefiner_Grid::
assign_grid(Grid& grid)
{
	set_grid(&grid);
}

void HangingNodeRefiner_Grid::
set_grid(Grid* grid)
{
//	call base class implementation
	HangingNodeRefinerBase::set_grid(grid);

//	clear own attachments
	if(m_pGrid)
	{
		m_pGrid->detach_from_edges(m_aVertex);
		m_pGrid->detach_from_faces(m_aVertex);
		m_pGrid = NULL;
	}

//	attach new attachments
	if(grid){
		grid->attach_to_edges_dv(m_aVertex, NULL, false);
		grid->attach_to_faces_dv(m_aVertex, NULL, false);

		m_aaVertexEDGE.access(*grid, m_aVertex);
		m_aaVertexFACE.access(*grid, m_aVertex);

		m_pGrid = grid;
	}
}

void HangingNodeRefiner_Grid::
post_refine()
{
	if(!m_pGrid)
		throw(UGError("HangingNodeRefiner_Grid::post_refine: No grid assigned."));

//	erase unused elements
	UG_DLOG(LIB_GRID, 1, "  erasing elements.\n");

	Grid& grid = *m_pGrid;
	vector<Face*>	 	vFaces;
	vector<Volume*>		vVols;

//	erase faces that are no longer needed.
	if(grid.num_volumes() > 0)
	{
		FaceIterator iter = m_selMarkedElements.begin<Face>();
		while(iter != m_selMarkedElements.end<Face>())
		{
			Face* f = *iter;
			++iter;
			CollectVolumes(vVols, grid, f);
			if(vVols.size() == 0)
			{
			//	erase
				grid.erase(f);
			}
		}
	}

//	erase edges that are no longer needed.
	if(grid.num_faces() > 0)
	{
		EdgeBaseIterator iter = m_selMarkedElements.begin<EdgeBase>();
		while(iter != m_selMarkedElements.end<EdgeBase>())
		{
			EdgeBase* e = *iter;
			++iter;
			CollectFaces(vFaces, grid, e);
			if(vFaces.size() == 0)
			{
			//	erase
				grid.erase(e);
			}
		}
	}
}

void HangingNodeRefiner_Grid::
refine_constraining_edge(ConstrainingEdge* e)
{
//	call original implementation
	HangingNodeRefinerBase::refine_constraining_edge(e);

//	if there are no faces, the edge can be erased
	if(m_pGrid->num_faces() == 0)
		m_pGrid->erase(e);
}

void HangingNodeRefiner_Grid::
refine_edge_with_normal_vertex(EdgeBase* e, VertexBase** newCornerVrts)
{
//	call original implementation
	HangingNodeRefinerBase::refine_edge_with_normal_vertex(e, newCornerVrts);

//	if there are no faces, the edge can be erased
	if(m_pGrid->num_faces() == 0)
		m_pGrid->erase(e);
}

void HangingNodeRefiner_Grid::
refine_face_with_normal_vertex(Face* f, VertexBase** newCornerVrts)
{
//	call original implementation
	HangingNodeRefinerBase::refine_face_with_normal_vertex(f, newCornerVrts);

//	if there are no volumes, the face can be erased
	if(m_pGrid->num_volumes() == 0)
		m_pGrid->erase(f);
}

void HangingNodeRefiner_Grid::
refine_constraining_face(ConstrainingFace* f)
{
//	call original implementation
	HangingNodeRefinerBase::refine_constraining_face(f);

//	if there are no volumes, the face can be erased
	if(m_pGrid->num_volumes() == 0)
		m_pGrid->erase(f);
}

void HangingNodeRefiner_Grid::
refine_volume_with_normal_vertex(Volume* v, VertexBase** newCornerVrts)
{
//	call original implementation
	HangingNodeRefinerBase::refine_volume_with_normal_vertex(v, newCornerVrts);

//	erase the volume
	m_pGrid->erase(v);
}

VertexBase* HangingNodeRefiner_Grid::
get_center_vertex(EdgeBase* e)
{
	return m_aaVertexEDGE[e];
}


void HangingNodeRefiner_Grid::
set_center_vertex(EdgeBase* e, VertexBase* v)
{
	m_aaVertexEDGE[e] = v;
}


VertexBase* HangingNodeRefiner_Grid::
get_center_vertex(Face* f)
{
	return m_aaVertexFACE[f];
}


void HangingNodeRefiner_Grid::
set_center_vertex(Face* f, VertexBase* v)
{
	m_aaVertexFACE[f] = v;
}

}// end of namespace
