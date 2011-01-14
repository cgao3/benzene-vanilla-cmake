//----------------------------------------------------------------------------
/** @file VC.hpp */
//----------------------------------------------------------------------------

#ifndef VC_HPP
#define VC_HPP

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
        
    /** Built from an OR using all semi-connections in the
        list. */
    VC_RULE_ALL 

} VcCombineRule;

/** Utilities on VcCombineRule. */
namespace VcRuleUtil
{
    /** Returns string representation of the rule.*/
    std::string ToString(VcCombineRule rule);

} // namespace VcRuleUtil

inline std::string VcRuleUtil::ToString(VcCombineRule rule)
{
    if (rule == VC_RULE_BASE)
        return "base";
    else if (rule == VC_RULE_AND)
        return "and";
    else if (rule == VC_RULE_OR)
        return "or";
    else if (rule == VC_RULE_ALL)
        return "all";
    return "unknown";
}

/** Extends standard output operator to handle VcCombineRule. */
inline std::ostream& operator<<(std::ostream& os, VcCombineRule rule)
{
    os << VcRuleUtil::ToString(rule);
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
        VC_RULE_BASE. */
    VC(HexPoint x, HexPoint y);

    /** Creates a 0-connection between x and y with the given
        carrier. */
    VC(HexPoint x, HexPoint y, const bitset_t& carrier, VcCombineRule from);

    /** Creates a 1-connection between x and y with the given carrier
        and key. */
    VC(HexPoint x, HexPoint y, HexPoint key, const bitset_t& carrier, 
       VcCombineRule from);

    //----------------------------------------------------------------------

    /** Returns the smaller of the two endpoints. */
    HexPoint X() const;

    /** Returns the larger of the two endpoints. */
    HexPoint Y() const;

    /** Returns the key of the connection. */
    HexPoint Key() const;

    /** The set of cells required in order to realize this
        connection. */
    bitset_t Carrier() const;

    /** Returns the type of connection. */
    Type GetType() const;

    /** Returns rule used to construct this connection. */
    VcCombineRule Rule() const;

    /** Returns the number of set bits in the carrier; this is cached
        so only takes constant time. */
    int Count() const;
   
    /** Returns true if the carrier is empty, false otherwise. */
    bool IsEmpty() const;

    /** Returns string representation of connection. */
    std::string ToString() const;

    //----------------------------------------------------------------------

    /** Returns true if this vc has been processed; false, otherwise.  */
    bool Processed() const;

    /** Sets the processed flag.
        
        ONLY USE THIS IF YOU KNOW WHAT YOU ARE DOING!
    
        Should only be called inside of VCSet.
    */
    void SetProcessed(bool flag);

    //----------------------------------------------------------------------

    /** Two VCs are equal if their keys and carriers are equal. */
    bool operator==(const VC& o) const;

    bool operator!=(const VC& o) const;

    /** Comparison is by the number of set bits in the carrier. */
    bool operator<(const VC& o) const;

    bool operator>(const VC& o) const;

    bool operator<=(const VC& o) const;

    /** Is this a subset of o? */
    bool IsSubsetOf(const VC& o) const;

    //------------------------------------------------------------

    /** @name Static methods */
    // @{

    /** Returns a new full VC by unioning v1 and v2. */
    static VC AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2);

    static VC AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2,
                     const bitset_t& capturedSet);

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

    /** The rule used to construct this connection. */
    byte m_rule;

    /** Flag denoting whether this connection has been used to build
        other connections. */
    byte m_processed;

    /** Cached number of bits in the carrier. Used for sorting. */
    byte m_count;
};

inline HexPoint VC::X() const
{
    return static_cast<HexPoint>(m_x);
}

inline HexPoint VC::Y() const
{
    return static_cast<HexPoint>(m_y);
}

inline HexPoint VC::Key() const 
{
    return static_cast<HexPoint>(m_key);
}

inline bitset_t VC::Carrier() const
{
    return m_carrier;
}

inline VC::Type VC::GetType() const
{
    return (m_key == NO_KEY) ? FULL : SEMI;
}

inline VcCombineRule VC::Rule() const
{
    return static_cast<VcCombineRule>(m_rule);
}

inline int VC::Count() const
{ 
    return m_count;
}

inline bool VC::IsEmpty() const
{
    return m_carrier.none();
}

inline bool VC::Processed() const
{
    return m_processed;
}

inline void VC::SetProcessed(bool flag)
{
    m_processed = flag;
}

inline bool VC::operator==(const VC& o) const
{ 
    return (m_key == o.m_key) && (m_carrier == o.m_carrier);
}

inline bool VC::operator!=(const VC& o) const
{
    return !(*this == o);
}

inline bool VC::operator<(const VC& o) const
{
    if (Count() != o.Count())
        return (Count() < o.Count());
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
    if (*this == o) 
        return true;
    if (*this < o) 
        return true;
    return false;
}

inline bool VC::IsSubsetOf(const VC& o) const
{
    return BitsetUtil::IsSubsetOf(m_carrier, o.m_carrier);
}

//----------------------------------------------------------------------------

/** Misc. utilities on VCs. */
namespace VCTypeUtil 
{
    bool IsValidType(VC::Type type);

    std::string ToString(VC::Type type);

    VC::Type FromString(const std::string& name);
}

//----------------------------------------------------------------------------

/** Extends standard output operator to print vcs. */
inline std::ostream& operator<<(std::ostream &os, const VC& vc)
{
    os << vc.ToString();
    return os;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif  // VC_HPP
