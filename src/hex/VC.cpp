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
    if (GetType() == VC::SEMI)
        BenzeneAssert(m_carrier.test(key));
}

std::string VC::ToString() const
{
    std::ostringstream os;
    os << std::setw(6) << this->X()
       << std::setw(6) << this->Y()
       << std::setw(6) << VCTypeUtil::ToString(this->GetType())
       << std::setw(7) << this->Rule()
       << " ["
       << HexPointUtil::ToString(this->Carrier())
       << " ]"
       << " [" // Removed stones: leave this so the gui doesn't break
       << " ]";
    if (this->GetType() == VC::SEMI)
        os << " " << this->Key();
    return os.str();
}

//----------------------------------------------------------------------------

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2)
{
    BenzeneAssert((v1.Carrier() & v2.Carrier()).none());
    return VC(x, y, v1.Carrier() | v2.Carrier(), VC_RULE_AND);
}

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2,
              const bitset_t& capturedSet)
{
    BenzeneAssert(BitsetUtil::IsSubsetOf(v1.Carrier() & v2.Carrier(),
                                     capturedSet));
    return VC(x, y, v1.Carrier() | v2.Carrier() | capturedSet, VC_RULE_AND);
}

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2, HexPoint key)
{
    BenzeneAssert((v1.Carrier() & v2.Carrier()).none());
    return VC(x, y, key, (v1.Carrier() | v2.Carrier()).set(key), VC_RULE_AND);
}

VC VC::AndVCs(HexPoint x, HexPoint y, const VC& v1, const VC& v2, 
              const bitset_t& capturedSet, HexPoint key)
{
    BenzeneAssert(BitsetUtil::IsSubsetOf(v1.Carrier() & v2.Carrier(),
                                     capturedSet));
    return VC(x, y, key, (v1.Carrier() | v2.Carrier() | capturedSet).set(key), 
              VC_RULE_AND);
}

VC VC::UpgradeSemi(const VC& v1, const bitset_t& takeout,
                   HexPoint outx, HexPoint outy)
{
    BenzeneAssert(v1.Key() != NO_KEY);
    BenzeneAssert(takeout.test(v1.Key()));
    return VC(outx, outy, v1.Carrier() - takeout, VC_RULE_AND);
}

VC VC::ShrinkFull(const VC& v1, const bitset_t& takeout,
                  HexPoint outx, HexPoint outy)
{
    BenzeneAssert(v1.Key() == NO_KEY);
    BenzeneAssert((v1.Carrier() & takeout).any());
    return VC(outx, outy, v1.Carrier() - takeout, v1.Rule());
}

VC VC::ShrinkSemi(const VC& v1, const bitset_t& takeout,
                  HexPoint outx, HexPoint outy)
{
    BenzeneAssert(v1.Key() != NO_KEY);
    BenzeneAssert(!takeout.test(v1.Key()));
    BenzeneAssert((v1.Carrier() & takeout).any());
    return VC(outx, outy, v1.Key(), v1.Carrier() - takeout, v1.Rule());
}

//----------------------------------------------------------------------------

bool VCTypeUtil::IsValidType(VC::Type type)
{
    return (type == VC::FULL || type == VC::SEMI); 
}

std::string VCTypeUtil::ToString(VC::Type type)
{
    BenzeneAssert(IsValidType(type));
    if (type == VC::FULL)
        return "full";
    return "semi";
}

VC::Type VCTypeUtil::FromString(const std::string& name)
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
