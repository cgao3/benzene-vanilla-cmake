//----------------------------------------------------------------------------
/** @file VCS.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"

#include "Hex.hpp"
#include "BitsetIterator.hpp"
#include "Misc.hpp"
#include "VCS.hpp"
#include "VCOr.hpp"
#include "VCPattern.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VCBuilderParam::VCBuilderParam()
    : and_over_edge(false),
      use_patterns(true),
      use_non_edge_patterns(true)
{
}

//----------------------------------------------------------------------------

inline CarrierList::CarrierList()
{
}

inline CarrierList::CarrierList(bitset_t carrier)
    : m_list(1, carrier)
{
}

inline CarrierList::CarrierList(const std::vector<bitset_t>& carriers_list)
{
    m_list.reserve(carriers_list.size());
    for (std::size_t i = 0; i < carriers_list.size(); ++i)
        AddNew(carriers_list[i]);
}

inline void CarrierList::AddNew(bitset_t carrier)
{
    m_list.push_back(carrier);
}

inline bool CarrierList::SupersetOfAny(bitset_t carrier) const
{
    for (std::size_t i = 0; i < m_list.size(); ++i)
        if (BitsetUtil::IsSubsetOf(m_list[i].carrier, carrier))
        {
            // Move to front
            Elem c = m_list[i];
            for (std::size_t j = i; j > 0; j--)
                m_list[j] = m_list[j - 1];
            m_list[0] = c;
            return true;
        }
        return false;
}

template <bool check_old>
inline bool CarrierList::RemoveSupersetsOf(bitset_t carrier)
{
    std::size_t j = 0;
    std::size_t i = 0;
    bool check_old_res = false;
    for (; i < m_list.size(); ++i)
    {
        if (!BitsetUtil::IsSubsetOf(carrier, m_list[i].carrier))
            m_list[j++] = m_list[i];
        else if (check_old)
        {
            if (m_list[i].old)
                check_old_res = true;
        }
    }
    m_list.resize(j);
    if (check_old)
        return check_old_res;
    else
        return j < i;
}

inline bool CarrierList::RemoveSupersetsOfCheckOldRemoved(bitset_t carrier)
{
    return RemoveSupersetsOf<true>(carrier);
}

inline bool CarrierList::RemoveSupersetsOfCheckAnyRemoved(bitset_t carrier)
{
    return RemoveSupersetsOf<false>(carrier);
}

inline void CarrierList::RemoveSupersetsOfUnchecked(bitset_t carrier)
{
    RemoveSupersetsOf<false>(carrier);
}

inline bool CarrierList::TrySetOld(bitset_t carrier) const
{
    for (std::size_t i = 0; i < m_list.size(); ++i)
        if (m_list[i].carrier == carrier)
        {
            m_list[i].old = true;
            return true;
        }
    return false;
}

bitset_t CarrierList::GetGreedyUnion() const
{
    bitset_t U;
    bitset_t I;
    I.set();
    for (Iterator i(*this); i; ++i)
    {
        if ((I & i.Carrier()) != I)
        {
            I &= i.Carrier();
            U |= i.Carrier();
        }
    }
    return U;
}

template <bool only_old>
inline bitset_t CarrierList::GetIntersection() const
{
    bitset_t I;
    I.set();
    for (Iterator i(*this); i; ++i)
        if (!only_old || i.Old())
            I &= i.Carrier();
    return I;
}

inline bitset_t CarrierList::GetOldIntersection() const
{
    return GetIntersection<true>();
}

inline bitset_t CarrierList::GetAllIntersection() const
{
    return GetIntersection<false>();
}

void CarrierList::MarkAllOld()
{
    for (std::size_t i = 0; i < m_list.size(); i++)
        m_list[i].old = true;
}

//----------------------------------------------------------------------------

inline VCS::Ends::Ends(HexPoint x, HexPoint y)
    : x(x), y(y)
{
}

inline VCS::Full::Full(HexPoint x, HexPoint y, bitset_t carrier)
    : Ends(x, y), carrier(carrier)
{
}

inline VCS::Semi::Semi(HexPoint x, HexPoint y, bitset_t carrier, HexPoint key)
    : Full(x, y, carrier), key(key)
{
}

//----------------------------------------------------------------------------

inline VCS::AndList::AndList(bitset_t carrier)
    : CarrierList(carrier)
{
    m_processed_intersection.set();
}

inline VCS::AndList::AndList(const std::vector<bitset_t>& carriers_list)
    : CarrierList(carriers_list)
{
    m_processed_intersection.set();
}

inline void VCS::AndList::RemoveSupersetsOf(bitset_t carrier)
{
    if (RemoveSupersetsOfCheckOldRemoved(carrier))
        m_processed_intersection = GetOldIntersection();
}

inline void VCS::AndList::Add(bitset_t carrier)
{
    RemoveSupersetsOf(carrier);
    AddNew(carrier);
}

inline bool VCS::AndList::TryAdd(bitset_t carrier)
{
    if (SupersetOfAny(carrier))
        return false;
    Add(carrier);
    return true;
}

inline bitset_t VCS::AndList::GetIntersection() const
{
    return m_processed_intersection;
}

inline bool VCS::AndList::TrySetProcessed(bitset_t carrier)
{
    if (TrySetOld(carrier))
    {
        m_processed_intersection &= carrier;
        return true;
    }
    else
        return false;
}

//----------------------------------------------------------------------------

inline VCS::SemiList::SemiList(bitset_t carrier, HexPoint key)
    : CarrierList(carrier),
      m_intersection(carrier),
      m_queued(false)
{
    memset(m_lists, 0, sizeof(m_lists));
    m_keys.set(key);
    m_lists[key] = new AndList(carrier);
}

VCS::SemiList::~SemiList()
{
    for (BitsetIterator it(m_keys); it; ++it)
        delete m_lists[*it];
}

inline bitset_t VCS::SemiList::GetKeySet() const
{
    return m_keys;
}

inline VCS::AndList* VCS::SemiList::Get(HexPoint key) const
{
    return m_lists[key];
}

inline bitset_t VCS::SemiList::GetIntersection() const
{
    return m_intersection;
}

inline void VCS::SemiList::Set(HexPoint key, AndList* list)
{
    m_keys.set(key);
    m_lists[key] = list;
}

inline void VCS::SemiList::Add(bitset_t carrier)
{
    if (SupersetOfAny(carrier))
        return;
    RemoveSupersetsOfUnchecked(carrier);
    AddNew(carrier);
    m_intersection &= carrier;
}

inline void VCS::SemiList::RemoveSupersetsOf(bitset_t carrier)
{
    if (!RemoveSupersetsOfCheckAnyRemoved(carrier))
        return;
    m_intersection = GetAllIntersection();
    for (BitsetIterator it(m_keys); it; ++it)
        m_lists[*it]->RemoveSupersetsOf(carrier);
}

inline bool VCS::SemiList::TryQueue(bitset_t capturedSet)
{
    bool prev_queued = m_queued;
    m_queued = BitsetUtil::IsSubsetOf(m_intersection, capturedSet);
    return !prev_queued && m_queued;
}

void VCS::SemiList::MarkAllProcessed()
{
    MarkAllOld();
    m_queued = false;
}

//----------------------------------------------------------------------------

template <class T>
inline void VCS::Nbs<T>::ClearLists()
{
    memset(m_lists, 0, sizeof(m_lists));
}

template <class T>
inline VCS::Nbs<T>::Nbs()
{
    ClearLists();
}

template <class T>
void VCS::Nbs<T>::Destroy(HexPoint x)
{
    for (BitsetIterator y(m_set); y; ++y)
    {
        if (*y > x)
            break;
        delete m_lists[*y];
    }
}

template <class T>
void VCS::Nbs<T>::Reset(HexPoint x)
{
    Destroy(x);
    ClearLists();
    m_set.reset();
}

template <class T>
inline T* VCS::Nbs<T>::Get(HexPoint x) const
{
    return m_lists[x];
}

template <class T>
inline void VCS::Nbs<T>::Set(HexPoint x, T* list)
{
    m_set.set(x);
    m_lists[x] = list;
}

template <class T>
inline bitset_t VCS::Nbs<T>::GetNbsSet() const
{
    return m_set;
}

inline VCS::AndList* VCS::FullNbs::Add(HexPoint x, bitset_t carrier)
{
    AndList* fulls = new AndList(carrier);
    Set(x, fulls);
    return fulls;
}

inline VCS::SemiList* VCS::SemiNbs::Add(HexPoint x, bitset_t carrier, HexPoint key)
{
    SemiList* semis = new SemiList(carrier, key);
    Set(x, semis);
    return semis;
}

//---------------------------------------------------------------------------

inline void VCS::TestQueuesEmpty()
{
    BenzeneAssert(m_fulls_and_queue.IsEmpty());
    BenzeneAssert(m_semis_and_queue.IsEmpty());
    BenzeneAssert(m_semis_or_queue.IsEmpty());
}

VCS::VCS(HexColor color)
    : m_color(color),
      m_edge1(HexPointUtil::colorEdge1(color)),
      m_edge2(HexPointUtil::colorEdge2(color))
{
    LoadCapturedSetPatterns();
}

void VCS::LoadCapturedSetPatterns()
{
    std::ifstream inFile;
    try {
        std::string file = MiscUtil::OpenFile("vc-captured-set.txt", inFile);
        LogConfig() << "VCS: reading captured set patterns from '"
        << file << "'.\n";
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "VCS: " << e.what();
    }
    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromStream(inFile, patterns);
    LogConfig() << "VCS:: parsed " << patterns.size() << " patterns.\n";
    for (std::size_t i = 0; i < patterns.size(); ++i)
    {
        m_capturedSetPatterns[WHITE].push_back(patterns[i]);
        patterns[i].FlipColors();
        m_capturedSetPatterns[BLACK].push_back(patterns[i]);
    }
    for (BWIterator c; c; ++c)
        m_hash_capturedSetPatterns[*c].Hash(m_capturedSetPatterns[*c]);
}

VCS::VCS(const VCS& other)
    : m_brd(other.m_brd),
      m_color(other.m_color)
{
    BenzeneAssert(false /* TODO implement copy constructor */);
}

