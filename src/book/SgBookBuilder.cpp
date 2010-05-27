//----------------------------------------------------------------------------
/** @file SgBookBuilder.cpp 
 */
//----------------------------------------------------------------------------

#include "SgBookBuilder.h"
#include <boost/numeric/conversion/bounds.hpp>

using namespace benzene;

//----------------------------------------------------------------------------

SgBookBuilder::SgBookBuilder()
    : m_alpha(50),
      m_use_widening(true),
      m_expand_width(16),
      m_expand_threshold(1000),
      m_flush_iterations(100)
{
}

SgBookBuilder::~SgBookBuilder()
{
}

//----------------------------------------------------------------------------

void SgBookBuilder::BeforeEvaluateChildren()
{
    // DEFAULT IMPLEMENTATION DOES NOTHING
}

void SgBookBuilder::AfterEvaluateChildren()
{
    // DEFAULT IMPLEMENTATION DOES NOTHING
}

void SgBookBuilder::Expand(int numExpansions)
{
    m_num_evals = 0;
    m_num_widenings = 0;

    double s = Time::Get();
    Init();
    EnsureRootExists();
    int num = 0;
    for (; num < numExpansions; ++num) 
    {
	LogInfo() << "\n--Iteration " << num << "--\n";
        // Flush the db if we've performed enough iterations
        if (num && (num % m_flush_iterations) == 0) 
            FlushBook();
	// If root position becomes a known win or loss, then there's
	// no point in continuing to expand the opening book.
        {
            BookNode root;
            GetNode(root);
            if (root.IsTerminal()) 
            {
                LogInfo() << "State solved!\n";
                break;
            }
        }
        std::vector<SgMove> pv;
        DoExpansion(pv);
    }
    FlushBook();
    Fini();
    double e = Time::Get();

    LogInfo() << '\n'
              << "  Total Time: " << Time::Formatted(e - s) << '\n'
              << "  Expansions: " << num 
              << std::fixed << std::setprecision(2) 
              << " (" << (num / (e - s)) << "/s)\n"
              << " Evaluations: " << m_num_evals 
              << std::fixed << std::setprecision(2)
              << " (" << (m_num_evals / (e - s)) << "/s)\n"
              << "   Widenings: " << m_num_widenings << '\n';
}

void SgBookBuilder::Refresh()
{
    m_num_evals = 0;
    m_num_widenings = 0;
    m_value_updates = 0;
    m_priority_updates = 0;
    m_internal_nodes = 0;
    m_leaf_nodes = 0;
    m_terminal_nodes = 0;

    double s = Time::Get();
    Init();
    Refresh(true);
    FlushBook();
    Fini();
    double e = Time::Get();

    LogInfo() << '\n'
              << "      Total Time: " << Time::Formatted(e - s) << '\n'
              << "   Value Updates: " << m_value_updates << '\n'
              << "Priority Updates: " << m_priority_updates << '\n'
              << "  Internal Nodes: " << m_internal_nodes << '\n'
              << "  Terminal Nodes: " << m_terminal_nodes << '\n'
              << "      Leaf Nodes: " << m_leaf_nodes << '\n'
              << "     Evaluations: " << m_num_evals 
              << std::fixed << std::setprecision(2)
              << " (" << (m_num_evals / (e - s)) << "/s)\n"
              << "       Widenings: " << m_num_widenings << '\n';
}

void SgBookBuilder::IncreaseWidth()
{
    if (!m_use_widening)
    {
        LogInfo() << "Widening not enabled!\n";
        return;
    }

    m_num_evals = 0;
    m_num_widenings = 0;

    double s = Time::Get();
    Init();
    IncreaseWidth(true);
    FlushBook();
    Fini();
    double e = Time::Get();

    LogInfo() << '\n'
              << "      Total Time: " << Time::Formatted(e - s) << '\n'
              << "       Widenings: " << m_num_widenings << '\n'
              << "     Evaluations: " << m_num_evals 
              << std::fixed << std::setprecision(2)
              << " (" << (m_num_evals / (e - s)) << "/s)\n";
}

//----------------------------------------------------------------------------

/** Creates a node for each of the leaf's first count children that
    have not been created yet. Returns true if at least one new node
    was created, false otherwise. */
