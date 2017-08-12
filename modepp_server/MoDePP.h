//
// MoDe++ - Monitoring & Debugging of C++ code
// Server declaration and implementation
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Valentin Heinitz, vheinitz@googlemail.com, http://heinitz-it.de
//
// Distributed under the GNU Lesser General Public License: 
//    http://www.gnu.org/licenses/lgpl-3.0.html
//
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

#ifndef _MoDePP_HG_
#define _MoDePP_HG_

////////////// Common client-server declatations /////////////////////////////

///Length of message-header: 4 hex digits for message-length + 4 hex digits for message-type
#define HEADER_LEN 8

///Enum for client/server commands
enum CommandNumber{
    MsgGetVersion=0,    ///<client's request for server's version
    MsgVersion=1,       ///<Server's version-response
    MsgAddFunction,     ///<server sends to client (multiple times) as response to MsgListFunctions
    MsgListFunctions,   ///<client request server for sending list of available test-functions
    MsgCallFunction,    ///<client requests server to call a particular test-function
    MsgTrace,           ///<Server sends data which should be interpreted as trace-message
    MsgReturn,          ///<Server sends data which should be interpreted as return of test-function
};

///A client should declare this macro in order to disable server implementation.  If declared, stop here.
#ifndef MODEPP_INCLUDE_MESSAGE_TYPES_ONLY

#ifdef USING_BOOST_ASIO         // asio belongs to boost since 1.35
  #include <boost/asio.hpp>
  using namespace boost::asio;
  using boost::asio::ip::tcp;
  using namespace boost::system;
#else
  #include <asio.hpp>		// asio is standalone till 1.34
  using namespace asio;
  using asio::ip::tcp;
#endif

#include <iostream>
#include <boost/bind.hpp>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <list>
#define foreach BOOST_FOREACH
#include <iomanip>
#include <iterator>
#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <string>

using std::setw;
using std::hex;
using std::endl;
using std::setfill;
using std::cout;
using std::string;
using std::stringstream;

///Version/Info String
static const std::string MoDePP_Version="0.02 "__DATE__;

///Use this macro one time in order to start MoDe++ server
#define MODEPP_START( port ) static int DummyIntUsedForStartingServer=MoDePP::instance().start(port);\
struct DummyClassUsedForSurpressingWarning{ int i;DummyClassUsedForSurpressingWarning():i(DummyIntUsedForStartingServer){} };

///Begin test-function without parameters
#define MODEPP_BEGIN_TEST_FUNCTION( FN ) \
namespace testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &, const VarParam &, const VarParam &, const VarParam &, const VarParam &){


///One line test-function without parameters
#define MODEPP_TEST_FUNCTION( FN ) \
namespace simple_testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &, const VarParam &, const VarParam &, const VarParam &, const VarParam &){\
FN();\
}};static CCbWrapper cbwrapper;}


///Begin test-function with 1 parameter
#define MODEPP_BEGIN_TEST_FUNCTION1( FN, P1 ) \
namespace testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){ _parameters=#P1; testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam & (P1), const VarParam &, const VarParam &, const VarParam &, const VarParam &){

///One line test-function with 1 parameter
#define MODEPP_TEST_FUNCTION1( FN, P1 ) \
namespace simple_testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &P1, const VarParam &, const VarParam &, const VarParam &, const VarParam &){\
FN(P1);\
}};static CCbWrapper cbwrapper;}

///Begin test-function with 2 parameters
#define MODEPP_BEGIN_TEST_FUNCTION2( FN, P1, P2 ) \
namespace testFunction_ns_##FN{\
        const char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &P1, const VarParam &P2, const VarParam &, const VarParam &, const VarParam &){

///One line test-function with 2 parameters
#define MODEPP_TEST_FUNCTION2( FN, P1, P2 ) \
namespace simple_testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &P1, const VarParam &P2, const VarParam &, const VarParam &, const VarParam &){\
FN(P1,P2);\
}};static CCbWrapper cbwrapper;}


///Begin test-function with 3 parameters
#define MODEPP_BEGIN_TEST_FUNCTION3( FN, P1, P2, P3 ) \
namespace testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2" "#P3;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &P1, const VarParam &P2, const VarParam &P3, const VarParam &, const VarParam &){

///One line test-function with 3 parameters
#define MODEPP_TEST_FUNCTION3( FN, P1, P2, P3 ) \
namespace simple_testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2" "#P3;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &P1, const VarParam &P2, const VarParam &P3, const VarParam &, const VarParam &){\
FN(P1,P2,P3);\
}};static CCbWrapper cbwrapper;}


