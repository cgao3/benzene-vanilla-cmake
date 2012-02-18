//----------------------------------------------------------------------------
/** @file VCS.hpp */
//----------------------------------------------------------------------------

#ifndef VCS_HPP
#define VCS_HPP

#include "SgSystem.h"
#include "SgStatistics.h"
#include "Groups.hpp"
#include "Hex.hpp"
#include "BitsetMap.hpp"
#include "PatternState.hpp"
#include "Queue.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Settings for VCBuilder. */
struct VCBuilderParam
{
    /** Whether the and-rule can and over the edge or not.
     *        This results in many more connections. */
    bool and_over_edge;

    /** Whether to augment VC set with pre-computed VC patterns. */
    bool use_patterns;

    /** Whether to use pre-computed patterns between two non-edge
     *        cells. These can cause an explosion in the number of
     *        connections. */
    bool use_non_edge_patterns;

    /** Whether requests for incremental bulilds are done incrementally */
    bool incremental_builds;

    /** Whether limits acceptance of new vcs to
     *  those which reduce intersection of vcs. */
    bool limit;

    /** Constructor. */
    VCBuilderParam();
};

//----------------------------------------------------------------------------

class CarrierList
{
public:
    CarrierList();

    int Count() const;
    bool IsEmpty() const;

private:
    struct Elem
    {
        Elem() { }
        Elem(bitset_t carrier) : carrier(carrier), old(false) { }
        bitset_t carrier;
        bool old;
    };
    mutable std::vector<Elem> m_list;

public:
    /** Iterates over a CarrierList. */
    class Iterator : public SafeBool<Iterator>
    {
    public:
        /** Creates iterator on a CarrierList. */
        explicit Iterator(const CarrierList& lst);

        /** Returns current carrier. */
        bitset_t Carrier() const;

        /** Returns whether carrier is marked old */
        bool Old() const;

        /** Moves to the next carrier. */
        void operator++();

        /** Used by SafeBool. */
        bool boolean_test() const;

    private:
        const std::vector<Elem>& m_lst;
        std::size_t m_index;
    };

    friend class Iterator;

    bitset_t GetGreedyUnion() const;
    bool SupersetOfAny(bitset_t carrier) const;
    bool RemoveSupersetsOfCheckAnyRemoved(const CarrierList& filter);
    bitset_t GetAllIntersection() const;
    bitset_t GetOldIntersection() const;
    bitset_t GetNewIntersection() const;

    size_t RemoveAllContaining(bitset_t set);
    size_t RemoveAllContaining(bitset_t set, std::vector<bitset_t>& removed);

    void RemoveSupersetsOfUnchecked(bitset_t carrier);

protected:
    CarrierList(bitset_t carrier);
    CarrierList(const std::vector<bitset_t>& carriers_list);

    void AddNew(bitset_t carrier);

    bool RemoveSupersetsOfCheckOldRemoved(bitset_t carrier);
    bool RemoveSupersetsOfCheckAnyRemoved(bitset_t carrier);

    bool TrySetOld(bitset_t carrier) const;

    void MarkAllOld();
    void MarkAllNew();

    void Clear();

private:
    template <bool check_old>
    bool RemoveSupersetsOf(bitset_t carrier);
    template <bool store_removed>
    size_t RemoveAllContaining_(bitset_t set, std::vector<bitset_t>* removed);
};

//----------------------------------------------------------------------------

inline CarrierList::Iterator::Iterator(const CarrierList& lst)
    : m_lst(lst.m_list),
      m_index(0)
{
}

inline bitset_t CarrierList::Iterator::Carrier() const
{
    return m_lst[m_index].carrier;
}

inline bool CarrierList::Iterator::Old() const
{
    return m_lst[m_index].old;
}

inline void CarrierList::Iterator::operator++()
{
    ++m_index;
}

inline bool CarrierList::Iterator::boolean_test() const
{
    return m_index < m_lst.size();
}

inline bool CarrierList::IsEmpty() const
{
    return m_list.empty();
}

inline int CarrierList::Count() const
{
    return int(m_list.size());
}

//----------------------------------------------------------------------------

