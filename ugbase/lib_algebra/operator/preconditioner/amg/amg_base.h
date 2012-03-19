/**
 * \file amg_base.h
 *
 * \author Martin Rupp
 *
 * \date 01.12.2010
 *
 * class declaration for amg base
 *
 * Goethe-Center for Scientific Computing 2010.
 */


#ifndef __H__UG__LIB_ALGEBRA__AMG_SOLVER__AMG_BASE_H__
#define __H__UG__LIB_ALGEBRA__AMG_SOLVER__AMG_BASE_H__

#include "lib_algebra/lib_algebra.h"

#include "lib_algebra/operator/interface/operator_inverse.h"
#include "lib_algebra/operator/interface/operator_iterator.h"
#include "lib_algebra/operator/vector_writer.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include "amg_debug_helper.h"
#ifdef UG_PARALLEL
#include "pcl/pcl.h"
#include "lib_algebra/parallelization/parallel_nodes.h"
#endif

template<typename T>
std::string ToString(const T &t)
{
	std::stringstream out;
	out << t;
	return out.str();
}
#define PRINTLAYOUT(pc, com, Layout1, Layout2) MyPrintLayout(pc, com, (Layout1), (Layout2), #Layout1, #Layout2)

namespace ug{
#ifdef UG_PARALLEL
void MyPrintLayout(pcl::InterfaceCommunicator<IndexLayout> &communicator, IndexLayout &layout1, IndexLayout &layout2, const char *name1, const char *name2);
#endif


template <typename TAlgebra>
class AMGBase:
	public IPreconditioner<	TAlgebra >
{
public:

//	Algebra type
	typedef TAlgebra algebra_type;

//	Vector type
	typedef typename TAlgebra::vector_type vector_type;

//	Matrix type
	typedef typename TAlgebra::matrix_type matrix_type;
	typedef typename TAlgebra::matrix_type prolongation_matrix_type;

///	Matrix Operator type
	typedef typename IPreconditioner<TAlgebra>::matrix_operator_type matrix_operator_type;

	typedef typename matrix_type::value_type value_type;

	class LevelInformation
	{
	public:
		LevelInformation(int _level)
		{
			level = _level;
		}
		double get_creation_time_ms() const { return m_dCreationTimeMS; }

		size_t get_nr_of_nodes_min() const { return m_iNrOfNodesMin; }
		size_t get_nr_of_nodes_max() const { return m_iNrOfNodesMax; }
		size_t get_nr_of_nodes() const { return m_iNrOfNodesSum; }
		size_t get_nnz() const { return m_iNNZsSum; }
		size_t get_nnz_min() const { return m_iNNZsMin; }
		size_t get_nnz_max() const { return m_iNNZsMax; }

		// nr of elements in master/slave interfaces (including multiplicities)
		size_t get_nr_of_interface_elements() const { return m_iInterfaceElements; }

		double get_fill_in() const { return ((double)m_iNNZsSum)/(((double)m_iNrOfNodesSum)*((double)m_iNrOfNodesSum)); }
		double get_avg_nnz_per_row() const { return m_iNNZsSum/(double)m_iNrOfNodesSum; }
		size_t get_max_connections() const { return m_connectionsMax; }
		bool is_valid() const { return this != NULL; }

	public:
		void set_nr_of_nodes(size_t nrOfNodesMin, size_t nrOfNodesMax, size_t nrOfNodesSum)
		{
			m_iNrOfNodesMin = nrOfNodesMin;	m_iNrOfNodesMax = nrOfNodesMax;	m_iNrOfNodesSum = nrOfNodesSum;
		}
		void set_nnz(size_t nnzMin, size_t nnzMax, size_t nnzSum)
		{
			m_iNNZsMin = nnzMin; m_iNNZsMax = nnzMax; m_iNNZsSum= nnzSum;
		}
		void set_max_connections(size_t connectionsMax)
		{
			m_connectionsMax = connectionsMax;
		}

		void set_coarsening_rate(double rate)
		{
			m_dCoarseningRate = rate;
		}

		double get_coarsening_rate() const
		{
			return m_dCoarseningRate;
		}

		std::string tostring() const
		{
			std::stringstream ss;
			ss << "Level " << level << ": creation time: " << get_creation_time_ms() << " ms. number of nodes: " << get_nr_of_nodes() << ". fill in " <<
					get_fill_in()*100 << "%. ";
			if(level != 0) ss << "coarsening rate: " << get_coarsening_rate()*100 << "%. ";
			ss << "nr of interface elements: " << get_nr_of_interface_elements() << " (" << (double)get_nr_of_interface_elements()/(double)get_nr_of_nodes()*100.0 << "%) "
					<< "nnzs: " << get_nnz() << " avgNNZs/row: " << get_avg_nnz_per_row() << " maxCon: " << get_max_connections();
			return ss.str();
		}

	public:
		int level;
		double m_dCoarseningRate;
		double m_dCreationTimeMS;
		size_t m_iNrOfNodesMin;
		size_t m_iNrOfNodesMax;
		size_t m_iNrOfNodesSum;
		size_t m_iNNZsMin;
		size_t m_iNNZsMax;
		size_t m_iNNZsSum;
		size_t m_iInterfaceElements;
		size_t m_connectionsMax;
	};

	struct checkResult
	{
		double preSmoothing;
		double preFSmoothing;
		size_t iInnerIterations;
		double lastCoarseReduction;
		double coarseDefect;
		double coarseCorrection;
		double reduction;
		double postFSmoothing;
		double postSmoothing;
	};

//  functions
	AMGBase();
	virtual SmartPtr<ILinearIterator<vector_type> > clone() = 0;

	//	Name of preconditioner
	virtual ~AMGBase();
	void cleanup();


protected:
	virtual const char* name() const = 0;

//	Preprocess routine
	virtual bool preprocess(matrix_operator_type& mat);

//	Postprocess routine
	virtual bool postprocess() {return true;}

//	Stepping routine
	virtual bool step(matrix_operator_type& mat, vector_type& c, const vector_type& d)
	{
		UG_ASSERT(m_bInited, "not inited?");
		return get_correction(c, d);
	}

	void create_direct_solver(size_t level);
	bool solve_on_base(vector_type &c, vector_type &d, size_t level);

	template<typename TAMGNodes>
	void create_fine_marks(int level, TAMGNodes &amgnodes, size_t N);

	void create_parent_index(int level, stdvector<int> newIndex, size_t nrOfCoarse);

	template<typename TAMGNodes>
	void create_new_indices(prolongation_matrix_type &PoldIndices, prolongation_matrix_type &PnewIndices,
				size_t N, TAMGNodes &amgnodes, stdvector<int> &newIndex, double dEpsilonTruncation);
#ifndef UG_PARALLEL
	template<typename TAMGNodes>
	void serial_process_prolongation(prolongation_matrix_type &PoldIndices, prolongation_matrix_type &PnewIndices, double dEpsilonTruncation, int level, TAMGNodes &amgnodes);
#else
	template<typename TAMGNodes>
	void parallel_process_prolongation(prolongation_matrix_type &PoldIndices, prolongation_matrix_type &PnewIndices, double dEpsilonTruncation, int level, TAMGNodes &amgnodes,
			ParallelNodes &PN, bool bCreateNewNodes, IndexLayout &nextLevelMasterLayout, IndexLayout &nextLevelSlaveLayout);

	void communicate_prolongation(ParallelNodes &PN, prolongation_matrix_type &PoldIndices, bool bCreateNewNodes);

	void create_minimal_layout_for_prolongation(ParallelNodes &PN, prolongation_matrix_type &P, IndexLayout &newMasterLayout, IndexLayout &newSlaveLayout);

	template<typename TAMGNodes>
	void postset_coarse(ParallelNodes &PN, prolongation_matrix_type &PoldIndices, TAMGNodes &nodes);

#endif

#ifdef UG_PARALLEL
public:
	bool gather_vertical(vector_type &vec, vector_type &collectedVec, size_t level, ParallelStorageType type);
	bool broadcast_vertical(vector_type &vec, vector_type &collectedVec, size_t level, ParallelStorageType type);
#endif


public:
	void write_interfaces();
	bool add_correction_and_update_defect(vector_type &c, vector_type &d, size_t level, size_t exactLevel);
	bool check2(const vector_type &const_c, const vector_type &const_d);
	bool check_fsmoothing();

	bool add_correction_and_update_defect(vector_type &c, vector_type &d, size_t level=0);
	bool add_correction_and_update_defect2(vector_type &c, vector_type &d,  matrix_operator_type &A, size_t level=0);
	bool get_correction(vector_type &c, const vector_type &d);

	bool injection(vector_type &vH, const vector_type &v, size_t level);
/*
	size_t get_nr_of_coarse(size_t level)
	{
		assert(level+1 < m_usedLevels);
		return A[level+1]->length;
	}
*/
	size_t get_used_levels() const { return m_usedLevels; }

	bool check_level(vector_type &c, vector_type &d, matrix_type &A, size_t level,
			checkResult &res, const vector_type *solution=NULL);
//	bool check(IMatrixOperator const vector_type &const_c, const vector_type &const_d);
	bool check(const vector_type &const_c, const vector_type &const_d);
//  data

	void 	set_num_presmooth(size_t new_presmooth) 				{ m_numPreSmooth = new_presmooth; }
	size_t	get_num_presmooth()	const				 				{ return m_numPreSmooth; }

	void 	set_num_postsmooth(size_t new_postsmooth) 				{ m_numPostSmooth = new_postsmooth; }
	size_t	get_num_postsmooth() const					 			{ return m_numPostSmooth; }

	void 	set_cycle_type(size_t new_cycletype)
	{
		UG_ASSERT(new_cycletype > 0, "cannot set cycle type " << new_cycletype << ", has to be > 0");
		m_cycleType = new_cycletype;
	}
	size_t	get_cycle_type() const									{ return m_cycleType; }

	void 	set_max_levels(size_t new_max_levels)
	{
		UG_ASSERT(new_max_levels > 0, "cannot set max levels " << new_max_levels << ", has to be > 0");
		m_maxLevels = new_max_levels;
	}
	size_t	get_max_levels() const									{ return m_maxLevels; }

	void 	set_fsmoothing(bool enable) 							{ m_bFSmoothing = enable; }
	bool 	get_fsmoothing() const									{ return m_bFSmoothing; }

	void 	set_max_nodes_for_base(size_t newMaxNodesForBase) 		{ m_maxNodesForBase = newMaxNodesForBase; }
	size_t 	get_max_nodes_for_base() const							{ return m_maxNodesForBase; }

	void 	set_max_fill_before_base(double newMaxFillBeforeBase)	{ m_dMaxFillBeforeBase = newMaxFillBeforeBase;	}
	double	get_max_fill_before_base() const						{ return m_dMaxFillBeforeBase;	}


	void 	set_min_nodes_on_one_processor(size_t newMinNodes)		{ m_minNodesOnOneProcessor = newMinNodes; }
	size_t	get_min_nodes_on_one_processor() const					{ return m_minNodesOnOneProcessor; }

	void 	set_preferred_nodes_on_one_processor(size_t i)			{ m_preferredNodesOnOneProcessor = i; }
	size_t	get_preferred_nodes_on_one_processor() const			{ return m_preferredNodesOnOneProcessor; }

	void 	set_checkLevel_post_iterations(size_t i)				{ m_checkLevelPostIterations = i; }
	size_t	get_checkLevel_post_iterations() const					{ return m_checkLevelPostIterations; }


	void 	set_presmoother(SmartPtr<ILinearIterator<vector_type> > presmoother) {	m_presmoother = presmoother; }
	void 	set_postsmoother(SmartPtr<ILinearIterator<vector_type> > postsmoother) { m_postsmoother = postsmoother; }
	void 	set_base_solver(SmartPtr<ILinearOperatorInverse<vector_type> > basesolver) { m_basesolver = basesolver; }


	void set_position_provider(IPositionProvider<2> *ppp2d)
	{
		m_pPositionProvider2d = ppp2d;
	}

	void set_position_provider(IPositionProvider<3> *ppp3d)
	{
		m_pPositionProvider3d = ppp3d;
	}

	void set_matrix_write_path(const char *path)
	{
		m_writeMatrixPath = path;
		m_writeMatrices = true;
	}

	void set_positions(const MathVector<2> *pos, size_t size)
	{
		m_dbgPositions.resize(size);
		for(size_t i=0; i<size; ++i)
		{
			m_dbgPositions[i].x = pos[i].x;
			m_dbgPositions[i].y = pos[i].y;
			m_dbgPositions[i].z = 0.0;
		}
		m_dbgDimension = 2;
	}
	void set_positions(const MathVector<3> *pos, size_t size)
	{
		m_dbgPositions.resize(size);
		for(size_t i=0; i<size; ++i)
			m_dbgPositions[i] = pos[i];
		m_dbgDimension = 3;
	}

	virtual void tostring() const;

	void set_one_init(bool b)
	{
		m_bOneInit = b;
	}

	void set_nr_of_preiterations_at_check(size_t i)
	{
		m_iNrOfPreiterationsCheck = i;
	}

	void set_Y_cycle(int maxIterations, double dYreduce, double dYabs)
	{
		m_dYreduce = dYreduce;
		m_dYabs = dYabs;
		m_iYCycle = maxIterations;
	}

	//! \return c_A = total nnz of all matrices divided by nnz of matrix A
	double get_operator_complexity() const { return m_dOperatorComplexity; }

	//! \return c_G = total number of nodes of all levels divided by number of nodes on level 0
	double get_grid_complexity() const { return m_dGridComplexity; }

	//! \return the time spent on the whole setup in ms
	double get_timing_whole_setup_ms() const { return m_dTimingWholeSetupMS; }

	//! \return the time spent in the coarse solver setup in ms
	double get_timing_coarse_solver_setup_ms() const { return m_dTimingCoarseSolverSetupMS; }

	void print_level_information() const
	{
		std::cout.setf(std::ios::fixed);
		UG_LOG("Operator Complexity: " << get_operator_complexity() << ", grid complexity: " << get_grid_complexity() << ".\n");
		UG_LOG("Whole setup took " << get_timing_whole_setup_ms() << " ms, coarse solver setup took " << get_timing_coarse_solver_setup_ms() << " ms.\n");
		/*for(int i = 0; i<get_used_levels(); i++)
			UG_LOG(get_level_information(i)->tostring() << "\n");*/

	}

	const LevelInformation *get_level_information(size_t i) const
	{
		if(i < levels.size())
			return &levels[i]->m_levelInformation;
		else return NULL;
	}



protected:
	void write_debug_matrices(matrix_type &AH, prolongation_matrix_type &R, const matrix_type &A,
			prolongation_matrix_type &P, size_t level);

	template<typename TMatrix>
	void write_debug_matrix(TMatrix &mat, size_t fromlevel, size_t tolevel, const char *name);

	template<typename TNodeType>
	void write_debug_matrix_markers(size_t level, const TNodeType &nodes);

	void init_fsmoothing();
	bool writevec(std::string filename, const vector_type &d, size_t level, const vector_type *solution=NULL);
	void update_positions();

	bool create_level_vectors(size_t level);

	bool f_smoothing(vector_type &corr, vector_type &d, size_t level);

// pure virtual functions
	virtual void create_AMG_level(matrix_type &AH, prolongation_matrix_type &R, const matrix_type &A,
				prolongation_matrix_type &P, size_t level) = 0;
	virtual void precalc_level(size_t level) = 0;


private:
#ifdef UG_PARALLEL
	bool agglomerate(size_t level);
#endif

protected:
// data
	size_t 	m_numPreSmooth;						///< nu_1 : nr. of pre-smoothing steps
	size_t 	m_numPostSmooth;					///< nu_2: nr. of post-smoothing steps
	int 	m_cycleType;						///< gamma: cycle type (1 = V-Cycle, 2 = W-Cycle)

	size_t 	m_maxLevels;						///< max. nr of levels used for FAMG
	size_t	m_usedLevels;						///< nr of FAMG levels used

	size_t 	m_maxNodesForBase;					///< max nr of coarse nodes before Base solver is used
	double 	m_dMaxFillBeforeBase;				///< max fill rate before Base solver is used
	size_t	m_minNodesOnOneProcessor;			///< min nr of nodes on one processor (for agglomeration)
	size_t	m_preferredNodesOnOneProcessor;		///< preferred nr of nodes on one processor (for agglomeration)
	bool 	m_bUseCollectedSolver;

	bool	m_bFSmoothing;
	bool	m_bOneInit;

	size_t 	m_iNrOfPreiterationsCheck;

	size_t m_checkLevelPostIterations;

	vector_type *m_vec4;						///< temporary Vector for defect (in get_correction)

	int iteration_glboal;


	bool 	m_writeMatrices;

	std::string m_writeMatrixPath;
	cAMG_helper m_amghelper;					///< helper struct for viewing matrices (optional)
	stdvector<MathVector<3> > m_dbgPositions;	///< positions of geometric grid (optional)
	int m_dbgDimension;							///< dimension of geometric grid (optional)


	SmartPtr<ILinearIterator<vector_type> > m_presmoother;	///< presmoother template
	SmartPtr<ILinearIterator<vector_type> > m_postsmoother;	///< postsmoother template \note: may be pre=post, is optimized away.

	SmartPtr<ILinearOperatorInverse<vector_type> > m_basesolver; ///< the base solver


	bool m_bInited;					///< true if inited. needed since preprocess doesnt give us a InterfaceCommunicator atm.
	double m_dOperatorComplexity;
	double m_dGridComplexity;
	double m_dTimingWholeSetupMS;
	double m_dTimingCoarseSolverSetupMS;

	size_t m_iYCycle;
	double m_dYreduce;
	double m_dYabs;

	IPositionProvider<2> *m_pPositionProvider2d;
	IPositionProvider<3> *m_pPositionProvider3d;

	stdvector<stdvector<int> > m_parentIndex;		///< parentIndex[i] is the index of i on the finer level

	struct AMGLevel
	{
		AMGLevel(int level)
			: m_levelInformation(level),
			  pA(NULL),
			  presmoother(NULL),
			  postsmoother(NULL)
#ifdef UG_PARALLEL
			  ,collectedA(new matrix_operator_type)
#endif
		{
#ifdef UG_PARALLEL
			slaveLayout.clear();
			masterLayout.clear();
			agglomerateMasterLayout.clear();
			bHasBeenMerged = false;
#endif
		}
		LevelInformation m_levelInformation;

		vector_type corr;					///< temporary Vector for storing the correction made on this level
		vector_type cH;						///< temporary Vector for storing rH
		vector_type dH; 					///< temporary Vector for storing eH

		stdvector<bool> is_fine;

		prolongation_matrix_type R; 	///< R Restriction Matrices
		prolongation_matrix_type P; 	///< P Prolongation Matrices
		SmartPtr<matrix_operator_type> pA;				///< A Matrices

		SmartPtr<ILinearIterator<vector_type> > presmoother;	///< presmoothers for each level
		SmartPtr<ILinearIterator<vector_type> > postsmoother;	///< postsmoothers for each level

#ifdef UG_PARALLEL
		pcl::InterfaceCommunicator<IndexLayout> com; ///< the communicator object on this level
		IndexLayout slaveLayout, masterLayout;
		IndexLayout slaveLayout2, masterLayout2;

		stdvector< typename block_traits<typename matrix_type::value_type>::inverse_type > m_diagInv;

		// agglomeration
		bool bHasBeenMerged;
		// level 0 - m_agglomerateLevel
		pcl::ProcessCommunicator *pProcessCommunicator;


		// level 0 - m_agglomerateLevel-1
		pcl::ProcessCommunicator agglomeratedPC;
		IndexLayout agglomerateMasterLayout;

		vector_type collC, collD;
		SmartPtr<matrix_operator_type> collectedA;
		bool bLevelHasMergers;
#endif
		// in contrast to collC, collD, and collectedA, this can also point to other vectors.
		SmartPtr<matrix_operator_type> pAgglomeratedA;
	};

	stdvector<AMGLevel*> levels;

#ifdef UG_PARALLEL
	// on level m_agglomerateLevel
	IndexLayout agglomerateSlaveLayout;
	size_t m_agglomerateLevel;

	// stuff from old level 0 agglomeration
	pcl::ProcessCommunicator m_emptyPC;
	IndexLayout m_emptyLayout;

	bool isMergingSlave(size_t level);
	bool isMergingMaster(size_t level);
	bool isNotMerging(size_t level);

#endif

	void calculate_level_information(size_t level, double createAMGlevelTiming);
};


///	@}

} // namespace ug



#include "amg_base_impl.h"


#endif // __H__UG__LIB_ALGEBRA__AMG_SOLVER__AMG_BASE_H__
