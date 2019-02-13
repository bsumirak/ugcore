/*
 * Copyright (c) 2016:  G-CSC, Goethe University Frankfurt
 * Author: Markus Breit
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
 *
 *  Created on: 2016-12-19
 */

#include "neurite_projector.h"
#include "common/error.h"
#include "lib_grid/global_attachments.h"
#include "lib_grid/algorithms/debug_util.h"  // ElementDebugInfo

#include <boost/lexical_cast.hpp>
#include <cmath>

namespace ug {

static number VecProd(const vector3& a, const vector3& b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static number VecNormSquared(const vector3& a)
{
	return a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
}

static number VecNorm(const vector3& a)
{
	return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}


NeuriteProjector::NeuriteProjector()
//: m_quadOrder(80)
{
	typedef Attachment<NeuriteProjector::SurfaceParams> NPSurfParam;
	if (!GlobalAttachments::is_declared("npSurfParams"))
		GlobalAttachments::declare_attachment<NPSurfParam>("npSurfParams", true);

    // If branching points extend 5r in each direction and we integrate over twice that size
    // we add around quadOrder/2 Gaussians with sigma=r over a total length of 20r.
    // So we should use about quadOrder=80 to ensure a smooth surface
    // that does not look like a pearl necklace.
	prepare_quadrature();
}


NeuriteProjector::NeuriteProjector(SPIGeometry3d geometry)
: RefinementProjector(geometry)
  //m_quadOrder(80)
{
	typedef Attachment<NeuriteProjector::SurfaceParams> NPSurfParam;
	if (!GlobalAttachments::is_declared("npSurfParams"))
		GlobalAttachments::declare_attachment<NPSurfParam>("npSurfParams", true);

    attach_surf_params();
    prepare_quadrature();
}


NeuriteProjector::~NeuriteProjector()
{}


void NeuriteProjector::set_geometry(SPIGeometry3d geometry)
{
    // call base class method
    RefinementProjector::set_geometry(geometry);

    attach_surf_params();
}



number NeuriteProjector::new_vertex(Vertex* vrt, Edge* parent)
{
    return push_into_place(vrt, parent);
}


number NeuriteProjector::new_vertex(Vertex* vrt, Face* parent)
{
    return push_into_place(vrt, parent);
}


number NeuriteProjector::new_vertex(Vertex* vrt, Volume* parent)
{
    return push_into_place(vrt, parent);
}


void NeuriteProjector::direction_at_grid_object(vector3& dirOut, GridObject* o) const
{
    // treat vertex separately as it is no vertex group
    if (o->base_object_id() == VERTEX)
    {
        Vertex* v = dynamic_cast<Vertex*>(o);
        UG_COND_THROW(!v, "Non-vertex with VERTEX base object id.")

        const SurfaceParams& sp = m_aaSurfParams[v];
        uint32_t nid = (sp.neuriteID << 12 ) >> 12;
        float t = sp.axial;

        const Section& sec = *get_section_iterator(nid, t);

        number te = sec.endParam;
        const number* s = &sec.splineParamsX[0];
        number& v0 = dirOut[0];
        v0 = -3.0*s[0]*(te-t) - 2.0*s[1];
        v0 = v0*(te-t) - s[2];

        s = &sec.splineParamsY[0];
        number& v1 = dirOut[1];
        v1 = -3.0*s[0]*(te-t) - 2.0*s[1];
        v1 = v1*(te-t) - s[2];

        s = &sec.splineParamsZ[0];
        number& v2 = dirOut[2];
        v2 = -3.0*s[0]*(te-t) - 2.0*s[1];
        v2 = v2*(te-t) - s[2];

        return;
    }

    IVertexGroup* vrtGrp = dynamic_cast<IVertexGroup*>(o);
    UG_COND_THROW(!vrtGrp, "Non-vertex element which is not a vertex group.");

    uint32_t nid;
    float t, dummy1, dummy2;
    average_params(nid, t, dummy1, dummy2, vrtGrp);

    const Section& sec = *get_section_iterator(nid, t);

    number te = sec.endParam;
    const number* s = &sec.splineParamsX[0];
    number& v0 = dirOut[0];
    v0 = -3.0*s[0]*(te-t) - 2.0*s[1];
    v0 = v0*(te-t) - s[2];

    s = &sec.splineParamsY[0];
    number& v1 = dirOut[1];
    v1 = -3.0*s[0]*(te-t) - 2.0*s[1];
    v1 = v1*(te-t) - s[2];

    s = &sec.splineParamsZ[0];
    number& v2 = dirOut[2];
    v2 = -3.0*s[0]*(te-t) - 2.0*s[1];
    v2 = v2*(te-t) - s[2];
}



void NeuriteProjector::attach_surf_params()
{
    Grid& grid = this->geometry()->grid();

    UG_COND_THROW(!GlobalAttachments::is_declared("npSurfParams"),
        "GlobalAttachment 'npSurfParams' not declared.");
    m_aSurfParams = GlobalAttachments::attachment<Attachment<SurfaceParams> >("npSurfParams");

    // make sure surfaceParams attachment is attached
    UG_COND_THROW(!grid.has_vertex_attachment(m_aSurfParams),
        "No surface parameter attachment for neurite projector attached to grid.");

    m_aaSurfParams.access(grid, m_aSurfParams);

    // handle attachment values also on higher grid levels (if required)
    MultiGrid* mg = dynamic_cast<MultiGrid*>(&grid);
    if (mg)
    {
        SmartPtr<MultiGrid> spMG(mg);

        // never destroy the grid from here - we did not create it
        ++(*spMG.refcount_ptr());

        m_cah.set_attachment(m_aSurfParams);
        m_cah.set_grid(spMG);
    }
}


void NeuriteProjector::debug_neurites() const
{
	UG_LOGN("BPs of neurites in projector:")
	// print out branching region data
	for (size_t n = 0; n < m_vNeurites.size(); ++n)
	{
		UG_LOGN("Neurite " << n);
		const NeuriteProjector::Neurite& neurite = m_vNeurites[n];
		const std::vector<NeuriteProjector::BranchingRegion> vBR = neurite.vBR;

		for (size_t b = 0; b < vBR.size(); ++b)
		{
			UG_LOGN("  BR " << b << ": " << vBR[b].t);
			SmartPtr<NeuriteProjector::BranchingPoint> bp = vBR[b].bp;

			UG_LOGN("    associated BP data:")
			size_t bpSz = bp->vNid.size();
			if (bpSz != bp->vRegions.size())
			{
				UG_LOGN(      "Size mismatch: vNid " << bpSz << ", vRegions " << bp->vRegions.size());
			}
			else
			{
				for (size_t i = 0; i < bpSz; ++i)
				{
					UG_LOGN("      " << bp->vNid[i] << " (" << bp->vRegions[i]->t << ")");
				}
			}
		}
	}
}


void NeuriteProjector::add_neurite(const Neurite& n)
{
    m_vNeurites.push_back(n);
}


const NeuriteProjector::Neurite& NeuriteProjector::neurite(uint32_t nid) const
{
    UG_COND_THROW(nid >= m_vNeurites.size(),
        "Requested neurite index " << nid << " that is not present in the neurite.");
    return m_vNeurites[nid];
}


Grid::VertexAttachmentAccessor<Attachment<NeuriteProjector::SurfaceParams> >&
NeuriteProjector::surface_params_accessor()
{
    return m_aaSurfParams;
}


const Grid::VertexAttachmentAccessor<Attachment<NeuriteProjector::SurfaceParams> >&
NeuriteProjector::surface_params_accessor() const
{
    return m_aaSurfParams;
}


const std::vector<std::pair<number, number> >& NeuriteProjector::quadrature_points() const
{
    return m_qPoints;
};



std::vector<NeuriteProjector::Section>::const_iterator
NeuriteProjector::get_section_iterator(uint32_t nid, float t) const
{
    Section cmpSec(t);
    const std::vector<Section>& vSections = m_vNeurites[nid].vSec;
    std::vector<Section>::const_iterator itSec =
        std::lower_bound(vSections.begin(), vSections.end(), cmpSec, CompareSections());

    UG_COND_THROW(itSec == vSections.end(),
        "Could not find section for parameter t = " << t << " in neurite " << nid << ".");

    return itSec;
}



static bool cmpQPairs(const std::pair<number, number>& a, const std::pair<number, number>& b)
{return a.first < b.first;}

void NeuriteProjector::prepare_quadrature()
{
    /*
    GaussLegendre gl(m_quadOrder);
    size_t nPts = gl.size();
    m_qPoints.resize(nPts);
    for (size_t i = 0; i < nPts; ++i)
    {
        m_qPoints[i].first = gl.point(i)[0];
        m_qPoints[i].second = gl.weight(i);
    }
    */
    m_qPoints.resize(40);
    m_qPoints[0].first = 4.8061379124697458903340327798768835266e-1;
    m_qPoints[1].first = 5.1938620875302541096659672201231164734e-1;
    m_qPoints[2].first = 4.4195796466237239575827435779598794312e-1;
    m_qPoints[3].first = 5.5804203533762760424172564220401205688e-1;
    m_qPoints[4].first = 4.0365120964931445014224157396742505259e-1;
    m_qPoints[5].first = 5.9634879035068554985775842603257494741e-1;
    m_qPoints[6].first = 3.6592390749637315942940782759570190829e-1;
    m_qPoints[7].first = 6.3407609250362684057059217240429809171e-1;
    m_qPoints[8].first = 3.2900295458712076349625375941040284497e-1;
    m_qPoints[9].first = 6.7099704541287923650374624058959715503e-1;
    m_qPoints[10].first = 2.9311039781419749923756012709814315851e-1;
    m_qPoints[11].first = 7.0688960218580250076243987290185684149e-1;
    m_qPoints[12].first = 2.584620991569106435457167128775884977e-1;
    m_qPoints[13].first = 7.415379008430893564542832871224115023e-1;
    m_qPoints[14].first = 2.2526643745243589896203434723524101488e-1;
    m_qPoints[15].first = 7.7473356254756410103796565276475898512e-1;
    m_qPoints[16].first = 1.9372305516600988102369377488465256131e-1;
    m_qPoints[17].first = 8.0627694483399011897630622511534743869e-1;
    m_qPoints[18].first = 1.6402165769291022581032274251925294501e-1;
    m_qPoints[19].first = 8.3597834230708977418967725748074705499e-1;
    m_qPoints[20].first = 1.3634087240503644835950177412253472572e-1;
    m_qPoints[21].first = 8.6365912759496355164049822587746527428e-1;
    m_qPoints[22].first = 1.1084717428674030615251422724675257599e-1;
    m_qPoints[23].first = 8.8915282571325969384748577275324742401e-1;
    m_qPoints[24].first = 8.7693884583344168401839884666950613046e-2;
    m_qPoints[25].first = 9.1230611541665583159816011533304938695e-1;
    m_qPoints[26].first = 6.7020248393870248089609095822690018215e-2;
    m_qPoints[27].first = 9.3297975160612975191039090417730998179e-1;
    m_qPoints[28].first = 4.8950596515562851635873334565753448208e-2;
    m_qPoints[29].first = 9.5104940348443714836412666543424655179e-1;
    m_qPoints[30].first = 3.3593595860661733319573916577397141783e-2;
    m_qPoints[31].first = 9.6640640413933826668042608342260285822e-1;
    m_qPoints[32].first = 2.1041590393104172097729500273620357453e-2;
    m_qPoints[33].first = 9.7895840960689582790227049972637964255e-1;
    m_qPoints[34].first = 1.1370025008112868668314858143548096511e-2;
    m_qPoints[35].first = 9.8862997499188713133168514185645190349e-1;
    m_qPoints[36].first = 4.6368806502714967734728238893139225189e-3;
    m_qPoints[37].first = 9.9536311934972850322652717611068607748e-1;
    m_qPoints[38].first = 8.8114514472039982518864878970675383211e-4;
    m_qPoints[39].first = 9.9911885485527960017481135121029324617e-1;

    m_qPoints[0].second = 3.8752973989212405631861981479163163482e-2;
    m_qPoints[1].second = 3.8752973989212405631861981479163163482e-2;
    m_qPoints[2].second = 3.8519909082123982794153767141905124262e-2;
    m_qPoints[3].second = 3.8519909082123982794153767141905124262e-2;
    m_qPoints[4].second = 3.8055180950313121185779037961247411506e-2;
    m_qPoints[5].second = 3.8055180950313121185779037961247411506e-2;
    m_qPoints[6].second = 3.7361584528984132100094668130662336596e-2;
    m_qPoints[7].second = 3.7361584528984132100094668130662336596e-2;
    m_qPoints[8].second = 3.6443291197902029530255341721258917929e-2;
    m_qPoints[9].second = 3.6443291197902029530255341721258917929e-2;
    m_qPoints[10].second = 3.530582369564338984774181542764341618e-2;
    m_qPoints[11].second = 3.530582369564338984774181542764341618e-2;
    m_qPoints[12].second = 3.3956022907616951912845054115961992992e-2;
    m_qPoints[13].second = 3.3956022907616951912845054115961992992e-2;
    m_qPoints[14].second = 3.2402006728300519037277264783376365016e-2;
    m_qPoints[15].second = 3.2402006728300519037277264783376365016e-2;
    m_qPoints[16].second = 3.0653121246464469583268998204199297951e-2;
    m_qPoints[17].second = 3.0653121246464469583268998204199297951e-2;
    m_qPoints[18].second = 2.87198845496957756833088654552129928e-2;
    m_qPoints[19].second = 2.87198845496957756833088654552129928e-2;
    m_qPoints[20].second = 2.6613923491968412177498239886130252278e-2;
    m_qPoints[21].second = 2.6613923491968412177498239886130252278e-2;
    m_qPoints[22].second = 2.4347903817536116030717080224073194034e-2;
    m_qPoints[23].second = 2.4347903817536116030717080224073194034e-2;
    m_qPoints[24].second = 2.1935454092836635995837343020857747906e-2;
    m_qPoints[25].second = 2.1935454092836635995837343020857747906e-2;
    m_qPoints[26].second = 1.9391083987236008819986015645223081127e-2;
    m_qPoints[27].second = 1.9391083987236008819986015645223081127e-2;
    m_qPoints[28].second = 1.6730097641273923696339091543205424489e-2;
    m_qPoints[29].second = 1.6730097641273923696339091543205424489e-2;
    m_qPoints[30].second = 1.3968503490011700549244578753860538651e-2;
    m_qPoints[31].second = 1.3968503490011700549244578753860538651e-2;
    m_qPoints[32].second = 1.1122924597083478630752162092104286604e-2;
    m_qPoints[33].second = 1.1122924597083478630752162092104286604e-2;
    m_qPoints[34].second = 8.2105291909539443564317424411819636462e-3;
    m_qPoints[35].second = 8.2105291909539443564317424411819636462e-3;
    m_qPoints[36].second = 5.2491422655764068073710855336398261884e-3;
    m_qPoints[37].second = 5.2491422655764068073710855336398261884e-3;
    m_qPoints[38].second = 2.2606385492665956292358664390926663639e-3;

    std::sort(m_qPoints.begin(), m_qPoints.end(), cmpQPairs);
}

static void compute_first_section_end_points
(
    vector3& pos0Out,
    vector3& pos1Out,
    std::vector<NeuriteProjector::Section>::const_iterator secIt
)
{
	number te = secIt->endParam;
	const number* s = &secIt->splineParamsX[0];
	number& p0 = pos0Out[0];
	p0 = s[0]*te + s[1];
	p0 = p0*te + s[2];
	p0 = p0*te + s[3];
	pos1Out[0] = s[3];

	s = &secIt->splineParamsY[0];
	number& p1 = pos0Out[1];
	p1 = s[0]*te + s[1];
	p1 = p1*te + s[2];
	p1 = p1*te + s[3];
	pos1Out[1] = s[3];

	s = &secIt->splineParamsZ[0];
	number& p2 = pos0Out[2];
	p2 = s[0]*te + s[1];
	p2 = p2*te + s[2];
	p2 = p2*te + s[3];
	pos1Out[2] = s[3];
}


static void compute_position_and_velocity_in_section
(
    vector3& posAxOut,
    vector3& velOut,
    number& radiusOut,
    std::vector<NeuriteProjector::Section>::const_iterator secIt,
    float t
)
{
    number te = secIt->endParam;
    const number* s = &secIt->splineParamsX[0];
    number& p0 = posAxOut[0];
    number& v0 = velOut[0];
    p0 = s[0]*(te-t) + s[1];  v0 = -3.0*s[0]*(te-t) - 2.0*s[1];
    p0 = p0*(te-t) + s[2];    v0 = v0*(te-t) - s[2];
    p0 = p0*(te-t) + s[3];

    s = &secIt->splineParamsY[0];
    number& p1 = posAxOut[1];
    number& v1 = velOut[1];
    p1 = s[0]*(te-t) + s[1];  v1 = -3.0*s[0]*(te-t) - 2.0*s[1];
    p1 = p1*(te-t) + s[2];    v1 = v1*(te-t) - s[2];
    p1 = p1*(te-t) + s[3];

    s = &secIt->splineParamsZ[0];
    number& p2 = posAxOut[2];
    number& v2 = velOut[2];
    p2 = s[0]*(te-t) + s[1];  v2 = -3.0*s[0]*(te-t) - 2.0*s[1];
    p2 = p2*(te-t) + s[2];    v2 = v2*(te-t) - s[2];
    p2 = p2*(te-t) + s[3];

    s = &secIt->splineParamsR[0];
    radiusOut = s[0]*(te-t) + s[1];
    radiusOut = radiusOut*(te-t) + s[2];
    radiusOut = radiusOut*(te-t) + s[3];
}


static inline void compute_ONB
(
    vector3& firstOut,
    vector3& secondOut,
    vector3& thirdOut,
    const vector3& firstIn,
    const vector3& refDir
)
{
    VecNormalize(firstOut, firstIn);
    number fac = VecProd(refDir, firstOut);
    VecScaleAdd(secondOut, 1.0, refDir, -fac, firstOut);
    VecNormalize(secondOut, secondOut);

    VecCross(thirdOut, firstOut, secondOut);
}


// computes the polar angle phi of a vector
// given a rotational axis direction and two additional
// directions, together being a orthoNORMAL basis
static inline number compute_angle
(
    const vector3& axis,
    const vector3& secondAxis,
    const vector3& thirdAxis,
    const vector3& posFromAxis
)
{
    // eliminate component in axis direction
	vector3 posFromAxisCopy;
    VecScaleAdd(posFromAxisCopy, 1.0, posFromAxis, -VecProd(posFromAxis, axis), axis);

    // compute coord vector (components 2 and 3) w.r.t. ONB
    vector2 relCoord;
    relCoord[0] = VecProd(posFromAxis, secondAxis);
    VecScaleAdd(posFromAxisCopy, 1.0, posFromAxisCopy, -relCoord[0], secondAxis);
    relCoord[1] = VecProd(posFromAxisCopy, thirdAxis);
    VecNormalize(relCoord, relCoord);

    // get angle from coord vector
    number angle;
    if (fabs(relCoord[0]) < 1e-8)
        angle = relCoord[1] < 0 ? 1.5*PI : 0.5*PI;
    else
        angle = relCoord[0] < 0 ? PI - atan(-relCoord[1]/relCoord[0]) : atan(relCoord[1]/relCoord[0]);

    // angle should be in [0,2*PI)
    if (angle < 0) angle += 2.0*PI;

    return angle;
}


void NeuriteProjector::average_params
(
	uint32_t& neuriteID,
	float& t,
	float& angle,
	float& radius,
	const IVertexGroup* parent
) const
{
	// find neurite ID parent belongs to
	size_t nVrt = parent->num_vertices();
	std::vector<std::vector<std::pair<uint32_t, uint32_t> > > candidates(nVrt);
	for (size_t i = 0; i < nVrt; ++i)
	{
		const SurfaceParams& surfParams = m_aaSurfParams[parent->vertex(i)];
		uint32_t thisNID = surfParams.neuriteID & ((1 << 20) - 1);
		candidates[i].push_back(std::make_pair(thisNID, 0));

		if (surfParams.neuriteID > (1 << 20))  // vrt also belongs to branch(es)
		{
			const uint32_t br = (surfParams.neuriteID & (255 << 20)) >> 20;
			const std::vector<uint32_t>& vNID = m_vNeurites[thisNID].vBR[br].bp->vNid;
			const uint32_t children = surfParams.neuriteID >> 28;
			for (int j = 0; j < 4; ++j)
				if (children & (1 << j))
					candidates[i].push_back(std::make_pair(vNID[j+1], (br << 20) + (1 << (28+j))));  // first NID is parent's
		}
	}

	// special case:
	// when refining the initial BP volume the center vertex needs to belong
	// to all branches and the parent
	if (nVrt == 8)
	{
		// all must have the same parent NID
		bool allBelongToParent = true;
		uint32_t parentNID = candidates[0][0].first;
		for (size_t i = 1; i < nVrt; ++i)
		{
			if (candidates[i][0].first != parentNID)
			{
				allBelongToParent = false;
				break;
			}
		}

		// any child NID that figures at least four times needs to be added to all vertices
		// todo: this implementation only handles one child (the first)
		if (allBelongToParent)
		{
			std::pair<uint32_t, uint32_t> childInfo;
			int nChildVrts = 0;
			for (size_t i = 0; i < nVrt; ++i)
			{
				if (candidates[i].size() > 1)
				{
					++nChildVrts;
					if (nChildVrts == 1)
						childInfo = candidates[i][1];
				}
			}
			if (nChildVrts >= 4)
				for (size_t i = 0; i < nVrt; ++i)
					if (candidates[i].size() == 1)
						candidates[i].push_back(childInfo);
		}
	}

	// find candidates that figure in all parent vertices
	for (size_t i = 1; i < nVrt; ++i)
	{
		for (size_t j = 0; j < candidates[0].size();)
		{
			uint32_t candNID = candidates[0][j].first;
			size_t k = 0;
			for (; k < candidates[i].size(); ++k)
				if (candidates[i][k].first == candNID)
					break;

			if (k == candidates[i].size())
				candidates[0].erase(candidates[0].begin() + j);
			else
				++j;
		}
	}

	if (!candidates[0].size())
	{
		uint32_t maxNID = 0;
		for (size_t i = 0; i < nVrt; ++i)
		{
			const SurfaceParams& surfParams = m_aaSurfParams[parent->vertex(i)];
			uint32_t thisNID = surfParams.neuriteID & ((1 << 20) - 1);
			maxNID = std::max(maxNID, thisNID);
		}
		candidates[0].push_back(std::make_pair(maxNID, 0));
	}

	uint32_t plainNID = neuriteID = candidates[0][0].first;
	for (size_t j = 1; j < candidates[0].size(); ++j)
		neuriteID = neuriteID | candidates[0][j].second;

	// special case:
	// when refining the initial connecting quadrilateral face of a branch
	// the center vertex needs to belong exclusively to the branch, not to the parent
	if (candidates[0].size() == 2 && nVrt == 4)
		plainNID = neuriteID = candidates[0][1].first;


	// average t, phi, r
	t = 0.0;
	float minRad = 1.0;
	float maxRad = 0.0;
	vector2 v(0.0);
	size_t nForeignParents = 0;
	for (size_t i = 0; i < nVrt; ++i)
	{
		const SurfaceParams& surfParams = m_aaSurfParams[parent->vertex(i)];
		uint32_t nid = surfParams.neuriteID & ((1 << 20) - 1);
		minRad = std::min(minRad, surfParams.radial);
		maxRad = std::max(maxRad, surfParams.radial);
		if (nid == plainNID)
		{
			t += surfParams.axial;
			if (surfParams.radial > 0 && surfParams.axial < 2.0)
			{
				const float phi = surfParams.angular;
				VecAdd(v, v, vector2(surfParams.radial*cos(phi), surfParams.radial*sin(phi)));
			}
		}
		else
		{
			++nForeignParents;
			// Here we are on a branching vertex that belongs to the parent.
			// We could just use t=0 here, but this is imprecise as t = 0
			// is in the middle of the parent neurite and also axially set off,
			// so we calculate the actual t and phi params here.
			std::vector<Section>::const_iterator secIt = m_vNeurites[plainNID].vSec.begin();
			vector3 pos0, pos1;
			compute_first_section_end_points(pos0, pos1, secIt);
			const vector3& vrtPos = this->pos(parent->vertex(i));
			VecSubtract(pos1, pos1, pos0);
			VecSubtract(pos0, vrtPos, pos0);
			number axPos = VecProd(pos0, pos1) / VecNormSquared(pos1) * secIt->endParam;

			vector3 vel;
			number surfRadius;
			compute_position_and_velocity_in_section(pos1, vel, surfRadius, secIt, axPos);
			const vector3& refDir = m_vNeurites[plainNID].refDir;
			vector3 posRefDir, thirdDir;
			compute_ONB(vel, posRefDir, thirdDir, vel, refDir);
			vector3 posFromAxis;
			VecSubtract(posFromAxis, vrtPos, pos1);
			number phi = compute_angle(vel, posRefDir, thirdDir, posFromAxis);

			t += axPos;
			const number radial = VecNorm(posFromAxis) / surfRadius;
			VecAdd(v, v, vector2(radial*cos(phi), radial*sin(phi)));
		}
	}
	t /= nVrt;
	radius = 0.5*(maxRad + minRad);

	// special cases: initial volume or face of branch:
	// the center vertex often is not close enough to r = 0, so enforce it
	if (nForeignParents == 4)
	{
		angle = 0.0;  // center angle is 0.0 by default
		radius = 0.0;
		return;
	}

	// special case tip of neurite
    if (fabs(t-2.0) < 1e-8)
    	return;

    if (fabs(v[0]) < 1e-4)
    {
		if (fabs(v[1]) < 1e-4)  // do not alter radius in tip
		{
			angle = 0.0;  // center angle is 0.0 by default
			radius = 0.0;
		}
		else
			angle = v[1] < 0 ? 1.5*PI : 0.5*PI;
    }
	else
		angle = v[0] < 0 ? PI - atan(-v[1]/v[0]) : atan(v[1]/v[0]);

	if (angle < 0)
		angle += 2.0*PI;

}


void NeuriteProjector::average_pos_from_parent(vector3& posOut, const IVertexGroup* parent) const
{
    posOut = 0.0;
    size_t nVrt = parent->num_vertices();
    for (size_t i = 0; i < nVrt; ++i)
        posOut += this->pos(parent->vertex(i));
    posOut /= (number) nVrt;
}


/*
void NeuriteProjector::average_pos_from_parent_weighted(vector3& posOut, const IVertexGroup* parent) const
{
	MultiGrid* mg = dynamic_cast<MultiGrid*>(&this->geometry()->grid());
	UG_COND_THROW(!mg, "Underlying grid is not a multigrid.");
	int lv = mg->get_level<Vertex>(parent->vertex(0));
	if (lv == 0)
		lv = 1;

    posOut = 0.0;
    size_t nVrt = parent->num_vertices();
    std::vector<int> vSheathID(nVrt);
    vSheathID[0] = 0;
    int sheathID[2] = {std::round(m_aaSurfParams[parent->vertex(0)].radial*(1 << (lv-1))), -1};
    int count[2] = {1, 0};
    for (size_t i = 1; i < nVrt; ++i)
    {
    	int thisSheathID = std::round(m_aaSurfParams[parent->vertex(i)].radial*(1 << (lv-1)));
    	if (thisSheathID != sheathID[0] && sheathID[1] == -1)
    		sheathID[1] = thisSheathID;
    	if (thisSheathID == sheathID[0])
    	{
    		++count[0];
        	vSheathID[i] = 0;
    	}
    	else if (thisSheathID == sheathID[1])
    	{
    		++count[1];
        	vSheathID[i] = 1;
    	}
    	else
    		UG_THROW("An element must only be part of at most 2 sheaths.");
    }

    if (!count[1])
    	count[1] = 1;

    for (size_t i = 0; i < nVrt; ++i)
    	VecScaleAppend(posOut, count[1-vSheathID[i]], this->pos(parent->vertex(i)));

    posOut /= 2*count[0]*count[1];
}
*/


vector3 NeuriteProjector::position(Vertex* vrt) const
{
	return this->pos(vrt);
}



number NeuriteProjector::axial_range_around_branching_region
(
	uint32_t nid,
	size_t brInd,
	number numberOfRadii
) const
{
	const Neurite& neurite = m_vNeurites[nid];
	const BranchingRegion& br = neurite.vBR[brInd];

	std::vector<Section>::const_iterator secIt = get_section_iterator(nid, br.t);
	vector3 secStartPos;
	vector3 secEndPos;
	const number te = secIt->endParam;
	number ts = 0.0;
	if (secIt == m_vNeurites[nid].vSec.begin())
		compute_first_section_end_points(secStartPos, secEndPos, secIt);
	else
	{
		secEndPos[0] = secIt->splineParamsX[3];
		secEndPos[1] = secIt->splineParamsY[3];
		secEndPos[2] = secIt->splineParamsZ[3];
		--secIt;
		ts = secIt->endParam;
		secStartPos[0] = secIt->splineParamsX[3];
		secStartPos[1] = secIt->splineParamsY[3];
		secStartPos[2] = secIt->splineParamsZ[3];
	}
	const number neuriteLengthApprox = VecDistance(secEndPos, secStartPos) / (te - ts);

	const number* s = &secIt->splineParamsR[0];
	number radius = s[0]*(te-br.t) + s[1];
	radius = radius*(te-br.t) + s[2];
	radius = radius*(te-br.t) + s[3];

	return numberOfRadii*radius / neuriteLengthApprox;
}


void NeuriteProjector::print_surface_params(Vertex* v) const
{
	const SurfaceParams& sp = m_aaSurfParams[v];
	UG_LOGN("neuriteID: " << sp.neuriteID);
	UG_LOGN("axial:     " << sp.axial);
	UG_LOGN("angular:   " << sp.angular);
	UG_LOGN("radial:    " << sp.radial);
}


static void bp_defect_and_gradient
(
    number& defectOut,
    vector3& gradientOut,
    const std::vector<NeuriteProjector::BPProjectionHelper>& bpList,
    const vector3& x,
	number rad,
    const NeuriteProjector* np
)
{
    // get integration rule points and weights
    const std::vector<std::pair<number, number> >& qPoints = np->quadrature_points();
    size_t nPts = qPoints.size();

    // evaluate integrand at points
    defectOut = 0.0;
    gradientOut = 0.0;
    size_t nParts = bpList.size();
    for (size_t i = 0; i < nParts; ++i)
    {
        const NeuriteProjector::BPProjectionHelper& helper = bpList[i];
        const number& start = helper.start;
        const number& end = helper.end;
        std::vector<NeuriteProjector::Section>::const_iterator secIt = helper.sec_start;

        // iterate IPs
        for (size_t j = 0; j < nPts; ++j)
        {
            // transform point coords to current context
            float t = (float) (start + qPoints[j].first*(end-start));
            while (secIt->endParam < t) ++secIt;    // this should be safe as last section must end with 1.0

            vector3 posAx, vel, blub;
            number radius;
            compute_position_and_velocity_in_section(posAx, vel, radius, secIt, t);

            number radInv = 1.0 / (rad*radius);
            VecSubtract(posAx, posAx, x);
            number posNormSq = VecNormSquared(posAx);
            number velNorm = sqrt(VecNormSquared(vel));
            number integrandVal = radInv*exp(-posNormSq*radInv*radInv) * velNorm;
            number gradIntegrandVal = 2.0*radInv*radInv * integrandVal;
            VecScale(blub, posAx, gradIntegrandVal);

            number w = qPoints[j].second;
            defectOut += integrandVal * w * (end-start);
            VecScaleAdd(gradientOut, 1.0, gradientOut, w * (end-start), blub);
/*
			UG_LOGN("  Ival: " << integrandVal << ",  iExp: " << -posNormSq*radInv*radInv
				<< ",  velNorm: " << velNorm << ",  weight: " << w
				<< ",  t: " << t << ",  intvl: " << end-start
				<< ", posAx: " << posAx);
*/
        }
    }

    defectOut -= sqrt(PI)*exp(-1.0);
}


/*
static void pos_on_neurite_spine
(
    vector3& posOut,
    const NeuriteProjector::Neurite& neurite,
    size_t neuriteID,
    float& t
)
{
    const std::vector<NeuriteProjector::Section>& vSections = neurite.vSec;

    // find correct section
    NeuriteProjector::Section cmpSec(t);
    std::vector<NeuriteProjector::Section>::const_iterator it =
        std::lower_bound(vSections.begin(), vSections.end(), cmpSec, NeuriteProjector::CompareSections());

    UG_COND_THROW(it == vSections.end(),
        "Could not find section for parameter t = " << t << " in neurite " << neuriteID << ".");

    // calculate correct axial position
    // and velocity dir
    vector3 vel_dummy;
    number radius_dummy;
    compute_position_and_velocity_in_section(posOut, vel_dummy, radius_dummy, it, t);
}
*/


static void newton_for_bp_projection
(
    vector3& posOut,
    const std::vector<NeuriteProjector::BPProjectionHelper>& vProjHelp,
	number rad,
    const NeuriteProjector* np
)
{
	vector3 posStart = posOut;
    // calculate start defect and gradient
    number def;
    number def_init;
    vector3 grad;
    bp_defect_and_gradient(def, grad, vProjHelp, posOut, rad, np);
    def_init = fabs(def);

    // perform some kind of Newton search for the correct position
    size_t maxIt = 10;
    number minDef = 1e-8*sqrt(PI)*exp(-1.0);
    size_t iter = 0;

//UG_LOGN(iter << "  " << def << "  " << grad << "  " << posOut);
    while (fabs(def) > minDef && fabs(def) > 1e-8 * def_init && ++iter <= maxIt)
    {
        vector3 posTest;
        vector3 gradTest;
        number defTest;
        VecScaleAdd(posTest, 1.0, posOut, -def / VecNormSquared(grad), grad);
        bp_defect_and_gradient(defTest, gradTest, vProjHelp, posTest, rad, np);

        // line search
        number lambda = 0.5;
        number bestDef = defTest;
        vector3 bestGrad = gradTest;
        vector3 bestPos = posTest;
        while (fabs(defTest) >= fabs(def) && lambda > 0.001)
        {
//UG_LOGN("    line search with lambda = " << lambda);
            VecScaleAdd(posTest, 1.0, posOut, -lambda*def / VecNormSquared(grad), grad);
            bp_defect_and_gradient(defTest, gradTest, vProjHelp, posTest, rad, np);
            if (fabs(defTest) < fabs(bestDef))
            {
                bestDef = defTest;
                bestGrad = gradTest;
                bestPos = posTest;
            }
            lambda *= 0.5;
        }
        def = bestDef;
        grad = gradTest;
        posOut = bestPos;

//UG_LOGN(iter << "  " << def << "  " << grad << "  " << posOut);
    }
    UG_COND_THROW(def != def,
        "Newton iteration did not converge for branching point projection at " << posStart << ". Defect is NaN.")
    UG_COND_THROW(fabs(def) > minDef && fabs(def) > 1e-8 * def_init,
        "Newton iteration did not converge for branching point projection at " << posStart << ".")
}


static void pos_in_neurite
(
    vector3& posOut,
    const NeuriteProjector::Neurite& neurite,
    size_t neuriteID,
    float t,
    float angle,
    float rad
)
{
    const std::vector<NeuriteProjector::Section>& vSections = neurite.vSec;

    // find correct section
    NeuriteProjector::Section cmpSec(t);
    std::vector<NeuriteProjector::Section>::const_iterator it =
        std::lower_bound(vSections.begin(), vSections.end(), cmpSec, NeuriteProjector::CompareSections());

    UG_COND_THROW(it == vSections.end(),
        "Could not find section for parameter t = " << t << " in neurite " << neuriteID << ".");

    // calculate correct axial position
    // and velocity dir
    vector3 posAx, vel;
    number radius;
    compute_position_and_velocity_in_section(posAx, vel, radius, it, t);

    // calculate orthonormal basis vectors
    vector3 projRefDir, thirdDir;
    compute_ONB(vel, projRefDir, thirdDir, vel, neurite.refDir);

    // calculate new position
    VecScaleAdd(posOut, 1.0, posAx, rad*radius*cos(angle), projRefDir, rad*radius*sin(angle), thirdDir);
}


#if 0
static void bp_newton_start_pos
(
    vector3& initOut,
    uint32_t neuriteID,
	float t,
	float angle,
	float rad,
	const IVertexGroup* parent,
    const NeuriteProjector* np
)
{
    // If t1, t2 are the axial positions of the (edge) parent vertices,
    // then construct the spline vectors v1, v2 corresponding to these
    // axial coordinates. Then construct the spline vector v3 corresponding
    // to t3 = 0.5*(t1+t2) and calculate:
    // q = ||v3 - 0.5*(v1+v2)|| / ||v1 - v2||
    // Then if q is smaller than some threshold value, the underlying
    // spline is quasi-linear on the parent, so we can use the averaged
    // vertex positions as initial solution.
    // Otherwise, we take the neurite surface position of t3.
    // Or, even better, some weighted version of this which coincides
    // with the first case if its condition applies.
	vector3 pos_avg;
	np->average_pos_from_parent(pos_avg, parent);
	vector3 pos_avg_onNeurite;
	pos_in_neurite(pos_avg_onNeurite, np->neurite(neuriteID), neuriteID, t, angle, rad);

	number distSq = VecDistanceSq(pos_avg_onNeurite, pos_avg);

	number qSq;
	if (rad*rad <= 1e-8*distSq)
		qSq = 0.0;
	else
		qSq = distSq / (rad*rad);
	number w1 = qSq*qSq / (qSq*qSq + 1e-4);	// in [0,1] with w1 = 0.5 for q = 0.1;
	VecScaleAdd(initOut, w1, pos_avg_onNeurite, 1.0-w1, pos_avg);
}
#endif


static void pos_in_bp
(
    vector3& posOut,
    const NeuriteProjector::Neurite& neurite,
	uint32_t neuriteID,
    float& t,
    float& angle,
    float& rad,
    std::vector<NeuriteProjector::BranchingRegion>::const_iterator& it,
    const IVertexGroup* parent,
    const NeuriteProjector* np
)
{
	// if vertex is on the neurite backbone, simply project onto spline
	// (Newton iteration will not succeed for Jacobian is zero)
	if (rad < 1e-5)
	{
		pos_in_neurite(posOut, np->neurite(neuriteID), neuriteID, t, angle, rad);
		return;
	}
/*
	// if inside BP (and not on surface) do not project
	else if (rad < 0.9999)
	{
		np->average_pos_from_parent_weighted(posOut, parent);
		return;
	}
*/

	// preparations for Newton method:
	// save integration start and end positions of all BRs of this BP,
	// also save a section iterator to start from
	SmartPtr<NeuriteProjector::BranchingPoint> bp = it->bp;
	size_t nParts = bp->vRegions.size();
	std::vector<NeuriteProjector::BPProjectionHelper> vProjHelp(nParts);
	const std::vector<NeuriteProjector::Section>* secs;
	for (size_t i = 0; i < nParts; ++i)
	{
		number& start = vProjHelp[i].start;
		number& end = vProjHelp[i].end;
		std::vector<NeuriteProjector::Section>::const_iterator& sec_start = vProjHelp[i].sec_start;

		uint32_t nid = bp->vNid[i];
		const NeuriteProjector::BranchingRegion* br = bp->vRegions[i];

		UG_COND_THROW(!br, "Requested branching region not accessible.");

		// integrate over twice the size of the original BR (i.e., 10 times rad*radius in each direction)
		const number range = np->axial_range_around_branching_region(nid, (size_t) (br - &np->neurite(nid).vBR[0]), 10.0*rad);
		start = std::max(br->t - range, 0.0);
		end = std::min(br->t + range, 1.0);

		secs = &np->neurite(nid).vSec;
		if (start == 0.0)
			sec_start = secs->begin();
		else
		{
			NeuriteProjector::Section cmpSec(start);
			sec_start = std::lower_bound(secs->begin(), secs->end(), cmpSec, NeuriteProjector::CompareSections());
			UG_COND_THROW(sec_start == secs->end(),
				"Could not find section for start parameter t = " << start << ".");
		}
	}

	// determine suitable start position for Newton iteration
	np->average_pos_from_parent(posOut, parent); // best results on regular BPs
	//pos_in_neurite(posOut, np->neurite(neuriteID), neuriteID, t, angle, rad);
	//bp_newton_start_pos(posOut, neuriteID, t, angle, rad, parent, np);

	// perform Newton iteration
	try {newton_for_bp_projection(posOut, vProjHelp, rad, np);}
	UG_CATCH_THROW("Newton iteration for neurite projection at branching point failed"
			" in neurite " << neuriteID << ".");

/*
	// FIXME: This might not be completely correct; at least,
	//        for two or more refinements, there are ripples
	//        in refined branching points.
	//        This becomes worse with every refinement
	//        resulting in self-intersections.
	// update the surface param info to the new position
	// In order not to have to compute zeros of a 5th order polynomial,
	// we make a linearity approximation here:
	// s(t) = s(t0) + v(t0)*(t-t0);
	// v(t) = v(t0)
	// Then from v(t) * (s(t)-pos) = 0, we get:
	// t = t0 + v(t0) * (pos - s(t0)) / (v(t0)*v(t0))
	vector3 s, v;
	number radius;
	std::vector<NeuriteProjector::Section>::const_iterator secIt = neurite.vSec.begin();
	while (t > secIt->endParam)
		++secIt;
	compute_position_and_velocity_in_section(s, v, radius, secIt, t);
	VecSubtract(s, posOut, s);
	t += VecProd(v,s) / VecProd(v,v);

	// and now angle
	vector3 projRefDir;
	vector3 thirdDir;
	compute_ONB(v, projRefDir, thirdDir, v, neurite.refDir);
	angle = compute_angle(v, projRefDir, thirdDir, s);

	// and radius
	//rad = VecNorm(s) / radius;


	// if we have ended up outside of the BP, then use the usual positioning
	if (t > it->tend)
	{
		vector3 posAx;
		number radius;
		compute_position_and_velocity_in_section(posAx, v, radius, secIt, t);
		VecScaleAdd(posOut, 1.0, posAx, rad*radius*cos(angle), projRefDir, rad*radius*sin(angle), thirdDir);
	}
*/
}


static void pos_on_surface_tip
(
    vector3& posOut,
    const NeuriteProjector::Neurite& neurite,
    const IVertexGroup* parent,
    const NeuriteProjector* np,
	number rad
)
{
    const std::vector<NeuriteProjector::Section>& vSections = neurite.vSec;

    // initial position: regular refinement
    np->average_pos_from_parent(posOut, parent);

    // project to half-sphere with given radius over tip center
    // TODO: One might think about something more sophisticated,
    // as the current approach ensures only continuity of the radius
    // at tips, but not differentiability.
    const NeuriteProjector::Section& sec = vSections[vSections.size()-1];
    vector3 tipCenter(sec.splineParamsX[3], sec.splineParamsY[3], sec.splineParamsZ[3]);
    number radius = sec.splineParamsR[3];

    vector3 radialVec;
    VecSubtract(radialVec, posOut, tipCenter);
    number radNorm = sqrt(VecProd(radialVec, radialVec));

    // only project if we are not in the center of the half sphere
    if (radNorm >= 1e-5*rad*radius)
    {
    	VecScale(radialVec, radialVec, rad*radius/radNorm);
    	VecAdd(posOut, tipCenter, radialVec);
    }
}



number NeuriteProjector::push_into_place(Vertex* vrt, const IVertexGroup* parent)
{
	// average axial and angular params from parent;
	// also decide on neuriteID
	uint32_t neuriteID;
	float t;
	float angle;
	float rad;
	average_params(neuriteID, t, angle, rad, parent);

	//UG_LOGN("averaged params: nid " << neuriteID << "   t " << t << "   angle "
	//		<< angle << "   rad " << rad);

	const uint32_t plainNID = (neuriteID << 12) >> 12;

	UG_COND_THROW(plainNID >= m_vNeurites.size(), "Requested neurite ID which is not present.");

	const Neurite& neurite = m_vNeurites[plainNID];
	const std::vector<BranchingRegion>& vBR = neurite.vBR;

	// vector for new position
	vector3 pos(0.0);

	// FOUR CASES can occur:
	// 1. We are at a branching point.
	// 2. We are well inside a regular piece of neurite.
	// 3. We are at the tip of a neurite.
	// 4. We are at the soma.

	// case 1: branching point?
	BranchingRegion cmpBR(t);
	std::vector<BranchingRegion>::const_iterator it =
		std::lower_bound(vBR.begin(), vBR.end(), cmpBR, CompareBranchingRegionEnds());

	if ((it != vBR.end() && it->t - t < axial_range_around_branching_region(plainNID, (size_t) (it - vBR.begin()), 5.0*rad))
		|| (it-- != vBR.begin() && t - it->t < axial_range_around_branching_region(plainNID, (size_t) (it - vBR.begin()), 5.0*rad)))
		pos_in_bp(pos, neurite, plainNID, t, angle, rad, it, parent, this);

	// case 2: normal neurite position
	else if (t <= 1.0)
		pos_in_neurite(pos, neurite, plainNID, t, angle, rad);

	// case 3: tip of neurite
	else
		pos_on_surface_tip(pos, neurite, parent, this, rad);

	// case 4: soma
	// TODO: implement!


	// save new surface params for new vertex
	m_aaSurfParams[vrt].neuriteID = neuriteID;
	m_aaSurfParams[vrt].axial = t;
	m_aaSurfParams[vrt].angular = angle;
	m_aaSurfParams[vrt].radial = rad;

	// set position
	set_pos(vrt, pos);

	return 1.0;
}

// -------------------------------------------------------- //
// DO NOT CHANGE below this line! Needed for serialization. //

std::ostream& operator<<(std::ostream &os, const NeuriteProjector::SurfaceParams& surfParams)
{
    using std::ostringstream;
    ostringstream strs;
    strs << surfParams.neuriteID << " ";
    strs << surfParams.axial << " ";
    strs << surfParams.angular << " ";
    strs << surfParams.radial;
    os << strs.str();
    return os;
}

std::istream& operator>>(std::istream& in, NeuriteProjector::SurfaceParams& surfParams)
{
    std::string temp;
    using boost::lexical_cast;

    in >> temp;
    surfParams.neuriteID = lexical_cast<uint32>(temp);
    temp.clear();

    in >> temp;
    surfParams.axial = (lexical_cast<float>(temp));
    temp.clear();

    in >> temp;
    surfParams.angular = lexical_cast<float>(temp);
    temp.clear();

    in >> temp;
    surfParams.radial = lexical_cast<float>(temp);
    temp.clear();

    return in;
}


} // namespace ug
