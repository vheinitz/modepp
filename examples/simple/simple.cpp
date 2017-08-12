// MoDe++ simple testprogram
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Valentin Heinitz, vheinitz@googlemail.com, http://heinitz-it.de
//
// Distributed under the GNU Lesser General Public License: 
//    http://www.gnu.org/licenses/lgpl-3.0.html
//
// Description:
//  This code demonstrates how to initiate MoDe++ server and declare functions to be invoked.
//
#include <iostream>


////////////////////////////////////////////////////////////////////////////////////
//  Your functions, which should be invoked for debugging and monitoring purposes //
////////////////////////////////////////////////////////////////////////////////////

///Test-function without parameters
void foo()
{
    std::cout << "FOO "<<std::endl;
}

///Test-function with 2 parameters: string and int
void foo2(const std::string & p1, int p2)
{
    std::cout << "FOO2 "<<p1.c_str()<<" "<<p2<<std::endl;
}

///Test-function with 5 parameters: 1 string and 4 ints
void foo5(const std::string & p1, int p2, int p3, int p4, int p5 )
{
    std::cout << "FOO5 "<<p1.c_str()<<" "<<p2<<" "<<p3<<" "<<p4<<" "<<p5<<std::endl;
}
///Test-function for demonstrating return
int add( int a, int b )
{
	return a+b;
}

////////////////////////////////////////////////////////////////////////////////////
// Initialize MODe++ and define which functions which                             //
// should be available for remote invocation.                                     //
// This code could be implemented in a separate file                              //
////////////////////////////////////////////////////////////////////////////////////

///Include the header where the macros are defined.
#include "MoDePP.h"

///Initialize and start MoDe++ Server at port 4545
MODEPP_START( 4545 )

///Declare test-tunction "foo" which will call a function with the same name
MODEPP_TEST_FUNCTION( foo )

///Declare test-tunction "foo2" which will call a function with the same name
MODEPP_TEST_FUNCTION2( foo2, a1,a2 )

///Declare test-tunction "foo5" which will call a function with the same name
MODEPP_TEST_FUNCTION5( foo5,a1,a2,a3,a4,a5 )

/**
Declare test-tunction "test_addition" which will call function "add".
The construct MODEPP_BEGIN_TEST_FUNCTIONx - END_TEST_FUNCTION ma be used for many other things,
not only for making aliases to the function.
*/
MODEPP_BEGIN_TEST_FUNCTION2( test_addition, a1,a2 )
       int sum =  add( a1,a2 );
       MODEPP_RETURN_TEST_FUNCTION (  sum )
MODEPP_END_TEST_FUNCTION

MODEPP_TEST_FUNCTION2( add, a1,a2 )

////////////////////////////////////////////////////////////////////////////////////
// Your Program.
////////////////////////////////////////////////////////////////////////////////////
int main()
{
	std::string line;
        std::cout << "Exit with 'q' as single character in the line"
                <<std::endl;
	do
	{
		std::getline(std::cin, line);
                std::cout << line <<std::endl;
	}while(line!="q");
    
	return 0;
}

