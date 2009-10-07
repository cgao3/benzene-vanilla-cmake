//----------------------------------------------------------------------------
/** @file VC.hpp
 */
//----------------------------------------------------------------------------

#ifndef VC_H
#define VC_H

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Rules used to combine VCs. */
typedef enum 
{ 
    /** Empty connection between two adjacent cells. */
    VC_RULE_BASE,

    /** Built from two connections with empty intersection sharing
        an endpoint. */
    VC_RULE_AND, 
    
    /** Built from a set of two or more semi-connections whose
        interestion is empty. */
    VC_RULE_OR, 
        
    /** Semi-connection built using the crossing rule. */
    VC_RULE_CROSSING, 
    
    /** Built from an OR using all semi-connections in the
        list. */
    VC_RULE_ALL 

} VcCombineRule;

/** Utilities on VcCombineRule. */
namespace VcRuleUtil
{
    /** Returns string representation of the rule.*/
    inline std::string toString(VcCombineRule rule)
    {
        if (rule == VC_RULE_BASE)
            return "base";
        else if (rule == VC_RULE_AND)
            return "and";
        else if (rule == VC_RULE_OR)
            return "or";
        else if (rule == VC_RULE_ALL)
            return "all";
        else if (rule == VC_RULE_CROSSING)
            return "crossing";
        return "unknown";
    }

} // namespace VcRuleUtil

/** Extends standout output operator to handle VcCombineRule. */
inline std::ostream& operator<<(std::ostream& os, VcCombineRule rule)
{
    os << VcRuleUtil::toString(rule);
    return os;
}

//----------------------------------------------------------------------------


/** Virtual Connection. */
class VC
{  
public:
    
    /** Two types of Virtual VCSet: FULL and SEMI. 
        
        FULL (or "0") connections are second-player strategies
        guaranteeing the connection even if the opponent goes first.
        FULL must have their key set to NO_KEY.
        
        SEMI (or "1") connections are first-player strategies; ie, the
        first player can make the connection if he is given one free
        move.  This free move is called the "key" of the connection.
     */
    typedef enum { FULL, SEMI, NUM_TYPES } Type;


    /** Full connections must have their keys set to NO_KEY. */
    static const HexPoint NO_KEY = INVALID_POINT;

    //----------------------------------------------------------------------

    /** Constucts and empty VC with endpoints are (INVALID_POINT,
        INVALID_POINT). */
    VC();

    /** Creates an empty VC between x and y: no key, empty carrier,
        empty stones, VC_RULE_BASE. */
    VC(HexPoint x, HexPoint y);

    /** Creates a 0-connection between x and y with the given
        carrier (stones are empty). */
    VC(HexPoint x, HexPoint y, const bitset_t& carrier, VcCombineRule from);

    /** Creates a 0-connection between x and y with the given carrier
        and stones. */
    VC(HexPoint x, HexPoint y, const bitset_t& carrier, 
       const bitset_t& stones, VcCombineRule from);

    /** Creates a 1-connection between x and y with the given carrier,
        stones, and key. */
    VC(HexPoint x, HexPoint y, HexPoint key, const bitset_t& carrier, 
       const bitset_t& stones, VcCombineRule from);

    //----------------------------------------------------------------------

    /** Returns the smaller of the two endpoints. */
    HexPoint x() const;

    /** Returns the larger of the two endpoints. */
    HexPoint y() const;

    /** Returns the key of the connection. */
    HexPoint key() const;

    /** The set of cells required in order to realize this
        connection. */
    bitset_t carrier() const;

    /** Returns the set of stones required (outside the carrier) in
        order to realize this vc. */
    bitset_t stones() const;

    /** Returns the type of connection. */
    Type type() const;

    /** Returns rule used to construct this connection. */
    VcCombineRule rule() const;

    /** Returns the number of set bits in the carrier; this is cached
        so only takes constant time. */
    int count() const;
   
    /** Returns true if the carrier is empty, false otherwise. */
    bool IsEmpty() const;

    /** Returns string representation of connection. */
    std::string toString() const;

    //----------------------------------------------------------------------

    /** Returns true if this vc has been processed; false, otherwise.  */
    bool processed() const;

    /** Sets the processed flag. 
        
        ONLY USE THIS IF YOU KNOW WHAT YOU ARE DOING!  
    
        Should only be called inside of VCSet.
    */
    void setProcessed(bool flag);

    //----------------------------------------------------------------------

