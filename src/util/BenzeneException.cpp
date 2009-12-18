//----------------------------------------------------------------------------
/** @file BenzeneException.cpp
 */
//----------------------------------------------------------------------------

#include <iostream>
#include "BenzeneException.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

BenzeneException::BenzeneException()
    : m_stream("")
{
}

BenzeneException::BenzeneException(const std::string& message)
    : m_stream(message)
{
}

BenzeneException::BenzeneException(const BenzeneException& other)
{
    m_stream << other.Response();
    m_stream.copyfmt(other.m_stream);
}

BenzeneException::~BenzeneException() throw()
{ 
}

const char* BenzeneException::what() const throw()
{
    // Cannot just do 'Response().c_str()' because the temporary will die
    // before the message is printed.
    m_what = Response(); 
    return m_what.c_str();
}

//----------------------------------------------------------------------------