VCS::~VCS()
{
    for (int i = 0; i < BITSETSIZE; i++)
        m_fulls[i].Destroy(HexPoint(i));
    for (int i = 0; i < BITSETSIZE; i++)
        m_semis[i].Destroy(HexPoint(i));
}

/** Computes the 0-connections defined by adjacency.*/
void VCS::AddBaseVCs()
{
    HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(m_color);
    for (GroupIterator x(*m_groups, not_other); x; ++x)
    {
        for (BitsetIterator y(x->Nbs() & m_brd->GetEmpty()); y; ++y)
        {
            BenzeneAssert(*y == m_groups->CaptainOf(*y));
            m_statistics.base_attempts++;
            if (TryAddFull(x->Captain(), *y, EMPTY_BITSET))
                m_statistics.base_successes++;
        }
    }
}

/** Adds vcs obtained by pre-computed patterns. */
void VCS::AddPatternVCs()
{
    const VCPatternSet& patterns
    = VCPattern::GetPatterns(m_brd->Width(), m_brd->Height(), m_color);
    for (std::size_t i=0; i<patterns.size(); ++i)
    {
        const VCPattern& pat = patterns[i];
        if (!m_param->use_non_edge_patterns
            && !HexPointUtil::isEdge(pat.Endpoint(0))
            && !HexPointUtil::isEdge(pat.Endpoint(1)))
            continue;
        if (pat.Matches(m_color, *m_brd))
        {
            bitset_t carrier = pat.NotOpponent() - m_brd->GetColor(m_color);
            carrier.reset(pat.Endpoint(0));
            carrier.reset(pat.Endpoint(1));

            HexPoint x = m_groups->CaptainOf(pat.Endpoint(0));
            HexPoint y = m_groups->CaptainOf(pat.Endpoint(1));
            if (x == y)
                continue;

            m_statistics.pattern_attempts++;
            if (TryAddFull(x, y, carrier))
                m_statistics.pattern_successes++;
        }
    }
}