bool SgBookBuilder::ExpandChildren(std::size_t count)
{
    // It is possible the state is determined, even though it was
    // already evaluated. This is not very likey if the evaluation
    // function is reasonably heavyweight, but if just using fillin
    // and vcs, it is possible that the fillin prevents a winning vc
    // from being created.
    HexEval value = 0;
    std::vector<SgMove> children;
    if (GenerateMoves(children, value))
    {
        LogInfo() << "ExpandChildren: State is determined!\n";
        WriteNode(BookNode(value));
        return false;
    }
    std::size_t limit = std::min(count, children.size());
    std::vector<SgMove> childrenToDo;
    for (std::size_t i = 0; i < limit; ++i)
    {
        PlayMove(children[i]);
        BookNode child;
        if (!GetNode(child))
            childrenToDo.push_back(children[i]);
        UndoMove(children[i]);
    }
    if (!childrenToDo.empty())
    {
        BeforeEvaluateChildren();
        std::vector<std::pair<SgMove, HexEval> > scores;
        EvaluateChildren(childrenToDo, scores);
        AfterEvaluateChildren();
        for (std::size_t i = 0; i < scores.size(); ++i)
        {
            PlayMove(scores[i].first);
            WriteNode(scores[i].second);
            UndoMove(scores[i].first);
        }
        m_num_evals += childrenToDo.size();
        return true;
    }
    else
        LogInfo() << "Children already evaluated.\n";
    return false;
}

std::size_t SgBookBuilder::NumChildren(const std::vector<SgMove>& legal)
{
    std::size_t num = 0;
    for (size_t i = 0; i < legal.size(); ++i) 
    {
	PlayMove(legal[i]);
	BookNode child;
        if (GetNode(child))
            ++num;
        UndoMove(legal[i]);
    }
    return num;
}

void SgBookBuilder::UpdateValue(BookNode& node, 
                                const std::vector<SgMove>& legal)
{
    bool hasChild = false;
    float bestValue = boost::numeric::bounds<float>::lowest();
    for (std::size_t i = 0; i < legal.size(); ++i)
    {
	PlayMove(legal[i]);
	BookNode child;
        if (GetNode(child))
        {
            hasChild = true;
            float value = BookUtil::InverseEval(Value(child));
            if (value > bestValue)
		bestValue = value;
        }
        UndoMove(legal[i]);
    }
    if (hasChild)
        node.m_value = bestValue;
}

/** Updates the node's value, taking special care if the value is a
    loss. In this case, widenings are performed until a non-loss child
    is added or no new children are added. The node is then set with
    the proper value. */
void SgBookBuilder::UpdateValue(BookNode& node)
{
    while (true)
    {
        std::vector<SgMove> legal;
        GetAllLegalMoves(legal);
        UpdateValue(node, legal);
        if (!IsLoss(Value(node)))
            break;
        
        // Round up to next nearest multiple of m_expand_width that is
        // larger than the current number of children.
        unsigned numChildren = NumChildren(legal);
        std::size_t width = (numChildren / m_expand_width + 1) 
            * m_expand_width;

        LogInfo() << "Forced Widening[" << numChildren << "->" 
                  << width << "]\n";
        if (!ExpandChildren(width))
            break;

        ++m_num_widenings;
    }
}

float SgBookBuilder::ComputePriority(const BookNode& parent,
                                     const float childValue,
                                     const float childPriority) const
{
    float delta = parent.m_value - InverseEval(childValue);
    HexAssert(delta >= 0.0);
    HexAssert(childPriority >= BookNode::LEAF_PRIORITY);
    HexAssert(childPriority < BookNode::DUMMY_PRIORITY);
    return m_alpha * delta + childPriority + 1;
}

/** Re-computes node's priority and returns the best child to
    expand. Requires that UpdateValue() has been called on this
    node. Returns SG_NULLMOVE if node has no children. */
