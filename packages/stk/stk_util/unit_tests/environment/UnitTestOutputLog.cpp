/*------------------------------------------------------------------------*/
/*                 Copyright 2010 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <iostream>
#include <stdexcept>
#include <stk_util/environment/OutputLog.hpp>

class UnitTestOutputLog : public CppUnit::TestCase {
private:
  CPPUNIT_TEST_SUITE(UnitTestOutputLog);
  CPPUNIT_TEST(testUnit);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {}

  void tearDown()
  {}

  void testUnit();
};


namespace {

} // namespace <empty>


void UnitTestOutputLog::testUnit()
{
  // Make cout and cerr available as log stream targets.
  stk::register_log_ostream(std::cout, "cout");
  stk::register_log_ostream(std::cerr, "cerr");

  // Test registration, binding, rebinding and unregistration
  {
    std::ostringstream log1;
    std::ostringstream log2;
    
    std::ostream out(std::cout.rdbuf());

    stk::register_ostream(out, "out");

    CPPUNIT_ASSERT(stk::is_registered_ostream("out"));
    
    stk::register_log_ostream(log1, "log1");
    stk::register_log_ostream(log2, "log2");

    stk::bind_output_streams("out>log1");

    out << "stk::bind_output_streams(\"out>log1\");" << std::endl;

    stk::bind_output_streams("out>+log2");
    out << "stk::bind_output_streams(\"out>+log2\");" << std::endl;
    
    stk::bind_output_streams("out>-log1");
    out << "stk::bind_output_streams(\"out>-log1\");" << std::endl;

    stk::bind_output_streams("out>-log2");
    out << "stk::bind_output_streams(\"out>-log2\");" << std::endl;

    std::ostringstream log1_result;
    log1_result << "stk::bind_output_streams(\"out>log1\");" << std::endl
                << "stk::bind_output_streams(\"out>+log2\");" << std::endl;
    
    std::ostringstream log2_result;
    log2_result << "stk::bind_output_streams(\"out>+log2\");" << std::endl
                << "stk::bind_output_streams(\"out>-log1\");" << std::endl;
    
    CPPUNIT_ASSERT_EQUAL(log1_result.str(), log1.str());
    CPPUNIT_ASSERT_EQUAL(log2_result.str(), log2.str());

    stk::unregister_log_ostream(log1);
    stk::unregister_log_ostream(log2);
    stk::unregister_ostream(out);

    CPPUNIT_ASSERT_EQUAL(out.rdbuf(), std::cout.rdbuf());
  }

  // Test logging to a file
  {
    std::ostream out(std::cout.rdbuf());

    stk::register_ostream(out, "out");

    stk::bind_output_streams("log=\"logfile\" out>log");

    CPPUNIT_ASSERT_EQUAL(std::string("logfile"), stk::get_log_path("log")); 
    
    out << "This is a test" << std::endl;

    stk::bind_output_streams("log=\"\"");
    
    stk::unregister_ostream(out);

    std::ostringstream log_result;
    log_result << "This is a test";
    
    std::ifstream log_stream("logfile");
    std::string log_string;
    getline(log_stream, log_string);
    CPPUNIT_ASSERT_EQUAL(log_result.str(), log_string);
  }

  // Test results of unregistration of an output stream bound as a log stream
  {
    std::ostringstream default_log;
    std::ostream out(default_log.rdbuf());
    std::ostream pout(std::cout.rdbuf());

    stk::register_ostream(out, "out");
    stk::register_ostream(pout, "pout");

    //  Constructing the log streams after the registered output stream is not exception safe.
    std::ostringstream log;
    stk::register_log_ostream(log, "log");

    // As a result, this try catch block must be represent to ensure the that unregistration
    // happens correctly.
    try {  
      stk::bind_output_streams("out>pout pout>log");

      out << "This is to out" << std::endl;
      pout << "This is to pout" << std::endl;

      std::ostringstream log_result;
      log_result << "This is to out" << std::endl
                 << "This is to pout" << std::endl;
    
      CPPUNIT_ASSERT_EQUAL(log_result.str(), log.str());

      throw std::exception();
    }
    catch (...) {
    }

    stk::unregister_log_ostream(log);
    stk::unregister_ostream(pout);
    stk::unregister_ostream(out);

    out << "This is to out" << std::endl;

    std::ostringstream log_result;
    log_result << "This is to out" << std::endl;
    CPPUNIT_ASSERT_EQUAL(log_result.str(), default_log.str());
  }

  // Test exception of registration with existing name
  {
    std::ostringstream log1;
    std::ostringstream log2;
    
    std::ostream out(std::cout.rdbuf());
    std::ostream pout(std::cout.rdbuf());

    stk::register_ostream(out, "out");
    CPPUNIT_ASSERT_THROW(stk::register_ostream(pout, "out"), std::runtime_error);

    CPPUNIT_ASSERT_EQUAL(&out, stk::get_ostream_ostream("out"));

    stk::register_log_ostream(log1, "log");
    
    CPPUNIT_ASSERT_THROW(stk::register_log_ostream(log2, "log"), std::runtime_error);

    CPPUNIT_ASSERT_THROW(stk::bind_output_streams("badout>log"), std::runtime_error);
    
    CPPUNIT_ASSERT_THROW(stk::bind_output_streams("out>badlog"), std::runtime_error);

    stk::unregister_log_ostream(log1);
    stk::unregister_ostream(out);
  }
}

  CPPUNIT_TEST_SUITE_REGISTRATION(UnitTestOutputLog);
