//----------------------------------------------------------------------------
/** @file VC.cpp */
//----------------------------------------------------------------------------

#include <sstream>
#include "VC.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VC::VC()
    : m_x(INVALID_POINT),
      m_y(INVALID_POINT), 
      m_key(VC::NO_KEY),
      m_carrier(),
      m_rule(VC_RULE_BASE),
      m_processed(false),
      m_count(0)
{
}

VC::VC(HexPoint x, HexPoint y)
    : m_x(std::min(x,y)),
      m_y(std::max(x,y)), 
      m_key(VC::NO_KEY),
      m_carrier(),
      m_rule(VC_RULE_BASE),
      m_processed(false),
      m_count(0)
{
}

VC::VC(HexPoint x, HexPoint y, const bitset_t& carrier, VcCombineRule rule)
    : m_x(std::min(x,y)),
      m_y(std::max(x,y)),
      m_key(VC::NO_KEY),
      m_carrier(carrier),
      m_rule(rule),
      m_processed(false),
      m_count(static_cast<byte>(carrier.count()))
{
}

VC::VC(HexPoint x, HexPoint y, HexPoint key, const bitset_t& carrier, 
       VcCombineRule rule)
    : m_x(std::min(x,y)),
      m_y(std::max(x,y)),
      m_key(key),
      m_carrier(carrier),
      m_rule(rule),
      m_processed(false),
      m_count(static_cast<byte>(carrier.count()))
{
    if (type() == VC::SEMI)
        BenzeneAssert(m_carrier.test(key));
}

std::string VC::toString() const
{
    std::ostringstream os;
    os << std::setw(6) << this->x();
    os << std::setw(6) << this->y();
    os << std::setw(6) << VCTypeUtil::toString(this->type());
    os << std::setw(7) << this->rule();

    os << " [";
    os << HexPointUtil::ToString(this->carrier());
    os << " ]";

    os << " ["; // Removed stones: leave this so the gui doesn't break
    os << " ]";

    if (this->type() == VC::SEMI)
        os << " " << this->key();

    return os.str();
}

//----------------------------------------------------------------------------

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2)
{
    BenzeneAssert((v1.carrier() & v2.carrier()).none());
    return VC(x, y, v1.carrier() | v2.carrier(), VC_RULE_AND);
}

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2,
              const bitset_t& capturedSet)
{
    BenzeneAssert(BitsetUtil::IsSubsetOf(v1.carrier() & v2.carrier(),
                                     capturedSet));
    return VC(x, y, v1.carrier() | v2.carrier() | capturedSet, VC_RULE_AND);
}

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2, HexPoint key)
{
    BenzeneAssert((v1.carrier() & v2.carrier()).none());
    return VC(x, y, key, (v1.carrier() | v2.carrier()).set(key), VC_RULE_AND);
}

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2, 
              const bitset_t& capturedSet, HexPoint key)
{
    BenzeneAssert(BitsetUtil::IsSubsetOf(v1.carrier() & v2.carrier(),
                                     capturedSet));
    return VC(x, y, key, (v1.carrier() | v2.carrier() | capturedSet).set(key), 
              VC_RULE_AND);
}

VC VC::UpgradeSemi(const VC& v1, const bitset_t& takeout,
                   HexPoint outx, HexPoint outy)
{
    BenzeneAssert(v1.key() != NO_KEY);
    BenzeneAssert(takeout.test(v1.key()));
    return VC(outx, outy, v1.carrier() - takeout, VC_RULE_AND);
}

VC VC::ShrinkFull(const VC& v1, const bitset_t& takeout,
                  HexPoint outx, HexPoint outy)
{
    BenzeneAssert(v1.key() == NO_KEY);
    BenzeneAssert((v1.carrier() & takeout).any());
    return VC(outx, outy, v1.carrier() - takeout, v1.rule());
}

VC VC::ShrinkSemi(const VC& v1, const bitset_t& takeout,
                  HexPoint outx, HexPoint outy)
{
    BenzeneAssert(v1.key() != NO_KEY);
    BenzeneAssert(!takeout.test(v1.key()));
    BenzeneAssert((v1.carrier() & takeout).any());
    return VC(outx, outy, v1.key(), v1.carrier() - takeout, v1.rule());
}

//----------------------------------------------------------------------------

bool VCTypeUtil::IsValidType(VC::Type type)
{
    return (type == VC::FULL || type == VC::SEMI); 
}

std::string VCTypeUtil::toString(VC::Type type)
{
    BenzeneAssert(IsValidType(type));
    if (type == VC::FULL)
        return "full";
    return "semi";
}

VC::Type VCTypeUtil::fromString(std::string name)
{
    if (name == "full") 
        return VC::FULL;
    if (name == "semi") 
        return VC::SEMI;
    std::istringstream is(name);    
    int num = 0;
#ifdef NDEBUG
    is >> num;
#else
    BenzeneAssert(is >> num);
#endif
    BenzeneAssert(num == 0 || num == 1);
    if (num == 0) 
        return VC::FULL;
    return VC::SEMI;
}

//----------------------------------------------------------------------------
