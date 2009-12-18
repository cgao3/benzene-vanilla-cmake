//----------------------------------------------------------------------------
/** @file BenzeneException.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXEXCEPTION_H
#define HEXEXCEPTION_H

#include <string>
#include <exception>
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Base class for exceptions. */
class BenzeneException
    : public std::exception
{
public:
    /** Constructs an exception with no message. */
    BenzeneException();

    /** Construct an exception with the given message. */
    BenzeneException(const std::string& message);
    
    /** Destructor. */
    virtual ~BenzeneException() throw();

    /** Implementation of std::exception::what(). */
    const char* what() const throw();

private:
    std::string m_message;
};

inline BenzeneException::BenzeneException()
    : m_message("")
{ }

inline BenzeneException::BenzeneException(const std::string& message)
    : m_message(message)
{ }

inline BenzeneException::~BenzeneException() throw()
{ }

inline const char* BenzeneException::what() const throw()
{
    return m_message.c_str();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENEEXCEPTION_H
