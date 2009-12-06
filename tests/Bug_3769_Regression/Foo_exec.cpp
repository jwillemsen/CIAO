// -*- C++ -*-
// $Id$

/**
 * Code generated by the The ACE ORB (TAO) IDL Compiler v1.7.4
 * TAO and the TAO IDL Compiler have been developed by:
 *       Center for Distributed Object Computing
 *       Washington University
 *       St. Louis, MO
 *       USA
 *       http://www.cs.wustl.edu/~schmidt/doc-center.html
 * and
 *       Distributed Object Computing Laboratory
 *       University of California at Irvine
 *       Irvine, CA
 *       USA
 *       http://doc.ece.uci.edu/
 * and
 *       Institute for Software Integrated Systems
 *       Vanderbilt University
 *       Nashville, TN
 *       USA
 *       http://www.isis.vanderbilt.edu/
 *
 * Information about TAO is available at:
 *     http://www.cs.wustl.edu/~schmidt/TAO.html
 **/

// TAO_IDL - Generated from
// .\be\be_codegen.cpp:1278

#include "Foo_exec.h"

namespace CIAO_Foo_Impl
{
  //============================================================
  // Component Executor Implementation Class: Foo_exec_i
  //============================================================
  
  Foo_exec_i::Foo_exec_i (void)
  {
  }
  
  Foo_exec_i::~Foo_exec_i (void)
  {
  }
  
  // Supported operations and attributes.
  
  // Component attributes and port operations.
  
  ::CORBA::Short
  Foo_exec_i::my_short (void)
  {
    /* Your code here. */
    return 0;
  }
  
  void
  Foo_exec_i::my_short (
    ::CORBA::Short my_short)
  {
    if(my_short != 22)
      ACE_ERROR ((LM_ERROR, "ERROR: my_short != 22, it is %d\n", my_short));
  }
  
  ::CORBA::Long
  Foo_exec_i::my_long (void)
  {
    /* Your code here. */
    return 0;
  }
  
  void
  Foo_exec_i::my_long (
    ::CORBA::Long my_long)
  {
    if(my_long != 33)
      ACE_ERROR ((LM_ERROR, "ERROR: my_long != 33, it is %d\n", my_long));
  }
  
  ::CORBA::Float
  Foo_exec_i::my_float (void)
  {
    /* Your code here. */
    return 0.0f;
  }
  
  void
  Foo_exec_i::my_float (
    ::CORBA::Float my_float)
  {
    if(my_float != 45.67F)
      ACE_ERROR ((LM_ERROR, "ERROR: my_float != 45.67, it is %f\n", my_float));
  }
  
  ::CORBA::Double
  Foo_exec_i::my_double (void)
  {
    /* Your code here. */
    return 0.0;
  }
  
  void
  Foo_exec_i::my_double (
    ::CORBA::Double my_double)
  {
    if(my_double != 56.78)
      ACE_ERROR ((LM_ERROR, "ERROR: my_double != 56.78, it is %f\n", my_double));
  }
  
  // Operations from Components::SessionComponent.
  
  void
  Foo_exec_i::set_session_context (
    ::Components::SessionContext_ptr ctx)
  {
    this->context_ =
      ::CCM_Foo_Context::_narrow (ctx);
    
    if ( ::CORBA::is_nil (this->context_.in ()))
      {
        throw ::CORBA::INTERNAL ();
      }
  }
  
  void
  Foo_exec_i::configuration_complete (void)
  {
    /* Your code here. */
  }
  
  void
  Foo_exec_i::ccm_activate (void)
  {
    /* Your code here. */
  }
  
  void
  Foo_exec_i::ccm_passivate (void)
  {
    /* Your code here. */
  }
  
  void
  Foo_exec_i::ccm_remove (void)
  {
    /* Your code here. */
  }
  
  extern "C" FOO_EXEC_Export ::Components::EnterpriseComponent_ptr
  create_Foo_Impl (void)
  {
    ::Components::EnterpriseComponent_ptr retval =
      ::Components::EnterpriseComponent::_nil ();
    
    ACE_NEW_NORETURN (
      retval,
      Foo_exec_i);
    
    return retval;
  }
}

