#pragma once
#include "contest_algorithm.h"

namespace contests {

    //! Implementation of contest_algorithm for a basic context exchange.

    //! The receive and transmit exchanges are RS(T) and Serial Number.
    //! Scoring is 1 point per unique contact in a different DXCC entity with 
    //! a multiplier of 1 per DXCC entity per band
    class basic :
        public contest_algorithm
    {
    public:

        //! Contructor.
        
        //! Identifies the receive exchange items as RST_RCVD and SRX.
        //! Identifies the transmit exchange items as RST_SENT and STX.
        //! Adds itself to the global attribute algorithms_.
        basic();

        //! Algorithm specific method to split text into a number of fields.
        
        //! The supplied \a text is parsed as "[RST_RCVD] [SRX]" and
        //! written into \a qso fields of these names.
        virtual void parse_exchange(record* qso, string text);

        //! Algorithm specific method to generate text from a number of fields.
        
        //! The default RS(T) is written into the RST_SENT field of \a qso.
        //! The fields RST_SENT and STX are concatenated and returned from
        //! the method.
        virtual string generate_exchange(record* qso);

        //! Algorithm specific method to score an individual QSO.
        
        //! The fields DXCC and BAND are concatenated to create a 
        //! multiplier. If this multiplier is not present in \a multipliers
        //! it is added and the value 1 is set in the multiplier attribute
        //! of the return value.
        //! If the DXCC entity of \a qso is not the same as the user's
        //! DXCC entity a value of 1 is returned in the qso_points 
        //! attribute of the return value.
        virtual score_result score_qso(record* qso, set<string>& multipliers);

        //! The algorithm uses serial numbers.
        
        //! \return true.
        virtual bool uses_serno() { return true; }

    };

}

