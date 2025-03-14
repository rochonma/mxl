#include "Time.hpp"

#include <ctime>

bool
operator==( const timespec &lhs, const timespec &rhs )
{
    return ( lhs.tv_sec == rhs.tv_sec ) && ( lhs.tv_nsec == rhs.tv_nsec );
}

bool
operator!=( const timespec &lhs, const timespec &rhs )
{
    return !( lhs == rhs );
}

bool
operator<( const timespec &lhs, const timespec &rhs )
{
    if ( lhs.tv_sec < rhs.tv_sec )
        return true;
    if ( lhs.tv_sec > rhs.tv_sec )
        return false;
    return lhs.tv_nsec < rhs.tv_nsec;
}

bool
operator<=( const timespec &lhs, const timespec &rhs )
{
    return ( lhs < rhs ) || ( lhs == rhs );
}

bool
operator>( const timespec &lhs, const timespec &rhs )
{
    return !( lhs <= rhs );
}

bool
operator>=( const timespec &lhs, const timespec &rhs )
{
    return !( lhs < rhs );
}