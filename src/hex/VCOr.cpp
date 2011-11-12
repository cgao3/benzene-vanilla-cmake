#include "VCOr.hpp"
#include "VCS.hpp"

using namespace std;
using namespace benzene;

class VCOrCombiner
{
public:
    VCOrCombiner(const CarrierList& semis,
                 const CarrierList& fulls,
                 bitset_t capturedSet);
    
    vector<bitset_t> SearchResult() const;
    
private:
    bitset_t m_capturedSet;
    mutable std::vector<bitset_t> m_mem;
    
    int Search(bitset_t forbidden, bool can_capture,
               int new_semis, int new_semis_count, int old_semis_count,
               int filtered_count);
    
    bitset_t Intersect(int start, int count) const;
    bitset_t Add(int start, int count, bitset_t capturedSet);
    int Filter(int start, int count, size_t a) const;
};

VCOrCombiner::VCOrCombiner(const CarrierList& semis,
                           const CarrierList& fulls,
                           bitset_t capturedSet)
    : m_capturedSet(capturedSet)
{
    m_mem.resize(semis.Count() + fulls.Count());
    size_t memidx = 0;
    int new_semis_count = 0;
    int old_semis_count = 0;
    for (CarrierList::Iterator i(semis); i; ++i)
        if (!i.Old())
        {
            m_mem[memidx++] = i.Carrier();
            new_semis_count++;
        }
    if (!new_semis_count)
    {
        m_mem.clear();
        return;
    }
    for (CarrierList::Iterator i(semis); i; ++i)
        if (i.Old())
        {
            m_mem[memidx++] = i.Carrier();
            old_semis_count++;
        }
    for (CarrierList::Iterator i(fulls); i; ++i)
        m_mem[memidx++] = i.Carrier();;
    Search(bitset_t(), true,
           0, new_semis_count, old_semis_count, fulls.Count());
}

inline vector<bitset_t> VCOrCombiner::SearchResult() const
{
    return m_mem;
}

int VCOrCombiner::Search(bitset_t forbidden, bool can_capture,
                         int new_semis, int new_semis_count, int old_semis_count,
                         int filtered_count)
{
    BenzeneAssert(new_semis_count > 0);
    int old_semis = new_semis + new_semis_count;
    
    bitset_t I_new = Intersect(new_semis, new_semis_count);
    bitset_t I_old = Intersect(old_semis, old_semis_count);
    bitset_t I = I_new & I_old;

    if (can_capture ? !BitsetUtil::IsSubsetOf(I, m_capturedSet) : I.any())
    {
        m_mem.resize(new_semis);
        return 0;
    }
    
    int filtered = old_semis + old_semis_count;
    int new_conn = filtered + filtered_count;
    int new_conn_count = 0;
    
    if (filtered_count == 0)
    {
        bitset_t new_t = Add(new_semis, new_semis_count + old_semis_count,
                             I.any() ? m_capturedSet : EMPTY_BITSET);
        m_mem.push_back(new_t);
        filtered_count++;
        new_conn_count++;
    }
    
    forbidden |= I_new;
    
    while (true)
    {
        size_t min_size = std::numeric_limits<size_t>::max();
        bitset_t allowed;
        for (int i = 0; i < filtered_count; i++)
        {
            bitset_t A = m_mem[filtered + i] - forbidden;
            size_t size = A.count();
            if (size < min_size)
            {
                min_size = size;
                allowed = A;
            }
        }
        
        if (min_size == 0)
        {
            for (int i = 0; i < new_conn_count; i++)
                m_mem[new_semis + i] = m_mem[new_conn + i];
            /* std::copy(m_mem.begin() + new_conn,
             *                      m_mem.begin() + new_conn + new_conn_count,
             *                      m_mem.begin() + new_semis); */
            m_mem.resize(new_semis + new_conn_count);
            return new_conn_count;
        }
        
        size_t a = allowed._Find_first();
        BenzeneAssert(a < allowed.size());
        forbidden.set(a);
        
        int rec_new_semis = filtered + filtered_count;
        int rec_new_semis_count = Filter(new_semis, new_semis_count, a);
        int rec_old_semis_count = Filter(old_semis, old_semis_count, a);
        int rec_filtered_count = Filter(filtered, filtered_count, a);
        int rec_new_conn_count =
        Search(forbidden, can_capture & !m_capturedSet[a],
               rec_new_semis, rec_new_semis_count, rec_old_semis_count,
               rec_filtered_count);
        filtered_count += rec_new_conn_count;
        new_conn_count += rec_new_conn_count;
    }
}

inline bitset_t VCOrCombiner::Intersect(int start, int count) const
{
    bitset_t I;
    I.flip();
    for (int i = 0; i < count; i++)
        I &= m_mem[start + i];
    return I;
}

inline bitset_t VCOrCombiner::Add(int start, int count, bitset_t capturedSet)
{
    bitset_t U = capturedSet;
    bitset_t I;
    I.flip();
    for (int i = 0; ; i++)
    {
        BenzeneAssert(i < count);
        bitset_t next = m_mem[start + i];
        if (BitsetUtil::IsSubsetOf(I, next))
            continue;
        I &= next;
        U |= next;
        if (BitsetUtil::IsSubsetOf(I, capturedSet))
            break;
    }
    return U;
}

inline int VCOrCombiner::Filter(int start, int count, size_t a) const
{
    int res = 0;
    for (int i = 0; i < count; i++)
    {
        bitset_t s = m_mem[start + i];
        if (!s[a])
        {
            m_mem.push_back(s);
            res++;
        }
    }
    return res;
}

vector<bitset_t> benzene::VCOr(const CarrierList& semis,
                               const CarrierList& fulls,
                               bitset_t capturedSet)
{
    VCOrCombiner comb(semis, fulls, capturedSet);
    return comb.SearchResult();
}
