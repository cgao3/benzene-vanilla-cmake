//----------------------------------------------------------------------------
/** @file VCBuilder.hpp
 */
//----------------------------------------------------------------------------

#ifndef VCBUILDER_HPP
#define VCBUILDER_HPP

#include "Hex.hpp"
#include "VC.hpp"
#include "VCList.hpp"
#include "VCSet.hpp"
#include "Groups.hpp"
#include "PatternState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Settings for VCBuilder. */
struct VCBuilderParam
{
    /** Maximum number of VCs in the OR combining rule. */
    int max_ors;

    /** Whether the and-rule can and over the edge or not.
        This results in many more connections. */
    bool and_over_edge;

    /** Whether to augment VC set with pre-computed VC patterns. */
    bool use_patterns;

    /** Whether to use pre-computed patterns between two non-edge
        cells. These can cause an explosion in the number of
        connections. */
    bool use_non_edge_patterns;

    /** Whether to use the crossing rule.
        Note that the crossing rule requires the use of stepping stones/
        mustuse to be useful, so this rule is near-useless unless the
        and_over_edge option is being used as well. */
    bool use_crossing_rule;

    /** Whether to use the greedy union or not. 
        @todo DOCUMENT GREEDY UNION! */
    bool use_greedy_union;

    /** Stop building VCs once a winning connection is constructed. */
    bool abort_on_winning_connection;

    /** Constructor. */
    VCBuilderParam();
};


//----------------------------------------------------------------------------

/** Statistics for the last call to Build(). */
struct VCBuilderStatistics
{
    /** Base connections built. */
    int base_attempts;

    /** Base connections successfully added. */
    int base_successes;

    /** Pattern connections that match the board. */
    int pattern_attempts;
        
    /** Pattern connections successfully added. */
    int pattern_successes;

    /** Full-connections built by and-rule. */
    int and_full_attempts;

    /** Full-connections successfully added by and-rule. */
    int and_full_successes;

    /** Semi-connections built by and-rule. */
    int and_semi_attempts;
        
    /** Semi-connections successfully added by and-rule. */
    int and_semi_successes;

    /** Full-connections built by or-rule. */
    int or_attempts;

    /** Full-connections successfully added by or-rule. */
    int or_successes;

    /** Semi-connections built by crossing-rule. */
    int crossing_attempts;
    
    /** Semi-connections successfully added by crossing-rule. */
    int crossing_successes;
    
    /** Calls to or-rule. */
    int doOrs;

    /** Successfull or-rule calls -- at least one full-connection
        successfully added by this call. */
    int goodOrs;

    /** Fulls shrunk in merge phase. */
    int shrunk0;

    /** Semis shrunk in merge phase. */
    int shrunk1;
    
    /** Semis upgraded to fulls in merge phase. */
    int upgraded;
    
    /** Fulls killed by opponent stones in merge phase. */
    int killed0;
    
    /** Semis killed by opponent stones in merge phase. */
    int killed1;
    
    /** Dumps statistics to a string. */
    std::string ToString() const;
};

//----------------------------------------------------------------------------

/** Builds the Virtual Connections (VCs) between groups of stones a
    single color.
    
    VCs can be built from scratch or incrementally from a previous
    state. We use Anchelevich's rules for VC computation. This means
    that between each pair of cells on the board, we store a VCList of
    FULL connections and another VCList of SEMI connections.
    
    IMPORTANT: Take a list of semis between x and y. If any subset of
    of these semis has an empty intersection, we require that the list
    of full connections between x and y has at least one connection.

    See also:
    - @ref mergeshrink
    - @ref workqueue
*/
class VCBuilder
{
public:

    /** Constructor. */
    VCBuilder(VCBuilderParam& param);
    
    /** Destrutor. */
    ~VCBuilder();

    //----------------------------------------------------------------------

    /** Returns parameters used in search. */
    VCBuilderParam& Parameters();

    /** Returns parameters used in search. */
    const VCBuilderParam& Parameters() const;

    /** Returns statistics for the last run. */
    VCBuilderStatistics Statistics(HexColor color) const;

    /** Clears the statistics for both colors. */
    void ClearStatistics();

    //----------------------------------------------------------------------

    /** Computes connections from scratch. Old connections are removed
        prior to starting. */
    void Build(VCSet& con, const Groups& groups, 
               const PatternState& patterns);
   
