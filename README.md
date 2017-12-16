// D E S C R I P T I O N
// Debugging-, monitoring-, testing-framework implemented in single c++ header file.
//
// 5-steps Tutorial
// ------------------
// In your program you want to test a function: void myFoo(). The function itself has no
// parameters and doesn't return anything. You just want to see the behaviour of your program
// if you called it.
// 1. Create a source file (e.g.) test.cpp and include MoDePP.h there.
// 2. Start MoDe++ using the macro MODEPP_START(4545)
// 3. Use MoDe++ macro MODEPP_TEST_FUNCTION to declare your function as calable test-function:
//    MODEPP_TEST_FUNCTION( myfoo );
// 4. Add test.cpp to your project, compile and build your project (boost 1.35 and above required)
// 5. Invoke your test-function over network and see the effect
//
// How it works
// ------------
// MoDe++ is a server which works in it's own thread. It contain a map of function-name to func. pointer.
// MoDe++ macros register functions of your program in this map. Using MoDe++ network protocol
// a client can get the list of registered test-functions and call them by name.
// MoDe+ macros use static-initialization of C++. There is no need to place any of MoDe++ macros
// in your program-code, no need to incude MoDePP.h in your program-code or to call any MoDe++ functions from there.
//
// MoDe++ communication protocol
// -----------------------------
// MoDe++ uses ASCII-bases, fixed-lenngth-header, 0-filled, length-value protocol
// The header consists of 8 hex digits: length - 4 digits, command - 4 digits e.g.:
// Message length doesn't include the length of header.
// 00000002 - Header for "List Functions" message. (The message has no data)
// For command IDs see the enum CommandNumber. Client programs should include MoDePP.h 
// and use enums as IDs rather than by magic numbers. Therefore there is no point to
// document all IDs as they could change. Only two commands MsgGetVersion and MsgVersion
// have fixed IDs - respectivly 0x0000 and 0x0001
//
// Messages
// MSG-Name         | Direction | example/descriprion
// MsgGetVersion    | C - S     | 00000000
// MsgVersion       | S - C     | <LenOfVersionString>0001<VersionString>
// MsgAddFunction   | S - C     | <LenOfFuncData><MsgAddFunctionID><FuncName>[ <ParamName1>[ ParamName2[...[ ParamName5]]]]]
// MsgListFunctions | C - S     | 0000<MsgListFunctionsID>
// MsgCallFunction  | C - S     | <LenOfFuncData><MsgCallFunctionID><LenOfFuncName><FuncName>[<LenOfParamData><ParamData>[...]]
// MsgTrace         | S - C     | <LenOfTraceData><MsgTraceID><TraceData>
// MsgReturn        | S - C     | <LenOfReturnData><MsgReturnID><ReturnData>
