//----------------------------------------------------------------------------
/** @file SolverDFPN.hpp
 */
//----------------------------------------------------------------------------

#ifndef SOLVERDFPN_HPP
#define SOLVERDFPN_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "TransTable.hpp"

#include <limits>
#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Maximum bound. */
static const std::size_t INFTY = 2000000000;

/** Bounds used in Dfpn search. */
struct DfpnBounds
{
    std::size_t phi;

    std::size_t delta;

    DfpnBounds();

    DfpnBounds(std::size_t p, std::size_t d);

    /** Print bounds in human readable format. */
    std::string Print() const;
};

inline DfpnBounds::DfpnBounds()
    : phi(INFTY), 
      delta(INFTY)
{
}

inline DfpnBounds::DfpnBounds(std::size_t p, std::size_t d)
    : phi(p), 
      delta(d)
{
}

inline std::string DfpnBounds::Print() const
{
    std::ostringstream os;
    os << "[" << phi << ", " << delta << "]";
    return os.str();
}


/** Extends global output operator for DfpnBounds. */
inline std::ostream& operator<<(std::ostream& os, const DfpnBounds& bounds)
{
    os << bounds.Print();
    return os;
}

//----------------------------------------------------------------------------

class DfpnData
{
public:

    hash_t m_hash;

    DfpnBounds m_bounds;

    bitset_t m_children;

    HexPoint m_bestMove;

    DfpnData();

    DfpnData(hash_t hash, const DfpnBounds& bounds, const bitset_t& children, 
             HexPoint bestMove);

    ~DfpnData();

    hash_t Hash() const;

    bool Initialized() const;

    bool ReplaceWith(const DfpnData& data) const;

private:

    bool m_initialized;
};


inline DfpnData::DfpnData()
    : m_initialized(false)
{ 
}

inline DfpnData::DfpnData(hash_t hash, const DfpnBounds& bounds, 
                          const bitset_t& children,  HexPoint bestMove)
    : m_hash(hash), 
      m_bounds(bounds),
      m_children(children),
      m_bestMove(bestMove),
      m_initialized(true)
{ 
}

inline DfpnData::~DfpnData()
{
}

inline hash_t DfpnData::Hash() const
{
    return m_hash;
}

inline bool DfpnData::ReplaceWith(const DfpnData& data) const
{
    SG_UNUSED(data);
    return true;
}

inline bool DfpnData::Initialized() const
{
    return m_initialized;
}

//----------------------------------------------------------------------------

/** Hex solver using DFPN search. */
class SolverDFPN 
{
public:

    SolverDFPN();

    ~SolverDFPN();

    HexColor StartSearch(HexColor colorToMove, HexBoard& game);
    
    /** Dumps output about root state what gui can display. */
    bool UseGuiFx() const;

    /** See UseGuiFx() */
    void SetUseGuiFx(bool enable);

private:

    typedef TransTable<DfpnData> DfpnHashTable;

    /** Handles guifx output. */
    class GuiFx
    {
    public:

        GuiFx();

        void SetChildren(const std::vector<HexPoint>& children,
                         const std::vector<DfpnBounds>& bounds);

        void PlayMove(HexColor color, HexPoint move);

        void UndoMove();

        void UpdateCurrentBounds(const DfpnBounds& bounds);

        void Write();

        void WriteForced();

    private:
        
        std::vector<HexPoint> m_children;

        std::vector<DfpnBounds> m_bounds;

        HexColor m_color;

        HexPoint m_move;

        double m_timeOfLastWrite;

        HexPoint m_moveAtLastWrite;

        double m_delay;

        void DoWrite();
    };

    boost::scoped_ptr<StoneBoard> m_brd;

    HexBoard* m_workBoard;

    boost::scoped_ptr<DfpnHashTable> m_hashTable;

    bool m_aborted;

    GuiFx m_guiFx;

    bool m_useGuiFx;

    int m_ttsize;

    size_t m_numTerminal;

    size_t m_numMIDcalls;

    void MID(const DfpnBounds& n, int depth);

    void SelectChild(int& bestMove, std::size_t& delta2, 
                     const std::vector<DfpnBounds>& childrenDfpnBounds) const;

    void UpdateBounds(DfpnBounds& bounds, 
                      const std::vector<DfpnBounds>& childBounds) const;

    bool CheckAbort();

    void CheckBounds(const DfpnBounds& bounds) const;

    void LookupBounds(DfpnBounds& bound, HexColor colorToMove, HexPoint cell);

    void TTStore(hash_t hash, const DfpnData& data);

    void GetVariation(const StoneBoard& state, 
                      std::vector<HexPoint>& pv) const;

    std::string PrintVariation(const std::vector<HexPoint>& pv) const;

    void DumpGuiFx(const std::vector<HexPoint>& children,
                   const std::vector<DfpnBounds>& childBounds) const;
};

inline bool SolverDFPN::UseGuiFx() const
{
    return m_useGuiFx;
}

inline void SolverDFPN::SetUseGuiFx(bool enable)
{
    m_useGuiFx = enable;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERDFPN_HPP
