// ======================================================================
// \title  Generic_fss.hpp
// \author jstar
// \brief  hpp file for Generic_fss component implementation class
// ======================================================================

#ifndef Components_Generic_fss_HPP
#define Components_Generic_fss_HPP

#include "fss_src/Generic_fssComponentAc.hpp"

namespace Components {

  class Generic_fss :
    public Generic_fssComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Generic_fss object
      Generic_fss(
          const char* const compName //!< The component name
      );

      //! Destroy Generic_fss object
      ~Generic_fss();

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------

      void REQUEST_DATA_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq //!< The command sequence number
      ) override;

      void updateData_handler(
        const NATIVE_INT_TYPE portNum, //!< The port number
        NATIVE_UINT_TYPE context //!< The call order
      ) override;

  };

}

#endif
