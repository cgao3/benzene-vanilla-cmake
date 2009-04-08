//----------------------------------------------------------------------------
/** @file
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

//----------------------------------------------------------------------------

#endif // HEXEXCEPTION_H