/** Stores, builds and operates on virtual connections. */
class VCS
{
public:
    /** Builder interface */
    // @{

    /** Constructor. */
    VCS(HexColor color);

    /** Copy constructor. */
    VCS(const VCS& other);

    /** Computes connections from scratch. */
    void Build(VCBuilderParam& param,
               const Groups& groups, const PatternState& patterns);

    /** Updates connections from oldGroups to newGroups Assumes
     *        existing vc data is valid for oldGroups. Breaks all connections
     *        whose carrier contains a new stone unless a 1-connection of
     *        player color and p is the key; these are upgraded to
     *        0-connections for player p. */
    void Build(VCBuilderParam& param,
               const Groups& oldGroups, const Groups& newGroups,
               const PatternState& patterns,
               bitset_t added[BLACK_AND_WHITE], bool use_changelog);

    /** Reverts last incremental build. */
    void Revert();

    // @}

    /** Calls used by solver/players staff */
    // @{

    /** Returns the greedy union of semis connecting the edges.
        It is needed for max proof set.
        @todo Is it equivalent to finding single smallest full VC?
        In fact, instead of propagating proof set,
        we should find all full VCs and then propagate all of them. */
    bitset_t GetSmallestSemisUnion() const;

    /** Tries to set the smallest carrier of full VC connecting edges.
        Returns false if there is none. */
    bool SmallestFullCarrier(bitset_t& carrier) const;

    /** returns -1 if no full vc, 0 if there is empty carrier vc,
        or 1 if there is a vc but non-empty. */
    int FullAdjacent(HexPoint x, HexPoint y) const;

    /** Tries to set the smallest carrier of semi VC connecting edges.
        Returns false if there is none. */
    bool SmallestSemiCarrier(bitset_t& carrier) const;

    /** Tries to get a key of the smallest carrier of semi VC
        connecting edges. Returns INVALID_POINT if there is none. */
    HexPoint SmallestSemiKey() const;

    bool FullExists() const;

    bool FullExists(HexPoint x, HexPoint y) const;

    bool SemiExists() const;

    /** Needed for endgame play */
    const CarrierList& GetFullCarriers() const;
    const CarrierList& GetSemiCarriers() const;

    bitset_t SemiIntersection() const;

    /** Needed for decomosition. */
    const CarrierList& GetFullCarriers(HexPoint x, HexPoint y) const;

    // @}

    /** Display commands */
    // @{

    template <class Stream>
    void DumpFulls(Stream& os, HexPoint x, HexPoint y) const;

    template <class Stream>
    void DumpSemis(Stream& os, HexPoint x, HexPoint y) const;

    template <class Stream>
    void DumpDataStats(Stream& os, int maxConnections, int numBins) const;

    template <class Stream>
    void DumpBuildStats(Stream& os) const;

    bitset_t GetFullNbs(HexPoint x) const;
    bitset_t GetSemiNbs(HexPoint x) const;
    bitset_t FullIntersection(HexPoint x, HexPoint y) const;
    bitset_t FullGreedyUnion(HexPoint x, HexPoint y) const;
    bitset_t SemiIntersection(HexPoint x, HexPoint y) const;
    bitset_t SemiGreedyUnion(HexPoint x, HexPoint y) const;

    // @}

private:

    /** Statistics for the last call to Build(). */
    struct Statistics
    {
        /** Base connections built. */
        std::size_t base_attempts;

        /** Base connections successfully added. */
        std::size_t base_successes;

        /** Pattern connections that match the board. */
        std::size_t pattern_attempts;

        /** Pattern connections successfully added. */
        std::size_t pattern_successes;

        /** Full-connections built by and-rule. */
        std::size_t and_full_attempts;

        /** Full-connections successfully added by and-rule. */
        std::size_t and_full_successes;

        /** Semi-connections built by and-rule. */
        std::size_t and_semi_attempts;

        /** Semi-connections successfully added by and-rule. */
        std::size_t and_semi_successes;

        /** Semi-connections built by and-rule. */
        std::size_t order2_attempts;

        /** Semi-connections successfully added by and-rule. */
        std::size_t order2_successes;