void VCS::ComputeCapturedSets(const PatternState& patterns)
{
    SG_UNUSED(patterns);
    for (BoardIterator p(m_brd->Const().EdgesAndInterior()); p; ++p)
    {
        m_capturedSet[*p] = EMPTY_BITSET;
        if (m_brd->GetColor(*p) == EMPTY)
        {
            PatternHits hits;
            patterns.MatchOnCell(m_hash_capturedSetPatterns[m_color],
                                 *p, PatternState::STOP_AT_FIRST_HIT, hits);
            for (std::size_t i = 0; i < hits.size(); ++i)
            {
                const std::vector<HexPoint>& moves = hits[0].Moves2();
                for (std::size_t j = 0; j < moves.size(); ++j)
                    m_capturedSet[*p].set(moves[j]);
            }
        }
    }
}

void VCS::Build(VCBuilderParam& param,
                const Groups& groups, const PatternState& patterns)
{
    TestQueuesEmpty();
    SgTimer timer;
    m_param = &param;
    m_groups = &groups;
    m_brd = &m_groups->Board();
    Reset();

    ComputeCapturedSets(patterns);
    AddBaseVCs();
    if (m_param->use_patterns)
        AddPatternVCs();
    DoSearch();

    LogFine() << "  " << timer.GetTime() << "s to build vcs.\n";
}

