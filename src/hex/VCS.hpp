//----------------------------------------------------------------------------
/** @file VCSet.hpp */
//----------------------------------------------------------------------------

#ifndef VCS_HPP
#define VCS_HPP

#include "SgSystem.h"
#include "SgStatistics.h"
#include "Groups.hpp"
#include "Hex.hpp"
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
    
    /** Constructor. */
    VCBuilderParam();
};

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
    
    /** Destructor. */
    ~VCS();
    
    /* FIXME: Build should clear build statistics first */
    
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

    /** @todo Needed for endgame play */
    const std::vector<bitset_t>& GetFullCarriers() const;
    std::vector<bitset_t> GetSemiCarriers() const;

    bitset_t SemiIntersection() const;
    
    /** @todo Needed for decomosition.
        Return rather CarrierList and supply it with iterator. */
    const std::vector<bitset_t>& GetFullCarriers(HexPoint x, HexPoint y) const;

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

    class CarrierList
    {
    public:
        CarrierList();
        CarrierList(bitset_t carrier);
        CarrierList(const std::vector<bitset_t>& list);
        
        void Add(bitset_t carrier);
        bool SupersetOfAny(bitset_t carrier) const;

        bool RemoveSupersetsOf(bitset_t carrier);

        const std::vector<bitset_t>& GetVec() const;
        int Count() const;

        bitset_t GetGreedyUnion() const;
        bitset_t GetIntersection() const;

        void MoveAll(CarrierList& other);
        
        /** Iterates over a CarrierList. */
        class Iterator : public SafeBool<Iterator>
        {
        public:
            /** Creates iterator on a CarrierList. */
            explicit Iterator(const CarrierList& lst);
            
            /** Returns current carrier. */
            bitset_t operator*() const;
            
            /** Allows access to current carrier's methods. */
            const bitset_t* operator->() const;
            
            /** Moves to the next carrier. */
            void operator++();
            
            /** Used by SafeBool. */
            bool boolean_test() const;
            
        private:
            const std::vector<bitset_t>& m_lst;
            std::size_t m_index;
        };

        friend class Iterator;
        
    private:
        mutable std::vector<bitset_t> m_list;
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
    
    class AndList
    {
    public:
        AndList(bitset_t carrier);
        AndList(const std::vector<bitset_t>& carriers_list);
        bool TryAdd(bitset_t carrier);
        void Add(bitset_t carrier);
        bitset_t GetIntersection() const;
        void RemoveSupersetsOf(bitset_t carrier);
        
        const CarrierList& ProcessedCarriers() const;
        void AddProcessed(bitset_t carrier);
        bool SupersetOfAny(bitset_t carrier) const;
        
        const CarrierList& GetAll() const;
    private:
        bitset_t m_processed_intersection;
        CarrierList m_processed;
        CarrierList m_carriers;
    };
    
    class SemiList
    {
    public:
        SemiList(bitset_t carrier, HexPoint key);
        ~SemiList();

        bitset_t GetKeySet() const;
        AndList* Get(HexPoint key) const;

        template <bool fulls_null>
        bool TryAdd(bitset_t carrier, HexPoint key, AndList* fulls);

        bitset_t GetIntersection() const;

        void RemoveSupersetsOf(bitset_t carrier);
        
        bool TryQueue(bitset_t capturedSet);
        void MarkAllOld();

        const CarrierList& GetNew() const;
        const CarrierList& GetOld() const;
    private:
        bitset_t m_keys;
        bitset_t m_intersection;
        CarrierList m_old;
        CarrierList m_new;
        AndList* m_lists[BITSETSIZE];
        bool m_queued;
    };

    template <class T>
    class Nbs
    {
    public:
        Nbs();
        void Destroy(HexPoint x);
        void Reset(HexPoint x);

        T* Get(HexPoint x) const;

        void Set(HexPoint x, T* list);

        bitset_t GetNbsSet() const;
    private:
        bitset_t m_set;
        T* m_lists[BITSETSIZE];

        void ClearLists();
    };
    
    class FullNbs : public Nbs<AndList>
    {
    public:
        /** Add new neighbor with given carrier */
        AndList* Add(HexPoint x, bitset_t carrier);
    };
    
    class SemiNbs : public Nbs<SemiList>
    {
    public:
        /** Add new neighbor with given carrier and key */
        SemiList* Add(HexPoint x, bitset_t carrier, HexPoint key);
    };
    
    FullNbs m_fulls[BITSETSIZE];
    SemiNbs m_semis[BITSETSIZE];
    
    Queue<Full> m_fulls_and_queue;
    Queue<Semi> m_semis_and_queue;
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

    void ProcessedFull(HexPoint x, HexPoint y, bitset_t carrier);

    void AndFull(HexPoint x, HexPoint y, bitset_t carrier);
    void AndFull(HexPoint x, HexPoint z, bitset_t carrier,
                 HexColor zcolor, bitset_t xzCapturedSet);
    
    void AndSemi(HexPoint x, HexPoint y, HexPoint key, bitset_t carrier);
    void AndSemi(HexPoint x, HexPoint z, HexPoint key, bitset_t carrier,
                 HexColor zcolor, bitset_t xzCapturedSet);

    void OrSemis(HexPoint x, HexPoint y);

    void AndFullEmptyFull(HexPoint x, HexPoint z, bitset_t carrier,
                          bitset_t xzCapturedSet);
    void AndFullEmptyFull(HexPoint x, HexPoint z, HexPoint y, bitset_t carrier,
                          bitset_t xyCapturedSet);
    template <bool xy_fulls_null>
    void AndFullEmptyFull(HexPoint x, HexPoint z, HexPoint y, bitset_t carrier,
                          bitset_t xyCapturedSet,
                          AndList* zy_fulls, AndList* xy_fulls);
    template <bool xy_fulls_null, bool xy_semis_null>
    void AndFullEmptyFullIterate(HexPoint x, HexPoint z, HexPoint y,
                                 bitset_t carrier, bitset_t xyCapturedSet,
                                 CarrierList::Iterator i,
                                 AndList* xy_fulls, SemiList* xy_semis);
    template <bool xy_fulls_null, bool xy_semis_null>
    bool TryAndFullEmptyFull(HexPoint x, HexPoint z, HexPoint y,
                             bitset_t xz_carrier, bitset_t zy_carrier,
                             bitset_t xyCapturedSet,
                             AndList* xy_fulls, SemiList* &xy_semis);
    
    void AndFullStoneFull(HexPoint x, HexPoint z, bitset_t carrier,
                          bitset_t xzCapturedSet);
    void AndFullStoneFull(HexPoint x, HexPoint z, HexPoint y,
                          bitset_t carrier, bitset_t xyCapturedSet);
    template <bool xy_fulls_null>
    void AndFullStoneFull(HexPoint x, HexPoint y,
                          bitset_t carrier, bitset_t xyCapturedSet,
                          AndList* zy_fulls, AndList* xy_fulls);
    template <bool xy_fulls_null, bool xy_semis_null>
    void AndFullStoneFullIterate(HexPoint x, HexPoint y,
                                 bitset_t carrier, bitset_t xyCapturedSet,
                                 CarrierList::Iterator i,
                                 AndList* xy_fulls, SemiList* xy_semis);
    template <bool xy_fulls_null, bool xy_semis_null>
    bool TryAndFullStoneFull(HexPoint x, HexPoint y,
                             bitset_t xz_carrier, bitset_t zy_carrier,
                             bitset_t xyCapturedSet,
                             AndList* &xy_fulls, SemiList* &xy_semis,
                             bool &new_semi);

    void AndFullStoneSemi(HexPoint x, HexPoint z, bitset_t carrier,
                          bitset_t xzCapturedSet);
    void AndFullStoneSemi(HexPoint x, HexPoint z, HexPoint y,
                          bitset_t carrier, bitset_t xyCapturedSet);
    void AndFullStoneSemi(HexPoint x, HexPoint y, HexPoint key,
                          bitset_t carrier, bitset_t xyCapturedSet,
                          AndList* zy_list);
    template <bool xy_fulls_null>
    void AndFullStoneSemi(HexPoint x, HexPoint y, HexPoint key,
                          bitset_t carrier, bitset_t xyCapturedSet,
                          AndList* zy_list, AndList* xy_fulls);
    template <bool xy_fulls_null, bool xy_semis_null>
    void AndFullStoneSemiIterate(HexPoint x, HexPoint y, HexPoint key,
                                 bitset_t carrier, bitset_t xyCapturedSet,
                                 CarrierList::Iterator i,
                                 AndList* xy_fulls, SemiList* xy_semis);
    template <bool xy_fulls_null, bool xy_semis_null>
    bool TryAndFullStoneSemi(HexPoint x, HexPoint y, HexPoint key,
                             bitset_t xz_carrier, bitset_t zy_carrier,
                             bitset_t xyCapturedSet,
                             AndList* xy_fulls, SemiList* &xy_semis);

    void AndSemiStoneFull(HexPoint x, HexPoint z, HexPoint key,
                          bitset_t carrier, bitset_t xzCapturedSet);

    template <bool fulls_null>
    bool TryAddFull(HexPoint x, HexPoint y, bitset_t carrier, AndList* &fulls);
    bool TryAddFull(HexPoint x, HexPoint y, bitset_t carrier);
    template <bool fulls_null, bool semis_null>
    bool TryAddSemi(HexPoint x, HexPoint y, bitset_t carrier, HexPoint key,
                    bitset_t xyCaptureSet, AndList* fulls, SemiList* &semis);
    void SemiRemoveSupersetsOf(HexPoint x, HexPoint y, bitset_t carrier);
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
