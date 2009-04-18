//----------------------------------------------------------------------------
/** @file HexException.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXEXCEPTION_H
#define HEXEXCEPTION_H

#include <string>
#include <exception>

//----------------------------------------------------------------------------

/** Base class for exceptions. */
class HexException
    : public std::exception
{
public:
    /** Constructs an exception with no message. */
    HexException();

    /** Construct an exception with the given message. */
    HexException(const std::string& message);
    
    /** Destructor. */
    virtual ~HexException() throw();

    /** Implementation of std::exception::what(). */
    const char* what() const throw();

private:
    std::string m_message;
};

inline HexException::HexException()
    : m_message("")
{ }

inline HexException::HexException(const std::string& message)
    : m_message(message)
{ }

inline HexException::~HexException() throw()
{ }

inline const char* HexException::what() const throw()
{
    return m_message.c_str();
}

//----------------------------------------------------------------------------

#endif // HEXEXCEPTION_H