void VCS::Build(VCBuilderParam& param,
                const Groups& oldGroups, const Groups& newGroups,
                const PatternState& patterns,
                bitset_t added[BLACK_AND_WHITE], bool use_changelog)
{
    BenzeneAssert(false /* stub */);
}

void VCS::Revert()
{
    BenzeneAssert(false /* stub */);
}

void VCS::Reset()
{
    for (int i = 0; i < BITSETSIZE; i++)
        m_fulls[i].Reset(HexPoint(i));
    for (int i = 0; i < BITSETSIZE; i++)
        m_semis[i].Reset(HexPoint(i));

    m_statistics = Statistics();
}

void VCS::DoSearch()
{
    while (true)
    {
        if (!m_fulls_and_queue.IsEmpty())
        {
            Full vc = m_fulls_and_queue.Pop();
            AndFull(vc.x, vc.y, vc.carrier);
        }
        else if (!m_semis_and_queue.IsEmpty())
        {
            Semi vc = m_semis_and_queue.Pop();
            AndSemi(vc.x, vc.y, vc.key, vc.carrier);
        }
        else if (!m_semis_or_queue.IsEmpty())
        {
            Ends p = m_semis_or_queue.Pop();
            OrSemis(p.x, p.y);
        }
        else
            break;
    }
    TestQueuesEmpty();
}

// And rule stuff

#define FUNC(__name) VCS::VCAnd::Functor##__name()
#define SWITCHTO(__S) return func.template Apply<__S>(*this)

inline VCS::VCAnd::VCAnd(VCS& vcs, HexPoint x, HexPoint y, bitset_t capturedSet,
                         bitset_t xz_carrier, AndList *zy_list, HexPoint key)
    : vcs(vcs), x(x), y(y), capturedSet(capturedSet),
      xz_carrier(xz_carrier), key(key), zy_iter(*zy_list)
{
}

template <class Func>
inline void VCS::VCAnd::Run(Func func)
{
    SWITCHTO(InitialState);
}

template <class S, class Func>
inline void VCS::VCAnd::SemiRemoveSupersetsOf(bitset_t carrier, Func func)
{
    if (!S::semis_initialized)
    {
        semis = vcs.m_semis[x].Get(y);
        if (semis)
            return SemiRemoveSupersetsOf<typename S::SemisSet>(carrier, func);
        else
            return SemiRemoveSupersetsOf<typename S::SemisNull>(carrier, func);
    }
    if (!S::semis_null)
        semis->RemoveSupersetsOf(carrier);
    SWITCHTO(S);
}

template <class S, class Func>
inline void VCS::VCAnd::TryAddFull(bitset_t carrier, Func func)
{
    if (!S::fulls_initialized)
    {
        fulls = vcs.m_fulls[x].Get(y);
        if (fulls)
            return TryAddFull<typename S::FullsSet>(carrier, func);
        else
            return TryAddFull<typename S::FullsNull>(carrier, func);
    }
    if (S::fulls_null)
    {
        fulls = vcs.m_fulls[x].Add(y, carrier);
        vcs.m_fulls[y].Set(x, fulls);
    }
    else if (!fulls->TryAdd(carrier))
        SWITCHTO(S);
    vcs.m_fulls_and_queue.Push(Full(x, y, carrier));
    SemiRemoveSupersetsOf<typename S::FullsSet>(carrier, func);
}

template <class S, class Func>
inline void VCS::VCAnd::TryAddSemi(bitset_t carrier, Func func)
{
    if (!S::semis_initialized)
    {
        semis = vcs.m_semis[x].Get(y);
        if (semis)
            return TryAddSemi<typename S::SemisSet>(carrier, func);
        else
            return TryAddSemi<typename S::SemisNull>(carrier, func);
    }
    if (!S::fulls_initialized)
    {
        fulls = vcs.m_fulls[x].Get(y);
        if (fulls)
            return TryAddSemi<typename S::FullsSet>(carrier, func);
        else
            return TryAddSemi<typename S::FullsNull>(carrier, func);
    }
    if (S::semis_null)
    {
        vcs.m_statistics.and_semi_attempts++;
        if (!S::fulls_null)
        {
            if (fulls->SupersetOfAny(carrier))
                SWITCHTO(S);
        }
        semis = vcs.m_semis[x].Add(y, carrier, key);
        vcs.m_semis[y].Set(x, semis);
        key_semis = semis->Get(key);
    }
    else
    {
        if (!S::key_semis_initialized)
        {
            key_semis = semis->Get(key);
            if (key_semis)
                return TryAddSemi<typename S::KeySemisSet>(carrier, func);
            else
                return TryAddSemi<typename S::KeySemisNull>(carrier, func);
        }
        vcs.m_statistics.and_semi_attempts++;
        if (S::key_semis_null)
        {
            if (!S::fulls_null)
            {
                if (fulls->SupersetOfAny(carrier))
                    SWITCHTO(S);
            }
            key_semis = new AndList(carrier);
            semis->Set(key, key_semis);
        }
        else
        {
            if (!S::fulls_null)
            {
                if (fulls->SupersetOfAny(carrier))
                    SWITCHTO(S);
            }
            if (key_semis->SupersetOfAny(carrier))
                SWITCHTO(S);
            key_semis->Add(carrier);
        }
        semis->Add(carrier);
    }

    if (semis->TryQueue(capturedSet))
        vcs.m_semis_or_queue.Push(Ends(x, y));
    if ((vcs.m_brd->GetColor(x) != EMPTY &&
        (vcs.m_param->and_over_edge || !HexPointUtil::isEdge(x)))
        ||
        (vcs.m_brd->GetColor(y) != EMPTY &&
        (vcs.m_param->and_over_edge || !HexPointUtil::isEdge(y))))
        vcs.m_semis_and_queue.Push(Semi(x, y, carrier, key));

    vcs.m_statistics.and_semi_successes++;
    SWITCHTO(typename S::SemisSet::KeySemisSet);
}