    /** Updates connections from oldGroups to newGroups Assumes
        existing vc data is valid for oldGroups. Logging is used if
        passed log is not 0. Breaks all connections whose carrier
        contains a new stone unless a 1-connection of player color and
        p is the key; these are upgraded to 0-connections for player
        p.  */
    void Build(VCSet& cons, const Groups& oldGroups,
               const Groups& newGroups, const PatternState& patterns,
               bitset_t added[BLACK_AND_WHITE],
               ChangeLog<VC>* log);

private:

    //----------------------------------------------------------------------

    /** Queue of endpoint pairs that need processing. 
        @ref workqueue
    */
    class WorkQueue
    {
    public:
        WorkQueue();
        bool empty() const;
        const HexPointPair& front() const;
        std::size_t capacity() const;

        void clear();
        void pop();
        void push(const HexPointPair& pair);

    private:
        std::size_t m_head;
        std::vector<HexPointPair> m_array;
        bool m_seen[BITSETSIZE][BITSETSIZE];
    };
    
    //----------------------------------------------------------------------

    /** The types of VC to create when using the AND rule. */
    typedef enum { CREATE_FULL, CREATE_SEMI } AndRule;

    void doAnd(HexPoint from, HexPoint over, HexPoint to,
               AndRule rule, const VC& vc, const bitset_t& capturedSet,
               const VCList* old);

    class OrRule 
    {
    public:
        OrRule(const VCBuilder& builder) 
            : m_builder(builder), m_semi(64), m_tail(64) {};

        int operator()(const VC& vc, const VCList* semi_list, 
                       VCList* full_list, std::list<VC>& added, 
                       int max_ors, ChangeLog<VC>* log, 
                       VCBuilderStatistics& stats);

    private:
        const VCBuilder& m_builder;
        /** Vectors used in or rule computation are reused between
            calls to avoid unnecessary dynamic memory allocation. */
        std::vector<VC> m_semi;
        std::vector<bitset_t> m_tail;
    };

    OrRule m_orRule;

    void andClosure(const VC& vc);

    void doCrossingRule(const VC& vc, const VCList* semi_list);

    void DoSearch();

    void ProcessSemis(HexPoint xc, HexPoint yc);

    void ProcessFulls(HexPoint p1, HexPoint p2);

    bool AddNewFull(const VC& vc);
    
    bool AddNewSemi(const VC& vc);

    void LoadCapturedSetPatterns();

    void ComputeCapturedSets(const PatternState& patterns);

    void AddBaseVCs();

    void AddPatternVCs();

    void AbsorbMergeShrinkUpgrade(const bitset_t& added_black,
                                  const bitset_t& added_white);

    void Merge(const Groups& oldGroups, bitset_t added[BLACK_AND_WHITE]);

    void MergeAndShrink(const bitset_t& affected,
                        const bitset_t& added);
    
    void MergeAndShrink(const bitset_t& added, 
                        HexPoint xin, HexPoint yin,
                        HexPoint xout, HexPoint yout);
    
    void RemoveAllContaining(const Groups& groups, const bitset_t& bs);
    
    //-----------------------------------------------------------------------

    VCBuilderParam& m_param;

    WorkQueue m_queue;

    VCBuilderStatistics m_statsForColor[BLACK_AND_WHITE];

    VCBuilderStatistics* m_statistics;

    const Groups* m_groups;

    const StoneBoard* m_brd;

    VCSet* m_con;
    
    HexColor m_color;

    ChangeLog<VC>* m_log;

    bitset_t m_capturedSet[BITSETSIZE];

    PatternSet m_capturedSetPatterns[BLACK_AND_WHITE];
    
    HashedPatternSet m_hash_capturedSetPatterns[BLACK_AND_WHITE];
};

inline VCBuilderParam& VCBuilder::Parameters()
{
    return m_param;
}

inline const VCBuilderParam& VCBuilder::Parameters() const
{
    return m_param;
}

inline VCBuilderStatistics VCBuilder::Statistics(HexColor color) const
{
    return m_statsForColor[color];
}

inline void VCBuilder::ClearStatistics()
{
    m_statsForColor[BLACK] = VCBuilderStatistics();
    m_statsForColor[WHITE] = VCBuilderStatistics();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCBUILDER_HPP
