#pragma once

#include "contest_algorithm.h"

namespace contests {


    class iaru_hf :
        public contest_algorithm
    {
    public:
        iaru_hf();

        // Algorithm specific method to split text into a number of fields
        virtual void parse_exchange(record * qso, string text);
        // Algorithm specific method to generate text from a number of fieds
        virtual string generate_exchange(record * qso);
        // Algorithm specific method to score an individual QSO
        virtual score_result score_qso(record * qso, const set<string>&multipliers);
        // Algorithmic specific method
        virtual bool uses_serno() { return false; }
    };

}


