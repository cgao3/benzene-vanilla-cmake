//----------------------------------------------------------------------------
/** @file BenzeneException.hpp */
//----------------------------------------------------------------------------

#ifndef BENZENEEXCEPTION_H
#define BENZENEEXCEPTION_H

#include <string>
#include <sstream>
#include <exception>
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Base class for exceptions. 
    Usage examples:
    @verbatim
      BenzeneException("Message");
      BenzeneException() << "Message" << data << "more message.";
    @endverbatim
 */
class BenzeneException
    : public std::exception
{
public:
    /** Constructs an exception with no message. */
    BenzeneException();

    /** Construct an exception with the given message. */
    BenzeneException(const std::string& message);

    /** Needed for operator<<. */
    BenzeneException(const BenzeneException& other);
    
    /** Destructor. */
    virtual ~BenzeneException() throw();

    /** Returns the error message. */
    const char* what() const throw();

    std::string Response() const;

    std::ostream& Stream();

private:
    std::ostringstream m_stream;

    mutable std::string m_what;
};

inline std::ostream& BenzeneException::Stream()
{
    return m_stream;
}

inline std::string BenzeneException::Response() const
{
    return m_stream.str();
}

//----------------------------------------------------------------------------

/** @relates BenzeneException
    @note Returns a new object, see @ref BenzeneException
*/
template<typename TYPE>
BenzeneException operator<<(const BenzeneException& except, const TYPE& type)
{
    BenzeneException result(except);
    result.Stream() << type;
    return result;
}

/** @relates BenzeneException
    @note Returns a new object, see @ref BenzeneException
*/
template<typename TYPE>
BenzeneException operator<<(const BenzeneException& except, TYPE& type)
{
    BenzeneException result(except);
    result.Stream() << type;
    return result;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENEEXCEPTION_H
