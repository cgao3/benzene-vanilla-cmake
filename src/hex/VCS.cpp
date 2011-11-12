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

inline VCS::CarrierList::CarrierList()
{
}

inline VCS::CarrierList::CarrierList(bitset_t carrier)
    : m_list(1, carrier)
{
}

inline VCS::CarrierList::CarrierList(const std::vector<bitset_t>& list)
    : m_list(list)
{
}

inline void VCS::CarrierList::Add(bitset_t carrier)
{
    m_list.push_back(carrier);
}

bool VCS::CarrierList::SupersetOfAny(bitset_t carrier) const
{
    for (std::size_t i = 0; i < m_list.size(); ++i)
        if (BitsetUtil::IsSubsetOf(m_list[i], carrier))
        {
            // Move to front
            bitset_t c = m_list[i];
            for (std::size_t j = i; j > 0; j--)
                m_list[j] = m_list[j - 1];
            m_list[0] = c;
            return true;
        }
        return false;
}

bool VCS::CarrierList::RemoveSupersetsOf(bitset_t carrier)
{
    std::size_t j = 0;
    std::size_t i = 0;
    for (; i < m_list.size(); ++i)
    {
        if (!BitsetUtil::IsSubsetOf(carrier, m_list[i]))
            m_list[j++] = m_list[i];
    }
    m_list.resize(j);
    return j < i;
}

bool VCS::CarrierList::Contains(bitset_t carrier) const
{
    for (Iterator i(*this); i; ++i)
        if (*i == carrier)
            return true;
    return false;
}

inline const std::vector<bitset_t>& VCS::CarrierList::GetVec() const
{
    return m_list;
}

inline int VCS::CarrierList::Count() const
{
    return int(m_list.size());
}

bitset_t VCS::CarrierList::GetGreedyUnion() const
{
    bitset_t U;
    bitset_t I;
    I.set();
    for (Iterator i(*this); i; ++i)
    {
        if ((I & *i) != I)
        {
            I &= *i;
            U |= *i;
        }
    }
    return U;
}

bitset_t VCS::CarrierList::GetIntersection() const
{
    bitset_t I;
    I.set();
    for (Iterator i(*this); i; ++i)
        I &= *i;
    return I;
}

void VCS::CarrierList::MoveAll(CarrierList& other)
{
    std::size_t size = m_list.size();
    m_list.resize(size + other.m_list.size());
    for (std::size_t i = 0; i < other.m_list.size(); i++)
        m_list[size + i] = other.m_list[i];
    other.m_list.clear();
}

//----------------------------------------------------------------------------

inline VCS::CarrierList::Iterator::Iterator(const CarrierList& lst)
    : m_lst(lst.m_list),
      m_index(0)
{
}

inline bitset_t VCS::CarrierList::Iterator::operator*() const
{
    return m_lst[m_index];
}

inline const bitset_t* VCS::CarrierList::Iterator::operator->() const
{
    return &m_lst[m_index];
}

inline void VCS::CarrierList::Iterator::operator++()
{
    ++m_index;
}

