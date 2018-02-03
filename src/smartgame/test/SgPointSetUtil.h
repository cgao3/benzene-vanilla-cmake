//----------------------------------------------------------------------------
/** @file SgPointSetUtil.h
    Utility functions for SgPointSet. */
//----------------------------------------------------------------------------

#ifndef SG_POINTSETUTIL_H
#define SG_POINTSETUTIL_H

#include <iosfwd>
#include <string>
#include "SgPointSet.h"

//----------------------------------------------------------------------------

/** Write all points in set. */
class SgWritePointSet
{
public:
    SgWritePointSet(const SgPointSet& pointSet, std::string label = "",
                    bool writeSize = true);

    std::ostream& Write(std::ostream& out) const;

private:
    bool m_writeSize;

    const SgPointSet& m_pointSet;

    std::string m_label;
};

/** @relatesalso SgWritePointSet */
std::ostream& operator<<(std::ostream& out, const SgWritePointSet& write);

/** @relatesalso SgPointSet */
std::ostream& operator<<(std::ostream& out, const SgPointSet& set);

//----------------------------------------------------------------------------

/** Write center point and size of set */
class SgWritePointSetID
{
public:
    explicit SgWritePointSetID(const SgPointSet& points)
        : m_points(points)
    { }

    const SgPointSet& Points() const
    {
    	return m_points;
    }
private:

    const SgPointSet& m_points;
};

std::ostream& operator<<(std::ostream& stream, const SgWritePointSetID& w);

//----------------------------------------------------------------------------

/** Read pointset from stream.
	The format is:
    1. number of points in set
	2. List of point coordinates
    @todo No error checking is done.
*/
std::istream& operator>>(std::istream& in, SgPointSet& pointSet);

//----------------------------------------------------------------------------

namespace SgPointSetUtil
{
    /** Rotate coordinates - see SgPointUtil::Rotate */
    void Rotate(int rotation, SgPointSet& pointSet, int boardSize);
}
//----------------------------------------------------------------------------

#endif // SG_POINTSETUTIL_H