template <class S, class Func>
inline void VCS::VCAnd::TryAddSemi(bitset_t carrier, HexPoint new_key,
                                   Func func)
{
    if (new_key != key)
    {
        key = new_key;
        TryAddSemi<typename S::KeySemisReset>(carrier, func);
    }
    else
        TryAddSemi<S>(carrier, func);
}

void VCS::AndFull(HexPoint x, HexPoint y, bitset_t carrier)
{
    BenzeneAssert(x == m_groups->CaptainOf(x));
    BenzeneAssert(y == m_groups->CaptainOf(y));
    BenzeneAssert(x != y);
    BenzeneAssert(m_brd->GetColor(x) != !m_color);
    BenzeneAssert(m_brd->GetColor(y) != !m_color);

    if (!m_fulls[x].Get(y)->TrySetProcessed(carrier))
        return;

    bitset_t xyCapturedSet = m_capturedSet[x] | m_capturedSet[y];
    AndFull(x, y, carrier, m_brd->GetColor(y), xyCapturedSet);
    AndFull(y, x, carrier, m_brd->GetColor(x), xyCapturedSet);
}

inline void VCS::AndFull(HexPoint x, HexPoint z, bitset_t carrier,
                         HexColor zcolor, bitset_t xzCapturedSet)
{
    if (!m_param->and_over_edge && HexPointUtil::isEdge(z))
        return;
    if (zcolor == EMPTY)
        AndFullEmptyFull(x, z, carrier, xzCapturedSet);
    else
    {
        AndFullStoneFull(x, z, carrier, xzCapturedSet);
        AndFullStoneSemi(x, z, carrier, xzCapturedSet);
    }
}

inline void VCS::AndFullEmptyFull(HexPoint x, HexPoint z, bitset_t carrier,
                                  bitset_t xzCapturedSet)
{
    for (BitsetIterator it(m_fulls[z].GetNbsSet().reset(x) - carrier); it; ++it)
    {
        HexPoint y = *it;
        BenzeneAssert(y == m_groups->CaptainOf(y));
        BenzeneAssert(x != y && z != y);
        BenzeneAssert(!carrier.test(y));
        AndFullEmptyFull(x, z, y, carrier, xzCapturedSet | m_capturedSet[y]);
    }
}

inline void VCS::AndFullEmptyFull(HexPoint x, HexPoint z, HexPoint y,
                                  bitset_t carrier, bitset_t xyCapturedSet)
{
    AndList* zy_fulls = m_fulls[z].Get(y);
    BenzeneAssert(zy_fulls);
    if (!BitsetUtil::IsSubsetOf(zy_fulls->GetIntersection() & carrier,
        xyCapturedSet))
        return;

    VCAnd vcAnd(*this, x, y, xyCapturedSet, carrier, zy_fulls, z);
    vcAnd.Run(FUNC(FEF));
}

template <class S>
inline void VCS::VCAnd::FEF()
{
    if (!zy_iter)
        return;
    if (!zy_iter.Old())
        return FEFNext<S>();
    if (zy_iter.Carrier().test(x))
        return FEFNext<S>();
    intersection = xz_carrier & zy_iter.Carrier();
    if (intersection.none())
        return TryAddSemi<S>((xz_carrier | zy_iter.Carrier()).set(key), FUNC(FEFNext));
    else if (BitsetUtil::IsSubsetOf(intersection, capturedSet))
        return TryAddSemi<S>((xz_carrier | zy_iter.Carrier() | capturedSet).set(key),
                             FUNC(FEFNext));
    FEFNext<S>();
}

