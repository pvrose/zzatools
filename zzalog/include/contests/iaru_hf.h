#pragma once

#include "contest_algorithm.h"

namespace contests {

    //! Implementation of \a contest_algorithm for the IARU HF Championship (IARU-HF).
    //! Exchange is RS(T) and ITU Zone.
    //! 
    //! Scoring involves comparison of ITU Zone and continent.
    class iaru_hf :
        public contest_algorithm
    {
    public:

       //! Contructor.
       //! Identifies the receive items as RST_RCVD and ITUZ and CONT
       //! Identifies the transmit items as RST_SENT and MY_ITU_ZONE and APP_ZZA_MY_CONT.
       //! Adds itself to the global attribute algorithms_.
        iaru_hf();

        //! Algorithm specific method to split text into a number of fields.
        //! The value in \a text is split into the RST_RCVD and ITUZ fields
        //! of the supplied \a qso.
        virtual void parse_exchange(record * qso, string text);

        //! Algorithm specific method to generate text from a number of fields.
        //! The \a qso fields APP_ZZA_MY_CONT and MY_ITU_ZONE are written
        //! from the user's details. The \a qso fields RST_SENT and
        //! MY_ITU_ZONE are conctenated and returned from the method.
        virtual string generate_exchange(record * qso);

        //! Algorithm specific method to score an individual QSO.
        //! IARU HF 
        //! - 1 pt per QSO in same ITU zone.
        //! - 1 pt per QSO with HQ or IARU official (ITUZ != numeric).
        //! - 3 pt per QSO same continent (different ITUZ).
        //! - 5 pt per QSO in different continent and in different ITUZ.
        //! - Multiplier = count(ITUZ x BAND).
        //! 
        //! \return { points, multiplier }.
        virtual score_result score_qso(record * qso, set<string>& multipliers);

        //! This algorithm does not use serial numbers.
        //! \return false
        virtual bool uses_serno() { return false; }
    };

}