///Begin test-function with 4 parameters
#define MODEPP_BEGIN_TEST_FUNCTION4( FN, P1, P2, P3, P4 ) \
namespace testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2" "#P3" "#P4;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam & P1, const VarParam &P2, const VarParam &P3, const VarParam &P4, const VarParam &){

///One line test-function with 4 parameters
#define MODEPP_TEST_FUNCTION4( FN, P1, P2, P3, P4 ) \
namespace simple_testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2" "#P3" "#P4;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &P1, const VarParam &P2, const VarParam &P3, const VarParam &P4, const VarParam &){\
FN(P1,P2,P3,P4);\
}};static CCbWrapper cbwrapper;}


///Begin test-function with 5 parameter
#define MODEPP_BEGIN_TEST_FUNCTION5( FN, P1, P2, P3, P4, P5 ) \
namespace testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2" "#P3" "#P4" "#P5;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam & P1, const VarParam &P2, const VarParam &P3, const VarParam &P4, const VarParam &P5){

///One line test-function with 5 parameters
#define MODEPP_TEST_FUNCTION5( FN, P1, P2, P3, P4, P5 ) \
namespace simple_testFunction_ns_##FN{\
        const  char *testFunction_ns_fn=0;\
        struct CCbWrapper:public ITestFunctionWrapper{\
                CCbWrapper( const char * fn=#FN ){_parameters=#P1" "#P2" "#P3" "#P4" "#P5;testFunction_ns_fn = fn;MoDePP::instance().addFunction( fn, this );}\
                virtual void testFunction(const VarParam &P1, const VarParam &P2, const VarParam &P3, const VarParam &P4, const VarParam &P5){\
FN(P1,P2,P3,P4,P5);\
}};static CCbWrapper cbwrapper;}


///End test-function
#define MODEPP_END_TEST_FUNCTION  }};static CCbWrapper cbwrapper;}

///Send data to client tagged as "return-value". Value is sent with function-name.
#define MODEPP_RETURN_TEST_FUNCTION( VAL ) { std::stringstream s; s<<testFunction_ns_fn<<" "<<VAL;\
            MoDePP::instance().send( MsgReturn, s.str() ); }

///Static initialization. Code here will be executed before main()
#define MODEPP_BEGIN_STATIC_INITIALISATION( BN ) \
namespace Initializer_ns_##BN{ struct Initializer{ Initializer(){
        	
///End static initialization
#define MODEPP_END_STATIC_INITIALISATION }};static Initializer initializer;}

///TODO: add a variable to be used by client as valid value for a parameter
#define MODEPP_ADD_PARAM_ENUM( PName, VName )  MoDePP::instance().addParamEnum( PName, #VName, VarParam( VName ) ); 

///Send data to client tagged as "trace-value".
#define MODEPP_TRACE( VAL ){ \
	MoDePP::instance().send( MsgTrace, VAL ); }

///Same as above, but with a prefix string.
#define MODEPP_TRACE2( MSG, VAL ){ \
	MoDePP::instance().send( MsgTrace, MSG + VarParam( VAL).toString() ); }	

///Simple variant-value class. Contains value as string, able to convert it implicitely to different types.
class VarParam
{
	std::string _value;
	public:
	VarParam( const std::string & v):_value(v){}
	VarParam( int v){std::stringstream str; str << v; _value=str.str(); }
	operator const std::string &() const {return _value;}
	const std::string & toString() const {return _value;}
	operator int() const { std::stringstream str(_value); int ival=0; str >> ival; return ival; }
	int toInt() const { std::stringstream str(_value); int ival=0; str >> ival; return ival; }
};


///Interface for test-function
struct ITestFunctionWrapper
{
        ///Abstract method. Implemented as wrapper of the function which should be tested.
        virtual void testFunction(const VarParam &, const VarParam &, const VarParam &, const VarParam &, const VarParam &)=0;

        ///names separated by space. sent to client along with corresponding function-name
        std::string _parameters;
};

///States of simple message-receiver state machine
enum ReadState
{
            WaitingHeader,      ///<waiting till minimum number of bytes arrive
            WaitingMsgComplete, ///<waiting till data with length specified in header arrive
            ProcessingMessage   ///<read one message from data and return to WaitingHeader
};

/// Class for reading given number of bytes from stream into any (currently int/string) data type
class FixedLengthValue
{
        mutable int unused_int;
        mutable std::string tmp;
        mutable int & _val;
        mutable std::string & _sval;
        mutable int _len;
        mutable bool _readInt;