template <class S>
inline void VCS::VCAnd::FEFNext()
{
    ++zy_iter;
    FEF<S>();
}

inline void VCS::AndFullStoneFull(HexPoint x, HexPoint z, bitset_t carrier,
                                  bitset_t xzCapturedSet)
{
    for (BitsetIterator it(m_fulls[z].GetNbsSet().reset(x) - carrier); it; ++it)
    {
        HexPoint y = *it;
        BenzeneAssert(y == m_groups->CaptainOf(y));
        BenzeneAssert(x != y && z != y);
        BenzeneAssert(!carrier.test(y));
        AndFullStoneFull(x, z, y, carrier, xzCapturedSet | m_capturedSet[y]);
    }
}

inline void VCS::AndFullStoneFull(HexPoint x, HexPoint z, HexPoint y,
                                  bitset_t carrier, bitset_t xyCapturedSet)
{
    AndList* zy_fulls = m_fulls[z].Get(y);
    BenzeneAssert(zy_fulls);
    if (((zy_fulls->GetIntersection() & carrier) - xyCapturedSet).count() > 1)
        return;

    VCAnd vcAnd(*this, x, y, xyCapturedSet, carrier, zy_fulls);
    vcAnd.Run(FUNC(FSF));
}

template <class S>
inline void VCS::VCAnd::FSF()
{
    if (!zy_iter)
        return;
    if (!zy_iter.Old())
        return FSFNext<S>();
    if (zy_iter.Carrier().test(x))
        return FSFNext<S>();
    intersection = xz_carrier & zy_iter.Carrier();
    BitsetIterator it(intersection);
    if (!it /* intersection is empty */)
        return TryAddFull<S>(xz_carrier | zy_iter.Carrier(), FUNC(FSFNext));
    HexPoint new_key = *it;
    ++it;
    if (!it /* intersection is singleton */)
        return TryAddSemi<S>(xz_carrier | zy_iter.Carrier(), new_key,
                             FUNC(FSFCaptured));
    FSFCaptured<S>();
}

template <class S>
inline void VCS::VCAnd::FSFCaptured()
{
    BitsetIterator it(intersection - capturedSet);
    if (!it /* intersection is empty */)
        return TryAddFull<S>(xz_carrier | zy_iter.Carrier() | capturedSet,
                             FUNC(FSFNext));
    HexPoint new_key = *it;
    ++it;
    if (!it /* intersection is singleton */)
        return TryAddSemi<S>(xz_carrier | zy_iter.Carrier() | capturedSet, new_key,
                             FUNC(FSFNext));
    FSFNext<S>();
}

template <class S>
inline void VCS::VCAnd::FSFNext()
{
    ++zy_iter;
    FSF<S>();
}

inline void VCS::AndFullStoneSemi(HexPoint x, HexPoint z, bitset_t carrier,
                                  bitset_t xzCapturedSet)
{
    for (BitsetIterator it(m_semis[z].GetNbsSet().reset(x) - carrier); it; ++it)
    {
        HexPoint y = *it;
        BenzeneAssert(y == m_groups->CaptainOf(y));
        BenzeneAssert(x != y && z != y);
        BenzeneAssert(!carrier.test(y));
        AndFullStoneSemi(x, z, y, carrier, xzCapturedSet | m_capturedSet[y]);
    }
}

inline void VCS::AndFullStoneSemi(HexPoint x, HexPoint z, HexPoint y,
                                  bitset_t carrier, bitset_t xyCapturedSet)
{
    SemiList* zy_semis = m_semis[z].Get(y);
    BenzeneAssert(zy_semis);

    for (BitsetIterator it(zy_semis->GetKeySet()); it; ++it)
    {
        HexPoint key = *it;
        AndFullStoneSemi(x, y, key, carrier, xyCapturedSet,
                         zy_semis->Get(key));
    }
}

inline void VCS::AndFullStoneSemi(HexPoint x, HexPoint y, HexPoint key,
                                  bitset_t carrier, bitset_t xyCapturedSet,
                                  AndList* zy_list)
{
    BenzeneAssert(zy_list);
    if (!BitsetUtil::IsSubsetOf((zy_list->GetIntersection() & carrier).reset(key),
        xyCapturedSet))
        return;

    VCAnd vcAnd(*this, x, y, xyCapturedSet, carrier, zy_list, key);
    vcAnd.Run(FUNC(FSS));
}