        /** Full-connections built by or-rule. */
        std::size_t or_attempts;

        /** Full-connections successfully added by or-rule. */
        std::size_t or_successes;

        /** Calls to or-rule. */
        std::size_t doOrs;

        /** Successfull or-rule calls -- at least one full-connection
         *        successfully added by this call. */
        std::size_t goodOrs;

        /** Fulls shrunk in merge phase. */
        std::size_t shrunk0;

        /** Semis shrunk in merge phase. */
        std::size_t shrunk1;

        /** Semis upgraded to fulls in merge phase. */
        std::size_t upgraded;

        /** Fulls killed by opponent stones in merge phase. */
        std::size_t killed0;

        /** Semis killed by opponent stones in merge phase. */
        std::size_t killed1;

        /** Dumps statistics to a string. */
        std::string ToString() const;
    };

    struct Ends
    {
        HexPoint x;
        HexPoint y;

        Ends(HexPoint x, HexPoint y);
    };

    struct Full : public Ends
    {
        bitset_t carrier;

        Full(HexPoint x, HexPoint y, bitset_t carrier);
    };

    struct Semi : public Full
    {
        HexPoint key;

        Semi(HexPoint x, HexPoint y, bitset_t carrier, HexPoint key);
    };

    class AndList : public CarrierList
    {
    public:
        AndList();
        AndList(bitset_t carrier);
        AndList(const std::vector<bitset_t>& carriers_list);
        bool TryAdd(bitset_t carrier, bool limit);
        void Add(bitset_t carrier);
        bitset_t GetIntersection() const;
        bool RemoveSupersetsOf(bitset_t carrier);

        bool TrySetProcessed(bitset_t carrier);
        void MarkAllUnprocessed();
        void CalcIntersection();
    private:
        bitset_t m_processed_intersection;
    };

    class OrList : public CarrierList
    {
    public:
        OrList();
        OrList(bitset_t carrier);
        OrList(const CarrierList& carrier_list, bitset_t intersection);

        bool TryAdd(bitset_t carrier);
        bool TryAdd(bitset_t carrier, const CarrierList& filter);

        bitset_t GetIntersection() const;

        bool RemoveSupersetsOf(bitset_t carrier);
        bool RemoveSupersetsOf(const CarrierList& filter);

        bool TryQueue(bitset_t capturedSet);

        void MarkAllProcessed();
        void MarkAllUnprocessed();
        void CalcIntersection();

    protected:
        void Clear();

    private:
        bitset_t m_intersection;
        bool m_queued;
    };

    BitsetUPairMap<AndList> m_fulls;
    BitsetUPairMap<OrList> m_semis;

    Queue<Full> m_fulls_and_queue;
    Queue<Ends> m_semis_or_queue;

    const StoneBoard* m_brd;
    HexColor m_color;
    HexPoint m_edge1, m_edge2;
    const Groups* m_groups;

    VCBuilderParam *m_param;
    Statistics m_statistics;

    bitset_t m_capturedSet[BITSETSIZE];
    PatternSet m_capturedSetPatterns[BLACK_AND_WHITE];
    HashedPatternSet m_hash_capturedSetPatterns[BLACK_AND_WHITE];

    void LoadCapturedSetPatterns();
    void ComputeCapturedSets(const PatternState& patterns);
    void AddBaseVCs();
    void AddPatternVCs();

    /** Clears connections and statistics for the from scratch build. */
    void Reset();

    void TestQueuesEmpty();

    void DoSearch();

    /** And rule staff */
    // @{

    struct VCAnd
    {

    #define VCAND_DEFFUNC(__name)\
        template <class S>\
        void __name();\
        struct Functor##__name\
        {\
            template <class S>\
            void Apply(VCS::VCAnd& do_and)\
            {\
                do_and.__name<S>();\
            }\
        };
        enum Pointer { P_NONE, P_NULL, P_SET };

        template <Pointer fulls_p, Pointer semis_p>
        struct State
        {
            static const bool fulls_initialized = fulls_p != P_NONE;
            static const bool fulls_null = fulls_p != P_SET;
            static const bool semis_initialized = semis_p != P_NONE;
            static const bool semis_null = semis_p != P_SET;
            typedef State<P_SET, semis_p> FullsSet;
            typedef State<P_NULL, semis_p> FullsNull;
            typedef State<fulls_p, P_SET> SemisSet;
            typedef State<fulls_p, P_NULL> SemisNull;
        };

