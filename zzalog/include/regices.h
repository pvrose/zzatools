#pragma once

#include <regex>

using namespace std;

// regular expressions used in validation
// e.g. 3.1.0
const basic_regex<char> REGEX_ADIF_VERSION("[0-9]\\.[0-9]\\.[0-9]");
// One of Y, y, N, n
const basic_regex<char> REGEX_BOOLEAN("[YyNn]");
// A signed number with an optional decimal point
const basic_regex<char> REGEX_NUMERIC("-?([0-9]*(.?[0-9]*)|[0-9]+)");
// Valid date - YYYYMMDD from 1930
const basic_regex<char> REGEX_DATE("(19[0-9]{2}|[2-9][0-9]{3})(0[1-9]|1[0-2])(0[1-9]|[1-2][0-9]|3[01])");
// Valid Time - HHMM or HHMMSS
const basic_regex<char> REGEX_TIME("([01][0-9]|2[0-3])[0-5][0-9]([0-5][0-9]){0,1}");
// A sequence of ASCII characters
const basic_regex<char> REGEX_STRING("[\\x20-\\x7E]+");
// Sequence of Character + CR/LF
const basic_regex<char> REGEX_MULTILINE("([\\x20-\\x7E]|\\x0d\\x0a)+");
// Sequence of Character + CR or LF
const basic_regex<char> REGEX_BAD_MULTILINE("([\\x20-\\x7E]|\\x0d|\\x0a)+");
// A sequence of IntlCharacter (UTF-8) 
const basic_regex<char> REGEX_INTL_STRING("([\\xF0-\\xFF][\\x80-\\xBF]{3}|[\\xE0-\\xEF][\\x80-\\xBF]{2}|[\\xC0-\\xDF][\\x80-\\xBF]|[\\x20-\\x7F]|[\\x80-\\xBF])+");
// A single IntlCharacter (UTF-8) 
const basic_regex<char> REGEX_INTL_CHAR("[\\xF0-\\xFF][\\x80-\\xBF]{3}|[\\xE0-\\xEF][\\x80-\\xBF]{2}|[\\xC0-\\xDF][\\x80-\\xBF]|[\\x20-\\x7F]|[\\x80-\\xBF]");
// Sequence of IntlCharacter (UTF-8) plus CR/LF
// const basic_regex<char> REGEX_INTL_MULTILINE("([\\x20-\\x7F]|[\\x80-\\xBF]|\\x0d\\x0a)+");
const basic_regex<char> REGEX_INTL_MULTILINE("([\\xF0-\\xFF][\\x80-\\xBF]{3}|[\\xE0-\\xEF][\\x80-\\xBF]{2}|[\\xC0-\\xDF][\\x80-\\xBF]|[\\x20-\\x7F]|[\\x80-\\xBF]|\\x0d\\x0a)+");
const basic_regex<char> REGEX_BAD_INTL_MULTILINE("([\\xF0-\\xFF][\\x80-\\xBF]{3}|[\\xE0-\\xEF][\\x80-\\xBF]{2}|[\\xC0-\\xDF][\\x80-\\xBF]|[\\x20-\\x7F]|[\\x80-\\xBF]|\\x0d|\\x0a)+");
// XDDD MM.MMM   X = NESW, DDD 0->180, MM.MMM 00.000->59.999
const basic_regex<char> REGEX_LAT_LONG("[NESWnesw](0[0-9][0-9]|1[0-7][0-9]|180) [0-5][0-9]\\.[0-9]{3}");
// A signed integer
const basic_regex<char> REGEX_POS_INTEGER("[0-9]+");
// 2- 4- 6- or 8- character maidenhead locator, extension to 10 or 12 characters
const basic_regex<char> REGEX_GRIDSQUARE("[a-rA-R]{2}(|[0-9]{2}(|[a-xA-X]{2}(|[0-9]{2})))");
const basic_regex<char> REGEX_GRIDSQUARE_EXT("[a-xA-X]{2}(|[0-9]{2})");
// A signed integer
const basic_regex<char> REGEX_INTEGER("-?[0-9]+");
// CC-XXX - CC is a valid Continent, XXX is a valid number 000-999
const basic_regex<char> REGEX_IOTA("[A-Za-z]{2}-[0-9]{3}");
// A single decimal digit
const basic_regex<char> REGEX_DIGIT("[0-9]");
// A single non-control ASCII character
const basic_regex<char> REGEX_CHAR("[\\x20-\\x7E]");
// D/RR-XXX - D is a country id (nickname), RR is two letter range ID, XXX is a three digit summit ID
const basic_regex<char> REGEX_SOTA("[A-Za-z0-9]{1,3}/[A-Za-z]{2}-[0-9]{3}");
// 4-character gridsqaue
const basic_regex<char> REGEX_GRIDSQUARE4("[a-rA-R]{2}[0-9]{2}");
// FT8 reports
const basic_regex<char> REGEX_REPORT("[-+][0-9]{1,2}");
const basic_regex<char> REGEX_ROGER("R[-+][0-9]{1,2}"); 
// CQ target - CQ or CQ xxxx
const basic_regex<char> REGEX_CQ("CQ(| [A-Z][0-9]{1,4})");