template <class S>
inline void VCS::VCAnd::FSS()
{
    if (!zy_iter)
        return;
    if (!zy_iter.Old())
        return FSSNext<S>();
    if (zy_iter.Carrier().test(x))
        return FSSNext<S>();
    bitset_t intersection = (xz_carrier & zy_iter.Carrier()).reset(key);
    if (intersection.none())
        return TryAddSemi<S>(xz_carrier | zy_iter.Carrier(), FUNC(FSSNext));
    else if (BitsetUtil::IsSubsetOf(intersection, capturedSet))
        return TryAddSemi<S>(xz_carrier | zy_iter.Carrier() | capturedSet, FUNC(FSSNext));
    FSSNext<S>();
}

template <class S>
inline void VCS::VCAnd::FSSNext()
{
    ++zy_iter;
    FSS<S>();
}

void VCS::AndSemi(HexPoint x, HexPoint y, HexPoint key, bitset_t carrier)
{
    BenzeneAssert(x == m_groups->CaptainOf(x));
    BenzeneAssert(y == m_groups->CaptainOf(y));
    BenzeneAssert(x != y);
    BenzeneAssert(m_brd->GetColor(x) != !m_color);
    BenzeneAssert(m_brd->GetColor(y) != !m_color);

    if (!m_semis[x].Get(y)->Get(key)->TrySetProcessed(carrier))
        return;

    bitset_t xyCapturedSet = m_capturedSet[x] | m_capturedSet[y];
    AndSemi(x, y, key, carrier, m_brd->GetColor(y), xyCapturedSet);
    AndSemi(y, x, key, carrier, m_brd->GetColor(x), xyCapturedSet);
}

inline void VCS::AndSemi(HexPoint x, HexPoint z, HexPoint key, bitset_t carrier,
                         HexColor zcolor, bitset_t xzCapturedSet)
{
    if (zcolor == EMPTY)
        return;
    if (!m_param->and_over_edge && HexPointUtil::isEdge(z))
        return;
    AndSemiStoneFull(x, z, key, carrier, xzCapturedSet);
}

inline void VCS::AndSemiStoneFull(HexPoint x, HexPoint z, HexPoint key,
                                  bitset_t carrier, bitset_t xzCapturedSet)
{
    for (BitsetIterator it(m_fulls[z].GetNbsSet().reset(x) - carrier); it; ++it)
    {
        HexPoint y = *it;
        BenzeneAssert(y == m_groups->CaptainOf(y));
        BenzeneAssert(x != y && z != y);
        BenzeneAssert(!carrier.test(y));
        AndFullStoneSemi(x, y, key, carrier, xzCapturedSet | m_capturedSet[y],
                         m_fulls[z].Get(y));
    }
}

void VCS::OrSemis(HexPoint x, HexPoint y)
{
    SemiList* xy_semis = m_semis[x].Get(y);
    BenzeneAssert(xy_semis);
    AndList *xy_fulls = m_fulls[x].Get(y);
    m_statistics.doOrs++;
    std::vector<bitset_t> new_fulls =
        VCOr(*xy_semis, xy_fulls ? *xy_fulls : CarrierList(),
             m_capturedSet[x] | m_capturedSet[y]);
    xy_semis->MarkAllProcessed();
    if (new_fulls.empty())
        return;
    m_statistics.goodOrs++;
    m_statistics.or_attempts += new_fulls.size();
    m_statistics.or_successes += new_fulls.size();
    if (!xy_fulls)
    {
        for (std::vector<bitset_t>::iterator it = new_fulls.begin();
             it != new_fulls.end(); ++it)
            m_fulls_and_queue.Push(Full(x, y, *it));
        xy_fulls = new AndList(new_fulls);
        m_fulls[x].Set(y, xy_fulls);
        m_fulls[y].Set(x, xy_fulls);
    }
    else
    {
        for (std::vector<bitset_t>::iterator it = new_fulls.begin();
             it != new_fulls.end(); ++it)
        {
            xy_fulls->Add(*it);
            m_fulls_and_queue.Push(Full(x, y, *it));
        }
    }
}

inline bool VCS::TryAddFull(HexPoint x, HexPoint y, bitset_t carrier)
{
    AndList* fulls = m_fulls[x].Get(y);
    if (!fulls)
    {
        fulls = m_fulls[x].Add(y, carrier);
        m_fulls[y].Set(x, fulls);
    }
    else if (!fulls->TryAdd(carrier))
        return false;
    m_fulls_and_queue.Push(Full(x, y, carrier));
    SemiList* semis = m_semis[x].Get(y);
    if (semis)
        semis->RemoveSupersetsOf(carrier);
    return true;
}

