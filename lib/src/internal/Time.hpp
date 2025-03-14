#pragma once

#include <ctime>

bool operator==( const timespec &lhs, const timespec &rhs );

bool operator!=( const timespec &lhs, const timespec &rhs );

bool operator<( const timespec &lhs, const timespec &rhs );

bool operator<=( const timespec &lhs, const timespec &rhs );

bool operator>( const timespec &lhs, const timespec &rhs );

bool operator>=( const timespec &lhs, const timespec &rhs );