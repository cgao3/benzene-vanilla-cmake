//----------------------------------------------------------------------------
/** @file VCSet.hpp */
//----------------------------------------------------------------------------

#ifndef VCSET_HPP
#define VCSET_HPP

#include "SgSystem.h"
#include "SgStatistics.h"

#include "ChangeLog.hpp"
#include "ConstBoard.hpp"
#include "Groups.hpp"
#include "Hex.hpp"
#include "VC.hpp"
#include "VCList.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Stores the connections for a board and color. */
class VCSet
{
public:
    /** Creates a VCSet class on the given board size for color. */
    VCSet(const ConstBoard& brd, HexColor color);

    /** Copy constructor. */
    VCSet(const VCSet& other);

    /** Destructor. */
    virtual ~VCSet();

    //------------------------------------------------------------------------
    
    /** Returns the color of this set of connections. */
    HexColor Color() const;

    /** Returns the board the set is defined on. */
    const ConstBoard& Board() const;

    /** Returns soft limit for the given type of VC. This affects
        VCBuilder's performance! */
    std::size_t SoftLimit(VC::Type) const;

    /** Returns the VCList between (x, y). */
    const VCList& GetList(VC::Type type, HexPoint x, HexPoint y) const;
    
    /** Returns the VCList between (x, y). */
    VCList& GetList(VC::Type type, HexPoint x, HexPoint y);

    /** Determines if there is at least one valid connection between
        the given pair of cells for the color and VC type, x and y
        must both be the color of this connection set. */
    bool Exists(HexPoint x, HexPoint y, VC::Type type) const;

    /** Copies the smallest connection between x and y of type into
        out, returns true if successful. Returns false if no
        connection exists between x and y. */
    bool SmallestVC(HexPoint x, HexPoint y, VC::Type type, VC& out) const;

    /** Stores the valid connections between x and y for color in out
        (which is cleared beforehand). */
    void VCs(HexPoint x, HexPoint y, VC::Type type, std::vector<VC>& out) const;

    //------------------------------------------------------------------------

    /** @name Modifying methods */
    // @{

    /** See SoftLimit() */
    void SetSoftLimit(VC::Type, std::size_t limit);

    /** Clears the connections. */
    void Clear();

    /** Attempts to add the given vc to the list between (vc.x(),
        vc.y()). Returns result of the add operation. This method is
        just a wrapper for GetList(vc.type(), vc.x(), vc.y())->add(vc).

        @see VCList::add()
    */
    VCList::AddResult Add(const VC& vc, ChangeLog<VC>* log);

    /** Uses the given changelog to revert connections to state at 
        last marker in the changelog. Log will will have all entries
        and last marker removed. */
    void Revert(ChangeLog<VC>& log);

    // @}

    //------------------------------------------------------------------------

    /** @name Operators */
    // @{

    /** Assignment operator. */
    void operator=(const VCSet& other);

    /** Returns true if other is isomorphic to us. */
    bool operator==(const VCSet& other) const;

    /** Returns true if other is not isomorphic to us. */
    bool operator!=(const VCSet& other) const;

    // @}

private:
    /** Allocates space for, and copies lists from, the VCLists in
        other. */
    void AllocateAndCopyLists(const VCSet& other);

    /** Frees all allocated VCLists. */
    void FreeLists();

    /** @see Board() */
    const ConstBoard* m_brd;

    /** @see Color() */
    HexColor m_color;

    /** The lists of vcs. 
        @todo use actual boardsize instead of BITSETSIZE? */
    VCList* m_vc[VC::NUM_TYPES][BITSETSIZE][BITSETSIZE];
};

inline HexColor VCSet::Color() const
{
    return m_color;
}

inline const ConstBoard& VCSet::Board() const
{
    return *m_brd;
}

inline const VCList& 
VCSet::GetList(VC::Type type, HexPoint x, HexPoint y) const
{
    return *m_vc[type][x][y];
}

inline VCList& 
VCSet::GetList(VC::Type type, HexPoint x, HexPoint y)
{
    return *m_vc[type][x][y];
}

inline 
VCList::AddResult VCSet::Add(const VC& vc, ChangeLog<VC>* log)
{
    return m_vc[vc.GetType()][vc.X()][vc.Y()]->Add(vc, log);
}

inline std::size_t VCSet::SoftLimit(VC::Type type) const
{
    return m_vc[type]
        [HexPointUtil::colorEdge1(m_color)]
        [HexPointUtil::colorEdge2(m_color)]->Softlimit();
}

//----------------------------------------------------------------------------

/** Info on the set of connections. */
struct VCSetStatistics
{
    std::size_t m_fulls;

    std::size_t m_semis;

    SgStatisticsExt<float, std::size_t> m_fullCounts;
    
    SgStatisticsExt<float, std::size_t> m_semiCounts;
    
    SgStatisticsExt<float, std::size_t> m_fullCountsCell;
    
    SgStatisticsExt<float, std::size_t> m_semiCountsCell;
    
    SgStatisticsExt<float, std::size_t> m_fullConnectedTo;

    SgStatisticsExt<float, std::size_t> m_semiConnectedTo;
    
    SgHistogram<std::size_t, std::size_t> m_fullHisto;
    
    SgHistogram<std::size_t, std::size_t> m_semiHisto;

    VCSetStatistics();

    std::string Write() const;
};

//----------------------------------------------------------------------------

/** Utilities on VCSet. */
namespace VCSetUtil 
{
    /** Returns set of cells connected to x. */
    bitset_t ConnectedTo(const VCSet& con, const Groups& groups, 
                         HexPoint x, VC::Type type);

    /** Returns true if connection sets are equal on the given
        groups. */
    bool EqualOnGroups(const VCSet& c1, const VCSet& c2,
                       const Groups& groups);

    /** Obtain info on connections. */
    VCSetStatistics ComputeStatistics(const VCSet& con, const Groups& groups,
                                      std::size_t maxConnections,
                                      int numBins);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCSET_HPP