        typedef State<P_NONE, P_NONE> InitialState;

        VCS& vcs;
        HexPoint x;
        HexPoint y;
        bitset_t capturedSet;
        AndList* fulls;
        OrList* semis;
        bitset_t xz_carrier;
        bitset_t intersection;
        CarrierList::Iterator zy_iter;

        VCAnd(VCS& vcs, HexPoint x, HexPoint y, bitset_t capturedSet,
              bitset_t xz_carrier, CarrierList *zy_list);

        template <class Func>
        void Run(Func func);

        template <class S, class Func>
        void SemiRemoveSupersetsOf(bitset_t carrier, Func func);

        template <class S, class Func>
        void TryAddFull(bitset_t carrier, Func func);

        template <class S, class Func>
        void TryAddSemi(bitset_t carrier, Func func);

        VCAND_DEFFUNC(FEF)
        VCAND_DEFFUNC(FEFNext)

        VCAND_DEFFUNC(FSF)
        VCAND_DEFFUNC(FSFNext)
    };

    void AndFull(HexPoint x, HexPoint y, bitset_t carrier);
    void AndFull(HexPoint x, HexPoint z, bitset_t carrier,
                 HexColor zcolor, bitset_t xzCapturedSet);

    void AndFullEmptyFull(HexPoint x, HexPoint z, bitset_t carrier,
                          bitset_t xzCapturedSet);
    void AndFullEmptyFull(HexPoint x, HexPoint z, HexPoint y, bitset_t carrier,
                          bitset_t xyCapturedSet);

    void AndFullStoneFull(HexPoint x, HexPoint z, bitset_t carrier,
                          bitset_t xzCapturedSet);
    void AndFullStoneFull(HexPoint x, HexPoint z, HexPoint y,
                          bitset_t carrier, bitset_t xyCapturedSet);

    // @}

    void OrSemis(HexPoint x, HexPoint y);

    bool TryAddFull(HexPoint x, HexPoint y, bitset_t carrier);

    /** Incremental build staff */
    // @{
    void RemoveAllContaining(const Groups& oldGroups, bitset_t removed);
    void Merge(const Groups& oldGroups, bitset_t* oldCaptureSet,
               bitset_t added[BLACK_AND_WHITE]);
    void MergeAndShrink(bitset_t* oldCaptureSet, bitset_t added,
                        bitset_t x_merged, bitset_t y_merged,
                        HexPoint x, HexPoint y);
    void MergeRemoveSelfEnds(bitset_t x_merged);
    bool Shrink(bitset_t added, HexPoint x, HexPoint y, AndList* fulls);
    bool Shrink(bitset_t added, OrList* semis, const AndList* filter);
    bool Shrink(bitset_t added, HexPoint x, HexPoint y,
                AndList* fulls, const CarrierList& list);
    bool Shrink(bitset_t added, OrList* semis, const CarrierList& list,
                const AndList* filter);

    class Backup
    {
    public:
        void Create(VCS& vcs);
        void Restore(VCS& vcs);

    private:
        BitsetUPairMap<AndList>::Backup fulls;
        BitsetUPairMap<OrList>::Backup semis;
    };
    friend class Backup;

    std::vector<Backup> backups;
    // @}
};

template <class Stream>
void VCS::DumpFulls(Stream& os, HexPoint x, HexPoint y) const
{
    BenzeneAssert(false /* stub */);
}

template <class Stream>
void VCS::DumpSemis(Stream& os, HexPoint x, HexPoint y) const
{
    BenzeneAssert(false /* stub */);
}

template <class Stream>
void VCS::DumpDataStats(Stream& os, int maxConnections, int numBins) const
{
    BenzeneAssert(false /* stub */);
}

template <class Stream>
void VCS::DumpBuildStats(Stream& os) const
{
    BenzeneAssert(false /* stub */);
}

//---------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCS_HPP