inline bool VCS::CarrierList::Iterator::boolean_test() const
{
    return m_index < m_lst.size();
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
    : m_carriers(carrier)
{
    m_processed_intersection.set();
}

inline VCS::AndList::AndList(const std::vector<bitset_t>& carriers_list)
    : m_carriers(carriers_list)
{
    m_processed_intersection.set();
}

inline void VCS::AndList::Add(bitset_t carrier)
{
    RemoveSupersetsOf(carrier);
    m_carriers.Add(carrier);
}

inline bool VCS::AndList::TryAdd(bitset_t carrier)
{
    if (SupersetOfAny(carrier))
        return false;
    Add(carrier);
    return true;
}

inline void VCS::AndList::RemoveSupersetsOf(bitset_t carrier)
{
    if (m_carriers.RemoveSupersetsOf(carrier))
    {
        if (m_processed.RemoveSupersetsOf(carrier))
            m_processed_intersection = m_processed.GetIntersection();
    }
}

inline bool VCS::AndList::SupersetOfAny(bitset_t carrier) const
{
    return m_carriers.SupersetOfAny(carrier);
}

inline bitset_t VCS::AndList::GetIntersection() const
{
    return m_processed_intersection;
}

inline const VCS::CarrierList& VCS::AndList::ProcessedCarriers() const
{
    return m_processed;
}

inline void VCS::AndList::AddProcessed(bitset_t carrier)
{
    m_processed.Add(carrier);
    m_processed_intersection &= carrier;
}

inline bool VCS::AndList::StillExists(bitset_t carrier) const
{
    return m_carriers.Contains(carrier);
}

inline const VCS::CarrierList& VCS::AndList::GetAll() const
{
    return m_carriers;
}

//----------------------------------------------------------------------------

inline VCS::SemiList::SemiList(bitset_t carrier, HexPoint key)
    : m_intersection(carrier),
      m_new(carrier),
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

template <bool fulls_null>
inline bool VCS::SemiList::TryAdd(bitset_t carrier, HexPoint key, AndList* fulls)
{
    AndList* &key_semis = m_lists[key];
    if (key_semis)
    {
        if (key_semis->SupersetOfAny(carrier))
            return false;
        if (!fulls_null)
        {
            if (fulls->SupersetOfAny(carrier))
                return false;
        }
        key_semis->Add(carrier);
    }
    else
    {
        if (!fulls_null)
        {
            if (fulls->SupersetOfAny(carrier))
                return false;
        }
        key_semis = new AndList(carrier);
        m_keys.set(key);
    }
    if (m_old.SupersetOfAny(carrier))
        return true;
    if (m_new.SupersetOfAny(carrier))
        return true;
    m_old.RemoveSupersetsOf(carrier);
    m_new.RemoveSupersetsOf(carrier);
    m_new.Add(carrier);
    m_intersection &= carrier;
    return true;
}

inline void VCS::SemiList::Set(HexPoint key, AndList* list)
{
    m_keys.set(key);
    m_lists[key] = list;
}

inline void VCS::SemiList::Add(bitset_t carrier)
{
    if (m_old.SupersetOfAny(carrier))
        return;
    if (m_new.SupersetOfAny(carrier))
        return;
    m_old.RemoveSupersetsOf(carrier);
    m_new.RemoveSupersetsOf(carrier);
    m_new.Add(carrier);
    m_intersection &= carrier;
}

inline void VCS::SemiList::RemoveSupersetsOf(bitset_t carrier)
{
    bool old_res = m_old.RemoveSupersetsOf(carrier);
    bool new_res = m_new.RemoveSupersetsOf(carrier);
    if (!old_res && !new_res)
        return;
    m_intersection = m_old.GetIntersection() & m_new.GetIntersection();
    for (BitsetIterator it(m_keys); it; ++it)
        m_lists[*it]->RemoveSupersetsOf(carrier);
}

inline bool VCS::SemiList::TryQueue(bitset_t capturedSet)
{
    if (m_new.Count() == 0)
        return false;
    bool prev_queued = m_queued;
    m_queued = BitsetUtil::IsSubsetOf(m_intersection, capturedSet);
    return !prev_queued && m_queued;
}

void VCS::SemiList::MarkAllOld()
{
    m_old.MoveAll(m_new);
    m_queued = false;
}

inline const VCS::CarrierList& VCS::SemiList::GetNew() const
{
    return m_new;
}

inline const VCS::CarrierList& VCS::SemiList::GetOld() const
{
    return m_old;
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
#define CALL(__S,__name) return __name.template Apply<__S>(*this)

inline VCS::VCAnd::VCAnd(VCS& vcs, HexPoint x, HexPoint y, bitset_t capturedSet,
                         bitset_t xz_carrier, AndList *zy_list, HexPoint key)
    : vcs(vcs), x(x), y(y), capturedSet(capturedSet),
      xz_carrier(xz_carrier), key(key), zy_iter(zy_list->ProcessedCarriers())
{
}

template <class Func>
inline void VCS::VCAnd::Run(Func func)
{
    CALL(InitialState, func);
}

template <class S, class Func>
inline void VCS::VCAnd::SemiRemoveSupersetsOf(bitset_t carrier, Func func)
{
    if (!S::semis_initialized)
    {
        semis = vcs.m_semis[x].Get(y);
        if (semis)
            SemiRemoveSupersetsOf<typename S::SemisSet>(carrier, func);
        else
            SemiRemoveSupersetsOf<typename S::SemisNull>(carrier, func);
        return;
    }
    if (!S::semis_null)
        semis->RemoveSupersetsOf(carrier);
    CALL(S, func);
}

template <class S, class FuncYes, class FuncNo>
inline void VCS::VCAnd::TryAddFull(bitset_t carrier, FuncYes funcYes, FuncNo funcNo)
{
    if (!S::fulls_initialized)
    {
        fulls = vcs.m_fulls[x].Get(y);
        if (fulls)
            TryAddFull<typename S::FullsSet>(carrier, funcYes, funcNo);
        else
            TryAddFull<typename S::FullsNull>(carrier, funcYes, funcNo);
        return;
    }
    if (S::fulls_null)
    {
        fulls = vcs.m_fulls[x].Add(y, carrier);
        vcs.m_fulls[y].Set(x, fulls);
    }
    else if (!fulls->TryAdd(carrier))
        CALL(S, funcNo);
    vcs.m_fulls_and_queue.Push(Full(x, y, carrier));
    SemiRemoveSupersetsOf<typename S::FullsSet>(carrier, funcYes);
}

template <class S, class FuncYes, class FuncNo>
inline void VCS::VCAnd::TryAddSemi(bitset_t carrier, FuncYes funcYes, FuncNo funcNo)
{
    if (!S::semis_initialized)
    {
        semis = vcs.m_semis[x].Get(y);
        if (semis)
            TryAddSemi<typename S::SemisSet>(carrier, funcYes, funcNo);
        else
            TryAddSemi<typename S::SemisNull>(carrier, funcYes, funcNo);
        return;
    }
    if (!S::fulls_initialized)
    {
        fulls = vcs.m_fulls[x].Get(y);
        if (fulls)
            TryAddSemi<typename S::FullsSet>(carrier, funcYes, funcNo);
        else
            TryAddSemi<typename S::FullsNull>(carrier, funcYes, funcNo);
        return;
    }
    if (S::semis_null)
    {
        vcs.m_statistics.and_semi_attempts++;
        if (!S::fulls_null)
        {
            if (fulls->SupersetOfAny(carrier))
                CALL(S, funcNo);
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
                TryAddSemi<typename S::KeySemisSet>(carrier, funcYes, funcNo);
            else
                TryAddSemi<typename S::KeySemisNull>(carrier, funcYes, funcNo);
            return;
        }
        vcs.m_statistics.and_semi_attempts++;
        if (S::key_semis_null)
        {
            if (!S::fulls_null)
            {
                if (fulls->SupersetOfAny(carrier))
                    CALL(S, funcNo);
            }
            key_semis = new AndList(carrier);
            semis->Set(key, key_semis);
        }
        else
        {
            if (key_semis->SupersetOfAny(carrier))
                CALL(S, funcNo);
            if (!S::fulls_null)
            {
                if (fulls->SupersetOfAny(carrier))
                    CALL(S, funcNo);
            }
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
    CALL(typename S::SemisSet::KeySemisSet, funcYes);
}

template <class S, class FuncYes, class FuncNo>
inline void VCS::VCAnd::TryAddSemi(bitset_t carrier, HexPoint new_key,
                            FuncYes funcYes, FuncNo funcNo)
{
    if (new_key != key)
    {
        key = new_key;
        TryAddSemi<typename S::KeySemisReset>(carrier, funcYes, funcNo);
    }
    else
        TryAddSemi<S>(carrier, funcYes, funcNo);
}

void VCS::AndFull(HexPoint x, HexPoint y, bitset_t carrier)
{
    BenzeneAssert(x == m_groups->CaptainOf(x));
    BenzeneAssert(y == m_groups->CaptainOf(y));
    BenzeneAssert(x != y);
    BenzeneAssert(m_brd->GetColor(x) != !m_color);
    BenzeneAssert(m_brd->GetColor(y) != !m_color);

    if (!m_fulls[x].Get(y)->StillExists(carrier))
        return;

    bitset_t xyCapturedSet = m_capturedSet[x] | m_capturedSet[y];
    AndFull(x, y, carrier, m_brd->GetColor(y), xyCapturedSet);
    AndFull(y, x, carrier, m_brd->GetColor(x), xyCapturedSet);

    m_fulls[x].Get(y)->AddProcessed(carrier);
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
    if (zy_iter->test(x))
        return FEFNext<S>();
    intersection = xz_carrier & *zy_iter;
    if (intersection.none())
        return TryAddSemi<S>((xz_carrier | *zy_iter).set(key),
                             FUNC(FEFNext), FUNC(FEFNext));
    FEFCaptured<S>();
}

template <class S>
inline void VCS::VCAnd::FEFCaptured()
{
    if (BitsetUtil::IsSubsetOf(intersection, capturedSet))
        return TryAddSemi<S>((xz_carrier | *zy_iter | capturedSet).set(key),
                             FUNC(FEFNext), FUNC(FEFNext));
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
    if (zy_iter->test(x))
        return FSFNext<S>();
    intersection = xz_carrier & *zy_iter;
    BitsetIterator it(intersection);
    if (!it /* intersection is empty */)
        return TryAddFull<S>(xz_carrier | *zy_iter,
                                FUNC(FSFNext), FUNC(FSFNext));
    HexPoint new_key = *it;
    ++it;
    if (!it /* intersection is singleton */)
        return TryAddSemi<S>(xz_carrier | *zy_iter, new_key,
                             FUNC(FSFCaptured), FUNC(FSFCaptured));
    FSFCaptured<S>();
}

template <class S>
inline void VCS::VCAnd::FSFCaptured()
{
    BitsetIterator it(intersection - capturedSet);
    if (!it /* intersection is empty */)
        return TryAddFull<S>(xz_carrier | *zy_iter | capturedSet,
                             FUNC(FSFNext), FUNC(FSFNext));
    HexPoint new_key = *it;
    ++it;
    if (!it /* intersection is singleton */)
        return TryAddSemi<S>(xz_carrier | *zy_iter | capturedSet, new_key,
                             FUNC(FSFNext), FUNC(FSFNext));
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
    
    AndList* xy_fulls = m_fulls[x].Get(y);
    if (xy_fulls)
        AndFullStoneSemi<false>(x, y, key, carrier, xyCapturedSet,
                                zy_list, xy_fulls);
    else
        AndFullStoneSemi<true>(x, y, key, carrier, xyCapturedSet,
                               zy_list, xy_fulls);
}

template <bool xy_fulls_null>
inline void VCS::AndFullStoneSemi(HexPoint x, HexPoint y, HexPoint key,
                                  bitset_t carrier, bitset_t xyCapturedSet,
                                  AndList* zy_list, AndList* xy_fulls)
{
    SemiList* xy_semis = m_semis[x].Get(y);
    CarrierList::Iterator i(zy_list->ProcessedCarriers());
    if (xy_semis)
        AndFullStoneSemiIterate<xy_fulls_null, false>
                                (x, y, key, carrier, xyCapturedSet, i,
                                 xy_fulls, xy_semis);
    else
        AndFullStoneSemiIterate<xy_fulls_null, true>
                               (x, y, key, carrier, xyCapturedSet, i,
                                xy_fulls, xy_semis);
}

template <bool xy_fulls_null, bool xy_semis_null>
inline void VCS::AndFullStoneSemiIterate(HexPoint x, HexPoint y, HexPoint key,
                                         bitset_t carrier, bitset_t xyCapturedSet,
                                         CarrierList::Iterator i,
                                         AndList* xy_fulls, SemiList* xy_semis)
{
    if (!i)
        return;
    bool new_semi = TryAndFullStoneSemi<xy_fulls_null, xy_semis_null>
                                       (x, y, key, carrier, *i, xyCapturedSet,
                                        xy_fulls, xy_semis);
    ++i;
    if (xy_semis_null && !new_semi)
        AndFullStoneSemiIterate<xy_fulls_null, true>
                               (x, y, key, carrier, xyCapturedSet, i,
                                xy_fulls, xy_semis);
    else
        AndFullStoneSemiIterate<xy_fulls_null, false>
                               (x, y, key, carrier, xyCapturedSet, i,
                                xy_fulls, xy_semis);
}

template <bool xy_fulls_null, bool xy_semis_null>
inline bool VCS::TryAndFullStoneSemi(HexPoint x, HexPoint y, HexPoint key,
                                     bitset_t xz_carrier, bitset_t zy_carrier,
                                     bitset_t xyCapturedSet,
                                     AndList* xy_fulls, SemiList* &xy_semis)
{
    if (zy_carrier.test(x))
        return false;
    bitset_t intersection = (xz_carrier & zy_carrier).reset(key);
    if (intersection.none())
    {
        m_statistics.and_semi_attempts++;
        if (TryAddSemi<xy_fulls_null, xy_semis_null>
            (x, y, xz_carrier | zy_carrier, key,
             xyCapturedSet, xy_fulls, xy_semis))
        {
            m_statistics.and_semi_successes++;
            return true;
        }
    }
    else if (BitsetUtil::IsSubsetOf(intersection, xyCapturedSet))
    {
        m_statistics.and_semi_attempts++;
        if (TryAddSemi<xy_fulls_null, xy_semis_null>
            (x, y, xz_carrier | zy_carrier | xyCapturedSet, key,
             xyCapturedSet, xy_fulls, xy_semis))
        {
            m_statistics.and_semi_successes++;
            return true;
        }
    }
    return false;
}

void VCS::AndSemi(HexPoint x, HexPoint y, HexPoint key, bitset_t carrier)
{
    BenzeneAssert(x == m_groups->CaptainOf(x));
    BenzeneAssert(y == m_groups->CaptainOf(y));
    BenzeneAssert(x != y);
    BenzeneAssert(m_brd->GetColor(x) != !m_color);
    BenzeneAssert(m_brd->GetColor(y) != !m_color);

    if (!m_semis[x].Get(y)->Get(key)->StillExists(carrier))
        return;

    bitset_t xyCapturedSet = m_capturedSet[x] | m_capturedSet[y];
    AndSemi(x, y, key, carrier, m_brd->GetColor(y), xyCapturedSet);
    AndSemi(y, x, key, carrier, m_brd->GetColor(x), xyCapturedSet);
    
    m_semis[x].Get(y)->Get(key)->AddProcessed(carrier);
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
        VCOr(xy_semis->GetNew().GetVec(), xy_semis->GetOld().GetVec(),
             xy_fulls ? xy_fulls->GetAll().GetVec() : std::vector<bitset_t>(),
             m_capturedSet[x] | m_capturedSet[y]);
    xy_semis->MarkAllOld();
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

inline void VCS::SemiRemoveSupersetsOf(HexPoint x, HexPoint y, bitset_t carrier)
{
    SemiList* semis = m_semis[x].Get(y);
    if (semis)
        semis->RemoveSupersetsOf(carrier);
}

template <bool fulls_null>
inline bool VCS::TryAddFull(HexPoint x, HexPoint y, bitset_t carrier,
                            AndList* &fulls)
{
    if (fulls_null)
    {
        fulls = m_fulls[x].Add(y, carrier);
        m_fulls[y].Set(x, fulls);
    }
    else if (!fulls->TryAdd(carrier))
        return false;
    m_fulls_and_queue.Push(Full(x, y, carrier));
    SemiRemoveSupersetsOf(x, y, carrier);
    return true;
}

inline bool VCS::TryAddFull(HexPoint x, HexPoint y, bitset_t carrier)
{
    AndList* fulls = m_fulls[x].Get(y);
    if (fulls)
        return TryAddFull<false>(x, y, carrier, fulls);
    else
        return TryAddFull<true>(x, y, carrier, fulls);
}

template <bool fulls_null, bool semis_null>
inline bool VCS::TryAddSemi(HexPoint x, HexPoint y, bitset_t carrier, HexPoint key,
                            bitset_t xyCapturedSet, AndList* fulls, SemiList* &semis)
{
    if (semis_null)
    {
        if (!fulls_null)
        {
            if (fulls->SupersetOfAny(carrier))
                return false;
        }
        semis = m_semis[x].Add(y, carrier, key);
        m_semis[y].Set(x, semis);
    }
    else if (!semis->TryAdd<fulls_null>(carrier, key, fulls))
        return false;
    if (semis->TryQueue(xyCapturedSet))
        m_semis_or_queue.Push(Ends(x, y));
    if ((m_brd->GetColor(x) != EMPTY &&
         (m_param->and_over_edge || !HexPointUtil::isEdge(x)))
        ||
        (m_brd->GetColor(y) != EMPTY &&
         (m_param->and_over_edge || !HexPointUtil::isEdge(y))))
        m_semis_and_queue.Push(Semi(x, y, carrier, key));
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
    if (fulls->GetAll().Count() == 0)
        return false;
    size_t best = std::numeric_limits<size_t>::max();
    for (CarrierList::Iterator i(fulls->GetAll()); i; ++i)
    {
        size_t count = i->count();
        if (count < best)
        {
            best = count;
            carrier = *i;
        }
    }
    return true;
}

int VCS::FullAdjacent(HexPoint x, HexPoint y) const
{
    AndList* fulls = m_fulls[x].Get(y);
    if (!fulls)
        return -1;
    if (fulls->GetAll().Count() == 0)
        return -1;
    CarrierList::Iterator i(fulls->GetAll());
    return i->any() ? 1 : 0;
}

bool VCS::SmallestSemiCarrier(bitset_t& carrier) const
{
    SemiList* semis = m_semis[m_edge1].Get(m_edge2);
    if (!semis)
        return false;
    bool res = false;
    size_t best = std::numeric_limits<size_t>::max();
    for (CarrierList::Iterator i(semis->GetOld()); i; ++i)
    {
        size_t count = i->count();
        if (count < best)
        {
            best = count;
            carrier = *i;
        }
        res = true;
    }
    for (CarrierList::Iterator i(semis->GetNew()); i; ++i)
    {
        size_t count = i->count();
        if (count < best)
        {
            best = count;
            carrier = *i;
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
        for (CarrierList::Iterator i(semis->Get(key)->GetAll()); i; ++i)
        {
            size_t count = i->count();
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
    return fulls->GetAll().Count() != 0;
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
    return semis->GetOld().Count() != 0 || semis->GetNew().Count() != 0;
}

static std::vector<bitset_t> empty_vec;

const std::vector<bitset_t>& VCS::GetFullCarriers(HexPoint x, HexPoint y) const
{
    AndList* fulls = m_fulls[x].Get(y);
    if (!fulls)
        return empty_vec;
    return fulls->GetAll().GetVec();
}

const std::vector<bitset_t>& VCS::GetFullCarriers() const
{
    return GetFullCarriers(m_edge1, m_edge2);
}

std::vector<bitset_t> VCS::GetSemiCarriers() const
{
    SemiList* semis = m_semis[m_edge1].Get(m_edge2);
    if (!semis)
        return empty_vec;
    std::vector<bitset_t> res(semis->GetOld().GetVec());
    for (CarrierList::Iterator i(semis->GetNew()); i; ++i)
        res.push_back(*i);
    return res;
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
    return fulls ? fulls->GetAll().GetGreedyUnion() : EMPTY_BITSET;
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
    return semis ? semis->GetOld().GetGreedyUnion() : EMPTY_BITSET;
}