    /** Two VCs are equal if their keys, carriers, and stones are
        equal. */
    bool operator==(const VC& o) const;
    bool operator!=(const VC& o) const;

    /** Comparison is by the number of set bits in the carrier. */
    bool operator<(const VC& o) const;
    bool operator>(const VC& o) const;
    bool operator<=(const VC& o) const;

    /** Is this a subset of o? */
    bool isSubsetOf(const VC& o) const;

    //------------------------------------------------------------

    /** @name Static methods */
    // @{

    /** Returns a copy of vc with endpoints x and y. */
    static VC Translate(HexPoint x, HexPoint y, const VC& vc);

    /** Returns a new full VC by unioning v1 and v2. */
    static VC AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2,
                     const bitset_t& stones);

    static VC AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2,
                     const bitset_t& capturedSet, const bitset_t& stones);

    /** Returns a new semi VC with key key by unioning v1 and v2. */
    static VC AndVCs(HexPoint x, HexPoint y, 
                     const VC& v1, const VC& v2, HexPoint key);

    static VC AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2, 
                     const bitset_t& capturedSet, HexPoint key);

    static VC UpgradeSemi(const VC& v1, const bitset_t& takeout,
                          HexPoint outx, HexPoint outy);
    
    static VC ShrinkFull(const VC& v1, const bitset_t& takeout,
                         HexPoint outx, HexPoint outy);
    
    static VC ShrinkSemi(const VC& v1, const bitset_t& takeout,
                         HexPoint outx, HexPoint outy);
    // @}

private:

    /** The smaller of the two endpoints. */
    short m_x;

    /** The larger of the two endpoints. */
    short m_y;
    
    /** The connection key; if NO_KEY, then this is a FULL connection,
        otherwise this is a SEMI connectin. */
    short m_key;

    /** The empty cells that may be required to realize this
        connection. */
    bitset_t m_carrier;

    /** The MustUse set. */
    bitset_t m_stones;

    /** The rule used to construct this connection. */
    byte m_rule;

    /** Flag denoting whether this connection has been used to build
        other connections. */
    byte m_processed;

    /** Cached number of bits in the carrier. Used for sorting. */
    byte m_count;
};

inline HexPoint VC::x() const
{
    return static_cast<HexPoint>(m_x);
}

inline HexPoint VC::y() const
{
    return static_cast<HexPoint>(m_y);
}

inline HexPoint VC::key() const 
{
    return static_cast<HexPoint>(m_key);
}

inline bitset_t VC::carrier() const
{
    return m_carrier;
}

inline bitset_t VC::stones() const
{
    return m_stones;
}

inline VC::Type VC::type() const
{
    return (m_key == NO_KEY) ? FULL : SEMI;
}

inline VcCombineRule VC::rule() const
{
    return static_cast<VcCombineRule>(m_rule);
}

inline int VC::count() const
{ 
    return m_count;
}

inline bool VC::IsEmpty() const
{
    return m_carrier.none();
}

inline bool VC::processed() const
{
    return m_processed;
}

inline void VC::setProcessed(bool flag)
{
    m_processed = flag;
}

inline bool VC::operator==(const VC& o) const
{ 
    return (m_key == o.m_key) 
        && (m_carrier == o.m_carrier) 
        && (m_stones == o.m_stones);
}

inline bool VC::operator!=(const VC& o) const
{
    return !(*this == o);
}

inline bool VC::operator<(const VC& o) const
{
    if (count() != o.count())
        return (count() < o.count());

    if (m_key != o.m_key)
        return (m_key < o.m_key);

    return BitsetUtil::IsLessThan(m_carrier, o.m_carrier);
}

inline bool VC::operator>(const VC& o) const
{
    return (o < *this);
}

inline bool VC::operator<=(const VC& o) const
{
    if (*this == o) return true;
    if (*this < o) return true;
    return false;
}

inline bool VC::isSubsetOf(const VC& o) const
{
    return BitsetUtil::IsSubsetOf(m_carrier, o.m_carrier);
}

//----------------------------------------------------------------------------

/** Misc. utilities on VCs. */
namespace VCTypeUtil 
{
    bool IsValidType(VC::Type type);
    std::string toString(VC::Type type);
    VC::Type fromString(std::string name);
};

//----------------------------------------------------------------------------

/** Extends standard output operator to print vcs. */
inline std::ostream& operator<<(std::ostream &os, const VC& vc)
{
    os << vc.toString();
    return os;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif  // VC_H