SgMove SgBookBuilder::UpdatePriority(BookNode& node)
{
    bool hasChild = false;
    float bestPriority = boost::numeric::bounds<float>::highest();
    SgMove bestChild = SG_NULLMOVE;
    std::vector<SgMove> legal;
    GetAllLegalMoves(legal);
    for (std::size_t i = 0; i < legal.size(); ++i)
    {
	PlayMove(legal[i]);
	BookNode child;
        if (GetNode(child))
        {
            hasChild = true;
            // Must adjust child value for swap, but not the parent
            // because we are comparing with the best child's value,
            // ie, the minmax value.
            float childValue = Value(child);
            float childPriority = child.m_priority;
            float priority = ComputePriority(node, childValue, childPriority);
            if (priority < bestPriority)
            {
                bestPriority = priority;
                bestChild = legal[i];
            }
        }
        UndoMove(legal[i]);
    }
    if (hasChild)
        node.m_priority = bestPriority;
    return bestChild;
}

void SgBookBuilder::DoExpansion(std::vector<SgMove>& pv)
{
    BookNode node;
    if (!GetNode(node))
        SG_ASSERT(false);
    if (node.IsTerminal())
        return;
    if (node.IsLeaf())
    {
        // Expand this leaf's children
        //LogInfo() << "Expanding: " << ToString(pv) << '\n';
        ExpandChildren(m_expand_width);
    }
    else
    {
        // Widen this non-terminal non-leaf node if necessary
        if (m_use_widening && (node.m_count % m_expand_threshold == 0))
        {
            std::size_t width = (node.m_count / m_expand_threshold + 1)
                              * m_expand_width;
            // LogInfo() << "Widening[" << width << "]: " 
//                       << ToString(pv) << '\n';
            ++m_num_widenings;
            ExpandChildren(width);
        }

        // Compute value and priority. It's possible this node is newly
        // terminal if one of its new children is a winning move.
        GetNode(node);
        UpdateValue(node);
        SgMove mostUrgent = UpdatePriority(node);
        WriteNode(node);

        // Recurse on most urgent child only if non-terminal.
        if (!node.IsTerminal())
        {
            PlayMove(mostUrgent);
            pv.push_back(mostUrgent);
            DoExpansion(pv);
            pv.pop_back();
            UndoMove(mostUrgent);
        }
    }

    GetNode( node);
    UpdateValue(node);
    UpdatePriority(node);
    node.IncrementCount();
    WriteNode(node);
}

//----------------------------------------------------------------------------

/** Refresh's each child of the given state. UpdateValue() and
    UpdatePriority() are called on internal nodes. Returns true if
    state exists in book.  
    @ref bookrefresh
*/
bool SgBookBuilder::Refresh(bool root)
{
    BookNode node;
    if (!GetNode(node))
        return false;
    if (node.IsLeaf())
    {
        m_leaf_nodes++;
        if (node.IsTerminal())
            m_terminal_nodes++;
        return true;
    }
    double oldValue = Value(node);
    double oldPriority = node.m_priority;
    std::vector<SgMove> legal;
    GetAllLegalMoves(legal);
    for (std::size_t i = 0; i < legal.size(); ++i)
    {
        PlayMove(legal[i]);
        Refresh(false);
        if (root)
            LogInfo() << "Finished " << legal[i] << '\n';
        UndoMove(legal[i]);
    }
    UpdateValue(node);
    UpdatePriority(node);
    if (fabs(oldValue - Value(node)) > 0.0001)
        m_value_updates++;
    if (fabs(oldPriority - node.m_priority) > 0.0001)
        m_priority_updates++;
    WriteNode(node);
    if (node.IsTerminal())
        m_terminal_nodes++;
    else
        m_internal_nodes++;
    return true;
}

//----------------------------------------------------------------------------

void SgBookBuilder::IncreaseWidth(bool root)
{
    BookNode node;
    if (!GetNode(node))
        return;
    if (node.IsTerminal() || node.IsLeaf())
        return;
    std::vector<SgMove> legal;
    GetAllLegalMoves(legal);
    for (std::size_t i = 0; i < legal.size(); ++i)
    {
        PlayMove(legal[i]);
        IncreaseWidth(false);
        if (root)
            LogInfo() << "Finished " << legal[i] << '\n';
        UndoMove(legal[i]);
    }
    std::size_t width = (node.m_count / m_expand_threshold + 1)
        * m_expand_width;
    if (ExpandChildren(width))
        ++m_num_widenings;
}

//----------------------------------------------------------------------------