bitset_t VCS::GetSmallestSemisUnion() const
{
    return SemiGreedyUnion(m_edge1, m_edge2);
}

bool VCS::SmallestFullCarrier(bitset_t& carrier) const
{
    AndList* fulls = m_fulls[m_edge1].Get(m_edge2);
    if (!fulls)
        return false;
    if (fulls->IsEmpty())
        return false;
    size_t best = std::numeric_limits<size_t>::max();
    for (CarrierList::Iterator i(*fulls); i; ++i)
    {
        size_t count = i.Carrier().count();
        if (count < best)
        {
            best = count;
            carrier = i.Carrier();
        }
    }
    return true;
}

int VCS::FullAdjacent(HexPoint x, HexPoint y) const
{
    AndList* fulls = m_fulls[x].Get(y);
    if (!fulls)
        return -1;
    if (fulls->IsEmpty())
        return -1;
    CarrierList::Iterator i(*fulls);
    return i.Carrier().any() ? 1 : 0;
}

bool VCS::SmallestSemiCarrier(bitset_t& carrier) const
{
    SemiList* semis = m_semis[m_edge1].Get(m_edge2);
    if (!semis)
        return false;
    bool res = false;
    size_t best = std::numeric_limits<size_t>::max();
    for (CarrierList::Iterator i(*semis); i; ++i)
    {
        size_t count = i.Carrier().count();
        if (count < best)
        {
            best = count;
            carrier = i.Carrier();
        }
        res = true;
    }
    return res;
}

HexPoint VCS::SmallestSemiKey() const
{
    SemiList* semis = m_semis[m_edge1].Get(m_edge2);
    if (!semis)
        return INVALID_POINT;
    size_t best = std::numeric_limits<size_t>::max();
    HexPoint res = INVALID_POINT;
    for (BitsetIterator it(semis->GetKeySet()); it; ++it)
    {
        HexPoint key = *it;
        for (CarrierList::Iterator i(*semis->Get(key)); i; ++i)
        {
            size_t count = i.Carrier().count();
            if (count < best)
            {
                best = count;
                res = key;
            }
        }
    }
    return res;
}

bool VCS::FullExists(HexPoint x, HexPoint y) const
{
    AndList* fulls = m_fulls[x].Get(y);
    if (!fulls)
        return false;
    return !fulls->IsEmpty();
}

bool VCS::FullExists() const
{
    return FullExists(m_edge1, m_edge2);
}

bool VCS::SemiExists() const
{
    SemiList* semis = m_semis[m_edge1].Get(m_edge2);
    if (!semis)
        return false;
    return !semis->IsEmpty();
}

static CarrierList empty_carrier_list;

const CarrierList& VCS::GetFullCarriers(HexPoint x, HexPoint y) const
{
    AndList* fulls = m_fulls[x].Get(y);
    if (!fulls)
        return empty_carrier_list;
    return *fulls;
}

const CarrierList& VCS::GetFullCarriers() const
{
    return GetFullCarriers(m_edge1, m_edge2);
}

CarrierList VCS::GetSemiCarriers() const
{
    SemiList* semis = m_semis[m_edge1].Get(m_edge2);
    if (!semis)
        return CarrierList();
    return *semis;
}

bitset_t VCS::GetFullNbs(HexPoint x) const
{
    return m_fulls[x].GetNbsSet();
}

bitset_t VCS::GetSemiNbs(HexPoint x) const
{
    return m_semis[x].GetNbsSet();
}

bitset_t VCS::FullIntersection(HexPoint x, HexPoint y) const
{
    AndList* fulls = m_fulls[x].Get(y);
    return fulls ? fulls->GetIntersection() : bitset_t().set();
}

bitset_t VCS::FullGreedyUnion(HexPoint x, HexPoint y) const
{
    AndList* fulls = m_fulls[x].Get(y);
    return fulls ? fulls->GetGreedyUnion() : EMPTY_BITSET;
}

bitset_t VCS::SemiIntersection(HexPoint x, HexPoint y) const
{
    SemiList* semis = m_semis[x].Get(y);
    return semis ? semis->GetIntersection() : bitset_t().set();
}

bitset_t VCS::SemiIntersection() const
{
    return SemiIntersection(m_edge1, m_edge2);
}

bitset_t VCS::SemiGreedyUnion(HexPoint x, HexPoint y) const
{
    SemiList* semis = m_semis[x].Get(y);
    return semis ? semis->GetGreedyUnion() : EMPTY_BITSET;
}
