//----------------------------------------------------------------------------
// $Id: Connections.hpp 1795 2008-12-15 02:24:07Z broderic $
//----------------------------------------------------------------------------

#ifndef CONNECTIONS_HPP
#define CONNECTIONS_HPP

#include "Hex.hpp"
#include "VC.hpp"
#include "VCList.hpp"

class ConstBoard;
template<typename T> class ChangeLog;

//----------------------------------------------------------------------------

/** Stores the connections for a board and color. */
class Connections
{
public:

    /** Creates a Connections class on the given board size for
        color. */
    Connections(const ConstBoard& brd, HexColor color);

    /** Copy constructor. */
    Connections(const Connections& other);

    /** Destructor. */
    virtual ~Connections();

    //------------------------------------------------------------------------
    
    /** Returns the color of this set of connections. */
    HexColor Color() const;

    /** Returns the board the set is defined on. */
    const ConstBoard& Board() const;

    /** Returns soft limit for the given type of VC. This affects
        ConnectionBuilder's performance! */
    int SoftLimit(VC::Type) const;

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
    void VCs(HexPoint x, HexPoint y, VC::Type type,
             std::vector<VC>& out) const;

    //------------------------------------------------------------------------

    /** @name Modifying methods */
    // @{

    /** See SoftLimit() */
    void SetSoftLimit(VC::Type, int limit);

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
    void operator=(const Connections& other);

    /** Returns true if other is isomorphic to us. */
    bool operator==(const Connections& other) const;

    /** Returns true if other is not isomorphic to us. */
    bool operator!=(const Connections& other) const;

    // @}

private:

    /** Allocates space for, and copies lists from, the VCLists in
        other. */
    void AllocateAndCopyLists(const Connections& other);

    /** Frees all allocated VCLists. */
    void FreeLists();

    /** @see Board() */
    const ConstBoard* m_brd;

    /** @see Color() */
    HexColor m_color;

    /** The lists of vcs. 
        @todo use actual boardsize instead of BITSETSIZE? 
    */
    VCList* m_vc[VC::NUM_TYPES][BITSETSIZE][BITSETSIZE];

};

inline HexColor Connections::Color() const
{
    return m_color;
}

inline const ConstBoard& Connections::Board() const
{
    return *m_brd;
}

inline const VCList& 
Connections::GetList(VC::Type type, HexPoint x, HexPoint y) const
{
    return *m_vc[type][x][y];
}

inline VCList& 
Connections::GetList(VC::Type type, HexPoint x, HexPoint y)
{
    return *m_vc[type][x][y];
}

inline 
VCList::AddResult Connections::Add(const VC& vc, ChangeLog<VC>* log)
{
    return m_vc[vc.type()][vc.x()][vc.y()]->add(vc, log);
}

inline int Connections::SoftLimit(VC::Type type) const
{
    return m_vc[type]
        [HexPointUtil::colorEdge1(m_color)]
        [HexPointUtil::colorEdge2(m_color)]->softlimit();
}

//----------------------------------------------------------------------------

class GroupBoard;

/** Utilities on Connections. */
namespace ConUtil 
{
    
    /** Returns set of cells connected to x. */
    bitset_t ConnectedTo(const Connections& con, const GroupBoard& brd, 
                         HexPoint x, VC::Type type);

    /** Number of connections defined on groupset. */
    void NumActiveConnections(const Connections& con, const GroupBoard& brd, 
                              int& fulls, int& semis);

    /** Returns true if connection sets are equal on the given
        groups. */
    bool EqualOnGroups(const Connections& c1, const Connections& c2,
                       const GroupBoard& brd);
}

//----------------------------------------------------------------------------

#endif // CONNECTIONS_HPP