        public: FixedLengthValue( int & i, int len ):_val(i),_sval(tmp),_len(len),_readInt(true){};
        public: FixedLengthValue( std::string & s, int len ):_val(unused_int),_sval(s),_len(len),_readInt(false){};        

    public: std::istream& process (std::istream& strm ) const
        {
                if (_len < 0)
                        return strm;

                char *buf = new char[_len+1];
                strm.get(buf, _len+1);
                tmp = buf;
                delete[] buf;
                if(_readInt)
                {
                        std::stringstream ostr(tmp);
                        ostr >> std::hex >> _val;
                        return strm;
                }
                _sval = tmp;
                return strm;
        }
};	

///Main MoDe++ singleton-class. Contains Lists of test-functions, tcp-server.
class MoDePP
{	
    boost::shared_ptr <tcp::socket> _socket;
	io_service _service;
	std::auto_ptr<tcp::acceptor> _acceptor;
	std::auto_ptr<boost::thread> _thread;
	unsigned short _port;

        ///Pair for traversing map usinf boost's foreach
		typedef std::pair<std::string, ITestFunctionWrapper*> FuncMapEntry;

        ///Paid maps variable-name to its value
        typedef std::list< std::pair<std::string, VarParam> > TVarValues;

        ///Map of parameters and their valid lists of values
        typedef std::map< std::string, TVarValues > TParVarValues;

        ///TODO: unused now.
	TParVarValues _paramValues;

        ///Map of function-name to function-pointer wrapper and parameter list
        typedef std::map<std::string,  ITestFunctionWrapper*> FuncMap;
	FuncMap functionMap;
	
        bool _stop;             ///<stop MoDe++ server
        std::string _data;      ///<Buffer of received data

        ReadState _readState;   ///<current state of receiving state machine
        int _expectedLength;    ///<after fixed-size header is read, this variable contains length of message

        MoDePP(const MoDePP &); ///<Private copy-constructor - singleton
        MoDePP& operator=(const MoDePP &); ///<Private op= - singleton
        ///Constructor
	MoDePP():_port(4545),_stop(false), _readState(WaitingHeader),_expectedLength(0)
	{
	
	}
	
	///d-tor
	~MoDePP()
	{
	        stop();
	}
	
	///Waits for commands and processes them
	void doWork()
	{
	        char data[1024];
	        error_code error;
	        while(!_stop)
	        {
	                _socket = boost::shared_ptr <tcp::socket>( new tcp::socket(_service) );
	                _acceptor->accept( *_socket );
	                while(!_stop)
	                {
	
	                        size_t length = _socket->read_some(buffer(data, sizeof(data)/sizeof(data[0])), error);
	                        if (error)
	                        {
	                                break;
	                        }
	                        std::string readdata( data, length );
	                        _data+=readdata;
	                        while ( _data.length() >= HEADER_LEN )
	                        {
	                            int command=-1;
	                            //std::cout << "DATA:["<<_data<<"]"<<std::endl;
	                            if ( _readState == WaitingHeader  )
	                            {
	                                //std::cout << "WaitingHeader"<<std::endl;
	                                std::string header(_data.begin(),_data.begin()+HEADER_LEN);
	                                _data.erase(0,HEADER_LEN);
	
	                                std::stringstream s(header);
	                                _expectedLength=0;
	                                s >>hex;
	                                FixedLengthValue(_expectedLength,4).process(s); 
	                                s >> hex;
	                                FixedLengthValue(command,4).process(s);
	
	
	                                //std::cout << "DATA:["<<_data<<"]"<<_expectedLength<<std::endl;
	                                if ( _expectedLength >= 0)
	                                {
	                                    _readState = WaitingMsgComplete;
	                                }
	                                else
	                                {
	                                    _data.clear();
	                                    break; //todo: error in stream? an now?
	                                }
	
	                            }
	                            if ( _readState == WaitingMsgComplete  )
	                            {
	                                //std::cout << "WaitingMsgComplete"<<std::endl;
	                                if( _data.length() >= (size_t)_expectedLength )
	                                {
	                                    _readState = ProcessingMessage;
	                                }
	                                else break;
	                            }
	                            if ( _readState == ProcessingMessage  )
	                            {
	                                std::string msgdata( _data.begin(), _data.begin()+_expectedLength );
	
	                                //std::cout << "DATA:["<<_data<<"]"<<_expectedLength<<std::endl;
	                                //std::cout << "Processing:["<<msgdata<<"]"<<std::endl<<std::flush;
	                                _data.erase( 0, _expectedLength );
	                                _expectedLength=0;
	                                _readState = WaitingHeader;
	
	                                if ( command == MsgGetVersion )
	                                {
	                                        send( MsgVersion, MoDePP_Version );
	                                }
	                                else if (command == MsgListFunctions)
	                                {
	                                    //std::cout << "Processing MsgListFunctions"<<std::endl;
	                                    foreach (   FuncMapEntry fe, functionMap )
	                                    {
	                                        std::stringstream msgdata;
	                                        msgdata<<hex << setw(4)<<setfill('0')<<fe.first.length()+fe.second->_parameters.length()+1 // fname+" "+params
	                                                <<hex << setw(4)<<setfill('0')<<MsgAddFunction<<fe.first<<" "<<fe.second->_parameters;
	                                        send( msgdata.str() );
	                                    }
	                                }
	                                else if (command == MsgCallFunction)
	                                {
	                                    //std::cout << "Processing MsgCallFunction"<<std::endl;
	                                    std::string fname;
	                                    int len=0;
	
	                                    std::stringstream sstr(msgdata);
	                                    FixedLengthValue(len,4).process( sstr );
	                                    FixedLengthValue(fname,len).process( sstr );
	                                    //std::cout << "   "<<len<<" "<<fname<<std::endl;
	                                    std::string param1,param2,param3,param4,param5;
	                                    std::string *params[]={&param1,&param2,&param3,&param4,&param5};
	
	                                    int pidx=0;
	                                    while ( sstr.good() && !sstr.eof() && pidx < 5 )
	                                    {
	                                        FixedLengthValue(len,4).process( sstr );
	                                        FixedLengthValue(*params[pidx++],len).process( sstr );
	                                    }
	                                    FuncMap::iterator it = functionMap.find( fname );
	                                    if ( it != functionMap.end()  )
	                                    {
	                                            it->second->testFunction(param1,param2,param3,param4,param5);
	                                    }
	                                    else
	                                    {
	                                        cout << "Error! no such Function: "<<fname << endl;
	                                    }                                    
	                                }
	                                else
	                                {
	                                    //todo error! unknown command
	                                    std::cout << "error! unknown command"<<std::endl;
	                                }
	                            }
	
	                        }
	                }
	        }
	        _thread->join();
	}	 
public:	
	///Creates and returns singleton instance
	static MoDePP & instance()
	{
			static 	boost::mutex _mx;
	        static MoDePP inst;
	        
	        boost::mutex::scoped_lock lock(_mx);
	        return inst;
	}
	
	// Starts the server (return value is dummy, required for calling the function as static-initializer).
	int start( unsigned short p )
	{
	         if (! _thread.get() )
	         {
	                _port = p;
	                _acceptor = std::auto_ptr<tcp::acceptor>( new tcp::acceptor( _service, tcp::endpoint(tcp::v4(), _port )) );
	                _thread =  std::auto_ptr<boost::thread>(  new boost::thread ( boost::bind( &MoDePP::doWork, this ) )  );
	         }
	         return 0;
	}
	
	//Stops the server (hardly required in the praxis)
	void stop()
	{
	        _stop=true;
	        if ( _thread.get() )
	        {
	                _service.stop();
	                _thread->join();
	        }
	}
	
	
	void send( const std::string & data )
	{
	        if ( _socket.get() )
	        {
	                //std::cout << "SEND: "<<data << std::endl;
	                write(*_socket, buffer(data));
	        }
	}
	
	
	void send( CommandNumber cmd, int value )
	{
	        if ( _socket.get() )
	        {
	                std::stringstream s;
	                s << value;
	                send ( cmd, s.str() );
	        }
	}
	
	void send( CommandNumber cmd, const std::string & data )
	{
	        if ( _socket.get() )
	        {
	                std::stringstream msgdata;
	    			msgdata<< hex << setw(4) << setfill('0') << data.length()
	                       << hex << setw(4) << setfill('0') << cmd <<data;
	                send( msgdata.str() );
	        }
	}
	
	
	//adds a test function to the list
	void addFunction( const std::string & fname, ITestFunctionWrapper*  fptr )
	{
	        functionMap.insert(std::make_pair(fname, fptr));
	}
	
	void addParamEnum( const std::string & param, const std::string & ename, const VarParam & evalue  )
	{
		_paramValues[param].push_back( std::make_pair( ename, evalue ) );
	}
};

#endif //MODEPP_INCLUDE_MESSAGE_TYPES_ONLY
#endif //HG


