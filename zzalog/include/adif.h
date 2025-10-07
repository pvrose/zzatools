#pragma once

#include <cstdint>
#include <map>
#include <string>

//! This namespace holds the ADIF version dependent maps
namespace ADIF {

  //! Populated with all the valid ADIF field names plus ZLG Applixation specific ones
  enum field_t : uint16_t {
    ADDRESS,     //!<the contacted station's complete mailing address: full name, street address, city, postal code, and country
    ADDRESS_INTL,     //!<the contacted station's complete mailing address: full name, street address, city, postal code, and country
    ADIF_VER,     //!<identifies the version of ADIF used in this file in the format X.Y.Z where X is an integer designating the ADIF epoch Y is an integer between 0 and 9 designating the major version Z is an integer between 0 and 9 designating the minor version
    AGE,     //!<the contacted station's operator's age in years in the range 0 to 120 (inclusive)
    ALTITUDE,     //!<the height of the contacted station in meters relative to Mean Sea Level (MSL). For example 1.5 km is <ALTITUDE:4>1500 and 10.5 m is <ALTITUDE:4>10.5
    ANT_AZ,     //!<the logging station's antenna azimuth, in degrees with a value between 0 to 360 (inclusive). Values outside this range are import-only and must be normalized for export (e.g. 370 is exported as 10). True north is 0 degrees with values increasing in a clockwise direction.
    ANT_EL,     //!<the logging station's antenna elevation, in degrees with a value between -90 to 90 (inclusive). Values outside this range are import-only and must be normalized for export (e.g. 100 is exported as 80). The horizon is 0 degrees with values increasing as the angle moves in an upward direction.
    ANT_PATH,     //!<the signal path
    APP_LOTW_LASTQSL,     //!<Application-defined field
    APP_LOTW_NUMREC,     //!<Application-defined field
    APP_QRZLOG_LOGID,     //!<Application-defined field
    APP_QRZLOG_QSLDATE,     //!<Application-defined field
    APP_QRZLOG_STATUS,     //!<Application-defined field
    APP_ZZA_CQ,     //!<Application-defined field
    APP_ZZA_ECARD,     //!<Application-defined field
    APP_ZZA_EQSL_MSG,     //!<Application-defined field
    APP_ZZA_ERROR,     //!<Application-defined field
    APP_ZZA_MY_CONT,     //!<Application-defined field
    APP_ZZA_MY_WAB,     //!<Application-defined field
    APP_ZZA_NUMRECORDS,     //!<Application-defined field
    APP_ZZA_OP,     //!<Application-defined field
    APP_ZZA_OPERATOR,     //!<Application-defined field
    APP_ZZA_OP_DESCR,     //!<Application-defined field
    APP_ZZA_PFX,     //!<Application-defined field
    APP_ZZA_QSL_ROUTE,     //!<Application-defined field
    APP_ZZA_QTH,     //!<Application-defined field
    APP_ZZA_QTH_DESCR,     //!<Application-defined field
    ARRL_SECT,     //!<the contacted station's ARRL section
    AWARD_GRANTED,     //!<the list of awards granted by a sponsor. note that this field might not be used in a QSO record. It might be used to convey information about a user's "Award Account" between an award sponsor and the user. For example, in response to a request "send me a list of the awards granted to AA6YQ", this might be received: <CALL:5>AA6YQ <AWARD_GRANTED:64>ADIF_CENTURY_BASIC,ADIF_CENTURY_SILVER,ADIF_SPECTRUM_100-160m
    AWARD_SUBMITTED,     //!<the list of awards submitted to a sponsor. note that this field might not be used in a QSO record. It might be used to convey information about a user's "Award Account" between an award sponsor and the user. For example, AA6YQ might submit a request for awards by sending the following: <CALL:5>AA6YQ <AWARD_SUBMITTED:64>ADIF_CENTURY_BASIC,ADIF_CENTURY_SILVER,ADIF_SPECTRUM_100-160m
    A_INDEX,     //!<the geomagnetic A index at the time of the QSO in the range 0 to 400 (inclusive)
    BAND,     //!<QSO Band
    BAND_RX,     //!<in a split frequency QSO, the logging station's receiving band
    CALL,     //!<the contacted station's callsign
    CHECK,     //!<contest check (e.g. for ARRL Sweepstakes)
    CLASS,     //!<contest class (e.g. for ARRL Field Day)
    CLUBLOG_QSO_UPLOAD_DATE,     //!<the date the QSO was last uploaded to the Club Log online service
    CLUBLOG_QSO_UPLOAD_STATUS,     //!<the upload status of the QSO on the Club Log online service
    CNTY,     //!<the contacted station's Secondary Administrative Subdivision (e.g. US county, JA Gun), in the specified format
    CNTY_ALT,     //!<a semicolon (;) delimited, unordered list of Secondary Administrative Subdivision Alt codes for the contacted station See the Data Type for details.
    COMMENT,     //!<comment field for QSO for a message to be incorporated in a paper or electronic QSL for the contacted station's operator, use the QSLMSG field recommended use: information of interest to the contacted station's operator
    COMMENT_INTL,     //!<comment field for QSO for a message to be incorporated in a paper or electronic QSL for the contacted station's operator, use the QSLMSG_INTL field recommended use: information of interest to the contacted station's operator
    CONT,     //!<the contacted station's Continent
    CONTACTED_OP,     //!<the callsign of the individual operating the contacted station
    CONTEST_ID,     //!<QSO Contest Identifier use enumeration values for interoperability
    COUNTRY,     //!<the contacted station's DXCC entity name
    COUNTRY_INTL,     //!<the contacted station's DXCC entity name
    CQZ,     //!<the contacted station's CQ Zone in the range 1 to 40 (inclusive)
    CREATED_TIMESTAMP,     //!<identifies the UTC date and time that the file was created in the format of 15 characters YYYYMMDD HHMMSS where YYYYMMDD is a Date data type HHMMSS is a 6 character Time data type
    CREDIT_GRANTED,     //!<the list of credits granted to this QSO Use of data type AwardList and enumeration Award are import-only
    CREDIT_SUBMITTED,     //!<the list of credits sought for this QSO Use of data type AwardList and enumeration Award are import-only
    DARC_DOK,     //!<the contacted station's DARC DOK (District Location Code) A DOK comprises letters and numbers, e.g. <DARC_DOK:3>A01
    DCL_QSLRDATE,     //!<date QSL received from DCL (only valid if DCL_QSL_RCVD is Y, I, or V)(V import-only)
    DCL_QSLSDATE,     //!<date QSL sent to DCL (only valid if DCL_QSL_SENT is Y, Q, or I)
    DCL_QSL_RCVD,     //!<DCL QSL received status Default Value: N
    DCL_QSL_SENT,     //!<DCL QSL sent status Default Value: N
    DISTANCE,     //!<the distance between the logging station and the contacted station in kilometers via the specified signal path with a value greater than or equal to 0
    DXCC,     //!<the contacted station's DXCC Entity Code <DXCC:1>0 means that the contacted station is known not to be within a DXCC entity.
    EMAIL,     //!<the contacted station's email address
    EQSL_AG,     //!<indicates whether the QSO is known to be "Authenticity Guaranteed" by eQSL
    EQSL_QSLRDATE,     //!<date QSL received from eQSL.cc (only valid if EQSL_QSL_RCVD is Y, I, or V)(V import-only)
    EQSL_QSLSDATE,     //!<date QSL sent to eQSL.cc (only valid if EQSL_QSL_SENT is Y, Q, or I)
    EQSL_QSL_RCVD,     //!<eQSL.cc QSL received status instead of V (import-only) use <CREDIT_GRANTED:42>CQWAZ:eqsl,CQWAZ_BAND:eqsl,CQWAZ_MODE:eqsl Default Value: N
    EQSL_QSL_SENT,     //!<eQSL.cc QSL sent status Default Value: N
    EQ_CALL,     //!<the contacted station's owner's callsign
    FISTS,     //!<the contacted station's FISTS CW Club member number with a value greater than 0.
    FISTS_CC,     //!<the contacted station's FISTS CW Club Century Certificate (CC) number with a value greater than 0.
    FORCE_INIT,     //!<new EME "initial"
    FREQ,     //!<QSO frequency in Megahertz
    FREQ_RX,     //!<in a split frequency QSO, the logging station's receiving frequency in Megahertz
    GRIDSQUARE,     //!<the contacted station's 2-character, 4-character, 6-character, or 8-character Maidenhead Grid Square For 10 or 12 character locators, store the first 8 characters in GRIDSQUARE and the additional 2 or 4 characters in the GRIDSQUARE_EXT field
    GRIDSQUARE_EXT,     //!<for a contacted station's 10-character Maidenhead locator, supplements the GRIDSQUARE field by containing characters 9 and 10. For a contacted station's 12-character Maidenhead locator, supplements the GRIDSQUARE field by containing characters 9, 10, 11 and 12. Characters 9 and 10 are case-insensitive ASCII letters in the range A-X. Characters 11 and 12 are Digits in the range 0 to 9. On export, the field length must be 2 or 4. On import, if the field length is greater than 4, the additional characters must be ignored. Example of exporting the 10-character locator FN01MH42BQ: <GRIDSQUARE:8>FN01MH42 <GRIDSQUARE_EXT:2>BQ
    GUEST_OP,     //!<import-only: use OPERATOR instead
    HAMLOGEU_QSO_UPLOAD_DATE,     //!<the date the QSO was last uploaded to the HAMLOG.EU online service
    HAMLOGEU_QSO_UPLOAD_STATUS,     //!<the upload status of the QSO on the HAMLOG.EU online service
    HAMQTH_QSO_UPLOAD_DATE,     //!<the date the QSO was last uploaded to the HamQTH.com online service
    HAMQTH_QSO_UPLOAD_STATUS,     //!<the upload status of the QSO on the HamQTH.com online service
    HRDLOG_QSO_UPLOAD_DATE,     //!<the date the QSO was last uploaded to the HRDLog.net online service
    HRDLOG_QSO_UPLOAD_STATUS,     //!<the upload status of the QSO on the HRDLog.net online service
    IOTA,     //!<the contacted station's IOTA designator, in format CC-XXX, where CC is a member of the Continent enumeration XXX is the island group designator, where 1 <= XXX <= 999 [use leading zeroes]
    IOTA_ISLAND_ID,     //!<the contacted station's IOTA Island Identifier, an 8-digit integer in the range 1 to 99999999 [leading zeroes optional]
    ITUZ,     //!<the contacted station's ITU zone in the range 1 to 90 (inclusive)
    K_INDEX,     //!<the geomagnetic K index at the time of the QSO in the range 0 to 9 (inclusive)
    LAT,     //!<the contacted station's latitude
    LON,     //!<the contacted station's longitude
    LOTW_QSLRDATE,     //!<date QSL received from ARRL Logbook of the World (only valid if LOTW_QSL_RCVD is Y, I, or V)(V import-only)
    LOTW_QSLSDATE,     //!<date QSL sent to ARRL Logbook of the World (only valid if LOTW_QSL_SENT is Y, Q, or I)
    LOTW_QSL_RCVD,     //!<ARRL Logbook of the World QSL received status instead of V (import-only) use <CREDIT_GRANTED:39>DXCC:lotw,DXCC_BAND:lotw,DXCC_MODE:lotw Default Value: N
    LOTW_QSL_SENT,     //!<ARRL Logbook of the World QSL sent status Default Value: N
    MAX_BURSTS,     //!<maximum length of meteor scatter bursts heard by the logging station, in seconds with a value greater than or equal to 0
    MODE,     //!<QSO Mode
    MORSE_KEY_INFO,     //!<details of the contacted station's Morse key (e.g. make, model, etc). Example: <MORSE_KEY_INFO:16>Begali Sculpture
    MORSE_KEY_TYPE,     //!<the contacted station's Morse key type (e.g. straight key, bug, etc). Example for a dual-lever paddle: <MORSE_KEY_TYPE:2>DP
    MS_SHOWER,     //!<For Meteor Scatter QSOs, the name of the meteor shower in progress
    MY_ALTITUDE,     //!<the height of the logging station in meters relative to Mean Sea Level (MSL). For example 1.5 km is <MY_ALTITUDE:4>1500 and 10.5 m is <MY_ALTITUDE:4>10.5
    MY_ANTENNA,     //!<the logging station's antenna
    MY_ANTENNA_INTL,     //!<the logging station's antenna
    MY_ARRL_SECT,     //!<the logging station's ARRL section
    MY_CITY,     //!<the logging station's city
    MY_CITY_INTL,     //!<the logging station's city
    MY_CNTY,     //!<the logging station's Secondary Administrative Subdivision (e.g. US county, JA Gun), in the specified format
    MY_CNTY_ALT,     //!<a semicolon (;) delimited, unordered list of Secondary Administrative Subdivision Alt codes for the logging station See the Data Type for details.
    MY_COUNTRY,     //!<the logging station's DXCC entity name
    MY_COUNTRY_INTL,     //!<the logging station's DXCC entity name
    MY_CQ_ZONE,     //!<the logging station's CQ Zone in the range 1 to 40 (inclusive)
    MY_DARC_DOK,     //!<the logging station's DARC DOK (District Location Code) A DOK comprises letters and numbers, e.g. <MY_DARC_DOK:3>A01
    MY_DXCC,     //!<the logging station's DXCC Entity Code <MY_DXCC:1>0 means that the logging station is known not to be within a DXCC entity.
    MY_FISTS,     //!<the logging station's FISTS CW Club member number with a value greater than 0.
    MY_GRIDSQUARE,     //!<the logging station's 2-character, 4-character, 6-character, or 8-character Maidenhead Grid Square For 10 or 12 character locators, store the first 8 characters in MY_GRIDSQUARE and the additional 2 or 4 characters in the MY_GRIDSQUARE_EXT field
    MY_GRIDSQUARE_EXT,     //!<for a logging station's 10-character Maidenhead locator, supplements the MY_GRIDSQUARE field by containing characters 9 and 10. For a logging station's 12-character Maidenhead locator, supplements the MY_GRIDSQUARE field by containing characters 9, 10, 11 and 12. Characters 9 and 10 are case-insensitive ASCII letters in the range A-X. Characters 11 and 12 are Digits in the range 0 to 9. On export, the field length must be 2 or 4. On import, if the field length is greater than 4, the additional characters must be ignored. Example of exporting the 10-character locator FN01MH42BQ: <MY_GRIDSQUARE:8>FN01MH42 <MY_GRIDSQUARE_EXT:2>BQ
    MY_IOTA,     //!<the logging station's IOTA designator, in format CC-XXX, where CC is a member of the Continent enumeration XXX is the island group designator, where 1 <= XXX <= 999 [use leading zeroes]
    MY_IOTA_ISLAND_ID,     //!<the logging station's IOTA Island Identifier, an 8-digit integer in the range 1 to 99999999 [leading zeroes optional]
    MY_ITU_ZONE,     //!<the logging station's ITU zone in the range 1 to 90 (inclusive)
    MY_LAT,     //!<the logging station's latitude
    MY_LON,     //!<the logging station's longitude
    MY_MORSE_KEY_INFO,     //!<details of the logging station's Morse key (e.g. make, model, etc). Example: <MY_MORSE_KEY_INFO:16>Begali Sculpture
    MY_MORSE_KEY_TYPE,     //!<the logging station's Morse key type (e.g. straight key, bug, etc). Example for a dual-lever paddle: <MORSE_KEY_TYPE:2>DP
    MY_NAME,     //!<the logging operator's name
    MY_NAME_INTL,     //!<the logging operator's name
    MY_POSTAL_CODE,     //!<the logging station's postal code
    MY_POSTAL_CODE_INTL,     //!<the logging station's postal code
    MY_POTA_REF,     //!<a comma-delimited list of one or more of the logging station's POTA (Parks on the Air) reference(s). Examples: <MY_POTA_REF:6>K-0059 <MY_POTA_REF:7>K-10000 <MY_POTA_REF:40>K-0817,K-4566,K-4576,K-4573,K-4578@US-WY
    MY_RIG,     //!<description of the logging station's equipment
    MY_RIG_INTL,     //!<description of the logging station's equipment
    MY_SIG,     //!<special interest activity or event
    MY_SIG_INFO,     //!<special interest activity or event information
    MY_SIG_INFO_INTL,     //!<special interest activity or event information
    MY_SIG_INTL,     //!<special interest activity or event
    MY_SOTA_REF,     //!<the logging station's International SOTA Reference.
    MY_STATE,     //!<the code for the logging station's Primary Administrative Subdivision (e.g. US State, JA Island, VE Province)
    MY_STREET,     //!<the logging station's street
    MY_STREET_INTL,     //!<the logging station's street
    MY_USACA_COUNTIES,     //!<two US counties in the case where the logging station is located on a border between two counties, representing counties that the contacted station may claim for the CQ Magazine USA-CA award program. E.g. MA,Franklin:MA,Hampshire
    MY_VUCC_GRIDS,     //!<two or four adjacent Maidenhead grid locators, each four or six characters long, representing the logging station's grid squares that the contacted station may claim for the ARRL VUCC award program. E.g. EM98,FM08,EM97,FM07
    MY_WWFF_REF,     //!<the logging station's WWFF (World Wildlife Flora & Fauna) reference
    NAME,     //!<the contacted station's operator's name
    NAME_INTL,     //!<the contacted station's operator's name
    NOTES,     //!<QSO notes recommended use: information of interest to the logging station's operator
    NOTES_INTL,     //!<QSO notes recommended use: information of interest to the logging station's operator
    NR_BURSTS,     //!<the number of meteor scatter bursts heard by the logging station with a value greater than or equal to 0
    NR_PINGS,     //!<the number of meteor scatter pings heard by the logging station with a value greater than or equal to 0
    OPERATOR,     //!<the logging operator's callsign if STATION_CALLSIGN is absent, OPERATOR shall be treated as both the logging station's callsign and the logging operator's callsign
    OWNER_CALLSIGN,     //!<the callsign of the owner of the station used to log the contact (the callsign of the OPERATOR's host) if OWNER_CALLSIGN is absent, STATION_CALLSIGN shall be treated as both the logging station's callsign and the callsign of the owner of the station
    PFX,     //!<the contacted station's WPX prefix
    POTA_REF,     //!<a comma-delimited list of one or more of the contacted station's POTA (Parks on the Air) reference(s). Examples: <POTA_REF:6>K-5033 <POTA_REF:13>VE-5082@CA-AB <POTA_REF:40>K-0817,K-4566,K-4576,K-4573,K-4578@US-WY
    PRECEDENCE,     //!<contest precedence (e.g. for ARRL Sweepstakes)
    PROGRAMID,     //!<identifies the name of the logger, converter, or utility that created or processed this ADIF file To help avoid name clashes, the ADIF PROGRAMID Register provides a voluntary list of PROGRAMID values.
    PROGRAMVERSION,     //!<identifies the version of the logger, converter, or utility that created or processed this ADIF file
    PROP_MODE,     //!<QSO propagation mode
    PUBLIC_KEY,     //!<public encryption key
    QRZCOM_QSO_DOWNLOAD_DATE,     //!<date QSO downloaded from QRZ.COM logbook
    QRZCOM_QSO_DOWNLOAD_STATUS,     //!<QRZ.COM logbook QSO download status
    QRZCOM_QSO_UPLOAD_DATE,     //!<the date the QSO was last uploaded to the QRZ.COM online service
    QRZCOM_QSO_UPLOAD_STATUS,     //!<the upload status of the QSO on the QRZ.COM online service
    QSLMSG,     //!<a message for the contacted station's operator to be incorporated in a paper or electronic QSL
    QSLMSG_INTL,     //!<a message for the contacted station's operator to be incorporated in a paper or electronic QSL
    QSLMSG_RCVD,     //!<a message addressed to the logging station's operator incorporated in a paper or electronic QSL
    QSLRDATE,     //!<QSL received date (only valid if QSL_RCVD is Y, I, or V)(V import-only)
    QSLSDATE,     //!<QSL sent date (only valid if QSL_SENT is Y, Q, or I)
    QSL_RCVD,     //!<QSL received status instead of V (import-only) use <CREDIT_GRANTED:39>DXCC:card,DXCC_BAND:card,DXCC_MODE:card Default Value: N
    QSL_RCVD_VIA,     //!<if QSL_RCVD is set to 'Y' or 'V', the means by which the QSL was received by the logging station; otherwise, the means by which the logging station requested or intends to request that the QSL be conveyed. (Note: 'V' is import-only) use of M (manager) is import-only
    QSL_SENT,     //!<QSL sent status Default Value: N
    QSL_SENT_VIA,     //!<if QSL_SENT is set to 'Y', the means by which the QSL was sent by the logging station; otherwise, the means by which the logging station intends to convey the QSL use of M (manager) is import-only
    QSL_VIA,     //!<the contacted station's QSL route
    QSO_COMPLETE,     //!<indicates whether the QSO was complete from the perspective of the logging station Y - yes N - no NIL - not heard ? - uncertain
    QSO_DATE,     //!<date on which the QSO started
    QSO_DATE_OFF,     //!<date on which the QSO ended
    QSO_RANDOM,     //!<indicates whether the QSO was random or scheduled
    QTH,     //!<the contacted station's city
    QTH_INTL,     //!<the contacted station's city
    REGION,     //!<the contacted station's WAE or CQ entity contained within a DXCC entity. the value None indicates that the WAE or CQ entity is the DXCC entity in the DXCC field. nothing can be inferred from the absence of the REGION field
    RIG,     //!<description of the contacted station's equipment
    RIG_INTL,     //!<description of the contacted station's equipment
    RST_RCVD,     //!<signal report from the contacted station
    RST_SENT,     //!<signal report sent to the contacted station
    RX_PWR,     //!<the contacted station's transmitter power in Watts with a value greater than or equal to 0
    SAT_MODE,     //!<satellite mode - a code representing the satellite's uplink band and downlink band
    SAT_NAME,     //!<name of satellite
    SFI,     //!<the solar flux at the time of the QSO in the range 0 to 300 (inclusive).
    SIG,     //!<the name of the contacted station's special activity or interest group
    SIG_INFO,     //!<information associated with the contacted station's activity or interest group
    SIG_INFO_INTL,     //!<information associated with the contacted station's activity or interest group
    SIG_INTL,     //!<the name of the contacted station's special activity or interest group
    SILENT_KEY,     //!<'Y' indicates that the contacted station's operator is now a Silent Key.
    SKCC,     //!<the contacted station's Straight Key Century Club (SKCC) member information
    SOTA_REF,     //!<the contacted station's International SOTA Reference.
    SRX,     //!<contest QSO received serial number with a value greater than or equal to 0
    SRX_STRING,     //!<contest QSO received information use Cabrillo format to convey contest information for which ADIF fields are not specified in the event of a conflict between information in a dedicated contest field and this field, information in the dedicated contest field shall prevail
    STATE,     //!<the code for the contacted station's Primary Administrative Subdivision (e.g. US State, JA Island, VE Province)
    STATION_CALLSIGN,     //!<the logging station's callsign (the callsign used over the air) if STATION_CALLSIGN is absent, OPERATOR shall be treated as both the logging station's callsign and the logging operator's callsign
    STX,     //!<contest QSO transmitted serial number with a value greater than or equal to 0
    STX_STRING,     //!<contest QSO transmitted information use Cabrillo format to convey contest information for which ADIF fields are not specified in the event of a conflict between information in a dedicated contest field and this field, information in the dedicated contest field shall prevail
    SUBMODE,     //!<QSO Submode use enumeration values for interoperability
    SWL,     //!<indicates that the QSO information pertains to an SWL report
    TEN_TEN,     //!<Ten-Ten number with a value greater than 0
    TIME_OFF,     //!<HHMM or HHMMSS in UTC in the absence of <QSO_DATE_OFF>, the QSO duration is less than 24 hours. For example, the following is a QSO starting at 14 July 2020 23:55 and finishing at 15 July 2020 01:00: <QSO_DATE:8>20200714 <TIME_ON:4>2355 <TIME_OFF:4>0100
    TIME_ON,     //!<HHMM or HHMMSS in UTC
    TX_PWR,     //!<the logging station's power in Watts with a value greater than or equal to 0
    UKSMG,     //!<the contacted station's UKSMG member number with a value greater than 0
    USACA_COUNTIES,     //!<two US counties in the case where the contacted station is located on a border between two counties, representing counties credited to the QSO for the CQ Magazine USA-CA award program. E.g. MA,Franklin:MA,Hampshire
    USERDEFn,     //!<specifies the name and optional enumeration or range of the nth user-defined field, where n is a positive integer The name of a user-defined field may not be an ADIF Field name contain a comma a colon an open-angle-bracket or close-angle-bracket character an open-curly-bracket or close-curly-bracket character begin or end with a space character
    VE_PROV,     //!<import-only: use STATE instead
    VUCC_GRIDS,     //!<two or four adjacent Maidenhead grid locators, each four or six characters long, representing the contacted station's grid squares credited to the QSO for the ARRL VUCC award program. E.g. EM98,FM08,EM97,FM07
    WEB,     //!<the contacted station's URL
    WWFF_REF,     //!<the contacted station's WWFF (World Wildlife Flora & Fauna) reference
    MAX_FIELD               //!< Used to get maximum value
  };
	 //! Maps the enumerated value to string: used when exporting data
  static const std::map< field_t, std::string> FIELD_2_STRING = 
  {
    { ADDRESS, "ADDRESS" },
    { ADDRESS_INTL, "ADDRESS_INTL" },
    { ADIF_VER, "ADIF_VER" },
    { AGE, "AGE" },
    { ALTITUDE, "ALTITUDE" },
    { ANT_AZ, "ANT_AZ" },
    { ANT_EL, "ANT_EL" },
    { ANT_PATH, "ANT_PATH" },
    { APP_LOTW_LASTQSL, "APP_LOTW_LASTQSL" },
    { APP_LOTW_NUMREC, "APP_LOTW_NUMREC" },
    { APP_QRZLOG_LOGID, "APP_QRZLOG_LOGID" },
    { APP_QRZLOG_QSLDATE, "APP_QRZLOG_QSLDATE" },
    { APP_QRZLOG_STATUS, "APP_QRZLOG_STATUS" },
    { APP_ZZA_CQ, "APP_ZZA_CQ" },
    { APP_ZZA_ECARD, "APP_ZZA_ECARD" },
    { APP_ZZA_EQSL_MSG, "APP_ZZA_EQSL_MSG" },
    { APP_ZZA_ERROR, "APP_ZZA_ERROR" },
    { APP_ZZA_MY_CONT, "APP_ZZA_MY_CONT" },
    { APP_ZZA_MY_WAB, "APP_ZZA_MY_WAB" },
    { APP_ZZA_NUMRECORDS, "APP_ZZA_NUMRECORDS" },
    { APP_ZZA_OP, "APP_ZZA_OP" },
    { APP_ZZA_OPERATOR, "APP_ZZA_OPERATOR" },
    { APP_ZZA_OP_DESCR, "APP_ZZA_OP_DESCR" },
    { APP_ZZA_PFX, "APP_ZZA_PFX" },
    { APP_ZZA_QSL_ROUTE, "APP_ZZA_QSL_ROUTE" },
    { APP_ZZA_QTH, "APP_ZZA_QTH" },
    { APP_ZZA_QTH_DESCR, "APP_ZZA_QTH_DESCR" },
    { ARRL_SECT, "ARRL_SECT" },
    { AWARD_GRANTED, "AWARD_GRANTED" },
    { AWARD_SUBMITTED, "AWARD_SUBMITTED" },
    { A_INDEX, "A_INDEX" },
    { BAND, "BAND" },
    { BAND_RX, "BAND_RX" },
    { CALL, "CALL" },
    { CHECK, "CHECK" },
    { CLASS, "CLASS" },
    { CLUBLOG_QSO_UPLOAD_DATE, "CLUBLOG_QSO_UPLOAD_DATE" },
    { CLUBLOG_QSO_UPLOAD_STATUS, "CLUBLOG_QSO_UPLOAD_STATUS" },
    { CNTY, "CNTY" },
    { CNTY_ALT, "CNTY_ALT" },
    { COMMENT, "COMMENT" },
    { COMMENT_INTL, "COMMENT_INTL" },
    { CONT, "CONT" },
    { CONTACTED_OP, "CONTACTED_OP" },
    { CONTEST_ID, "CONTEST_ID" },
    { COUNTRY, "COUNTRY" },
    { COUNTRY_INTL, "COUNTRY_INTL" },
    { CQZ, "CQZ" },
    { CREATED_TIMESTAMP, "CREATED_TIMESTAMP" },
    { CREDIT_GRANTED, "CREDIT_GRANTED" },
    { CREDIT_SUBMITTED, "CREDIT_SUBMITTED" },
    { DARC_DOK, "DARC_DOK" },
    { DCL_QSLRDATE, "DCL_QSLRDATE" },
    { DCL_QSLSDATE, "DCL_QSLSDATE" },
    { DCL_QSL_RCVD, "DCL_QSL_RCVD" },
    { DCL_QSL_SENT, "DCL_QSL_SENT" },
    { DISTANCE, "DISTANCE" },
    { DXCC, "DXCC" },
    { EMAIL, "EMAIL" },
    { EQSL_AG, "EQSL_AG" },
    { EQSL_QSLRDATE, "EQSL_QSLRDATE" },
    { EQSL_QSLSDATE, "EQSL_QSLSDATE" },
    { EQSL_QSL_RCVD, "EQSL_QSL_RCVD" },
    { EQSL_QSL_SENT, "EQSL_QSL_SENT" },
    { EQ_CALL, "EQ_CALL" },
    { FISTS, "FISTS" },
    { FISTS_CC, "FISTS_CC" },
    { FORCE_INIT, "FORCE_INIT" },
    { FREQ, "FREQ" },
    { FREQ_RX, "FREQ_RX" },
    { GRIDSQUARE, "GRIDSQUARE" },
    { GRIDSQUARE_EXT, "GRIDSQUARE_EXT" },
    { GUEST_OP, "GUEST_OP" },
    { HAMLOGEU_QSO_UPLOAD_DATE, "HAMLOGEU_QSO_UPLOAD_DATE" },
    { HAMLOGEU_QSO_UPLOAD_STATUS, "HAMLOGEU_QSO_UPLOAD_STATUS" },
    { HAMQTH_QSO_UPLOAD_DATE, "HAMQTH_QSO_UPLOAD_DATE" },
    { HAMQTH_QSO_UPLOAD_STATUS, "HAMQTH_QSO_UPLOAD_STATUS" },
    { HRDLOG_QSO_UPLOAD_DATE, "HRDLOG_QSO_UPLOAD_DATE" },
    { HRDLOG_QSO_UPLOAD_STATUS, "HRDLOG_QSO_UPLOAD_STATUS" },
    { IOTA, "IOTA" },
    { IOTA_ISLAND_ID, "IOTA_ISLAND_ID" },
    { ITUZ, "ITUZ" },
    { K_INDEX, "K_INDEX" },
    { LAT, "LAT" },
    { LON, "LON" },
    { LOTW_QSLRDATE, "LOTW_QSLRDATE" },
    { LOTW_QSLSDATE, "LOTW_QSLSDATE" },
    { LOTW_QSL_RCVD, "LOTW_QSL_RCVD" },
    { LOTW_QSL_SENT, "LOTW_QSL_SENT" },
    { MAX_BURSTS, "MAX_BURSTS" },
    { MODE, "MODE" },
    { MORSE_KEY_INFO, "MORSE_KEY_INFO" },
    { MORSE_KEY_TYPE, "MORSE_KEY_TYPE" },
    { MS_SHOWER, "MS_SHOWER" },
    { MY_ALTITUDE, "MY_ALTITUDE" },
    { MY_ANTENNA, "MY_ANTENNA" },
    { MY_ANTENNA_INTL, "MY_ANTENNA_INTL" },
    { MY_ARRL_SECT, "MY_ARRL_SECT" },
    { MY_CITY, "MY_CITY" },
    { MY_CITY_INTL, "MY_CITY_INTL" },
    { MY_CNTY, "MY_CNTY" },
    { MY_CNTY_ALT, "MY_CNTY_ALT" },
    { MY_COUNTRY, "MY_COUNTRY" },
    { MY_COUNTRY_INTL, "MY_COUNTRY_INTL" },
    { MY_CQ_ZONE, "MY_CQ_ZONE" },
    { MY_DARC_DOK, "MY_DARC_DOK" },
    { MY_DXCC, "MY_DXCC" },
    { MY_FISTS, "MY_FISTS" },
    { MY_GRIDSQUARE, "MY_GRIDSQUARE" },
    { MY_GRIDSQUARE_EXT, "MY_GRIDSQUARE_EXT" },
    { MY_IOTA, "MY_IOTA" },
    { MY_IOTA_ISLAND_ID, "MY_IOTA_ISLAND_ID" },
    { MY_ITU_ZONE, "MY_ITU_ZONE" },
    { MY_LAT, "MY_LAT" },
    { MY_LON, "MY_LON" },
    { MY_MORSE_KEY_INFO, "MY_MORSE_KEY_INFO" },
    { MY_MORSE_KEY_TYPE, "MY_MORSE_KEY_TYPE" },
    { MY_NAME, "MY_NAME" },
    { MY_NAME_INTL, "MY_NAME_INTL" },
    { MY_POSTAL_CODE, "MY_POSTAL_CODE" },
    { MY_POSTAL_CODE_INTL, "MY_POSTAL_CODE_INTL" },
    { MY_POTA_REF, "MY_POTA_REF" },
    { MY_RIG, "MY_RIG" },
    { MY_RIG_INTL, "MY_RIG_INTL" },
    { MY_SIG, "MY_SIG" },
    { MY_SIG_INFO, "MY_SIG_INFO" },
    { MY_SIG_INFO_INTL, "MY_SIG_INFO_INTL" },
    { MY_SIG_INTL, "MY_SIG_INTL" },
    { MY_SOTA_REF, "MY_SOTA_REF" },
    { MY_STATE, "MY_STATE" },
    { MY_STREET, "MY_STREET" },
    { MY_STREET_INTL, "MY_STREET_INTL" },
    { MY_USACA_COUNTIES, "MY_USACA_COUNTIES" },
    { MY_VUCC_GRIDS, "MY_VUCC_GRIDS" },
    { MY_WWFF_REF, "MY_WWFF_REF" },
    { NAME, "NAME" },
    { NAME_INTL, "NAME_INTL" },
    { NOTES, "NOTES" },
    { NOTES_INTL, "NOTES_INTL" },
    { NR_BURSTS, "NR_BURSTS" },
    { NR_PINGS, "NR_PINGS" },
    { OPERATOR, "OPERATOR" },
    { OWNER_CALLSIGN, "OWNER_CALLSIGN" },
    { PFX, "PFX" },
    { POTA_REF, "POTA_REF" },
    { PRECEDENCE, "PRECEDENCE" },
    { PROGRAMID, "PROGRAMID" },
    { PROGRAMVERSION, "PROGRAMVERSION" },
    { PROP_MODE, "PROP_MODE" },
    { PUBLIC_KEY, "PUBLIC_KEY" },
    { QRZCOM_QSO_DOWNLOAD_DATE, "QRZCOM_QSO_DOWNLOAD_DATE" },
    { QRZCOM_QSO_DOWNLOAD_STATUS, "QRZCOM_QSO_DOWNLOAD_STATUS" },
    { QRZCOM_QSO_UPLOAD_DATE, "QRZCOM_QSO_UPLOAD_DATE" },
    { QRZCOM_QSO_UPLOAD_STATUS, "QRZCOM_QSO_UPLOAD_STATUS" },
    { QSLMSG, "QSLMSG" },
    { QSLMSG_INTL, "QSLMSG_INTL" },
    { QSLMSG_RCVD, "QSLMSG_RCVD" },
    { QSLRDATE, "QSLRDATE" },
    { QSLSDATE, "QSLSDATE" },
    { QSL_RCVD, "QSL_RCVD" },
    { QSL_RCVD_VIA, "QSL_RCVD_VIA" },
    { QSL_SENT, "QSL_SENT" },
    { QSL_SENT_VIA, "QSL_SENT_VIA" },
    { QSL_VIA, "QSL_VIA" },
    { QSO_COMPLETE, "QSO_COMPLETE" },
    { QSO_DATE, "QSO_DATE" },
    { QSO_DATE_OFF, "QSO_DATE_OFF" },
    { QSO_RANDOM, "QSO_RANDOM" },
    { QTH, "QTH" },
    { QTH_INTL, "QTH_INTL" },
    { REGION, "REGION" },
    { RIG, "RIG" },
    { RIG_INTL, "RIG_INTL" },
    { RST_RCVD, "RST_RCVD" },
    { RST_SENT, "RST_SENT" },
    { RX_PWR, "RX_PWR" },
    { SAT_MODE, "SAT_MODE" },
    { SAT_NAME, "SAT_NAME" },
    { SFI, "SFI" },
    { SIG, "SIG" },
    { SIG_INFO, "SIG_INFO" },
    { SIG_INFO_INTL, "SIG_INFO_INTL" },
    { SIG_INTL, "SIG_INTL" },
    { SILENT_KEY, "SILENT_KEY" },
    { SKCC, "SKCC" },
    { SOTA_REF, "SOTA_REF" },
    { SRX, "SRX" },
    { SRX_STRING, "SRX_STRING" },
    { STATE, "STATE" },
    { STATION_CALLSIGN, "STATION_CALLSIGN" },
    { STX, "STX" },
    { STX_STRING, "STX_STRING" },
    { SUBMODE, "SUBMODE" },
    { SWL, "SWL" },
    { TEN_TEN, "TEN_TEN" },
    { TIME_OFF, "TIME_OFF" },
    { TIME_ON, "TIME_ON" },
    { TX_PWR, "TX_PWR" },
    { UKSMG, "UKSMG" },
    { USACA_COUNTIES, "USACA_COUNTIES" },
    { USERDEFn, "USERDEFn" },
    { VE_PROV, "VE_PROV" },
    { VUCC_GRIDS, "VUCC_GRIDS" },
    { WEB, "WEB" },
    { WWFF_REF, "WWFF_REF" },
    { MAX_FIELD, "" }
  };

  //! Maps the string to enumerated value: used when importing data
  static const std::map< std::string, field_t> STRING_2_FIELD = 
  {
    { " ADDRESS", ADDRESS }, 
    { " ADDRESS_INTL", ADDRESS_INTL }, 
    { " ADIF_VER", ADIF_VER }, 
    { " AGE", AGE }, 
    { " ALTITUDE", ALTITUDE }, 
    { " ANT_AZ", ANT_AZ }, 
    { " ANT_EL", ANT_EL }, 
    { " ANT_PATH", ANT_PATH }, 
    { " APP_LOTW_LASTQSL", APP_LOTW_LASTQSL }, 
    { " APP_LOTW_NUMREC", APP_LOTW_NUMREC }, 
    { " APP_QRZLOG_LOGID", APP_QRZLOG_LOGID }, 
    { " APP_QRZLOG_QSLDATE", APP_QRZLOG_QSLDATE }, 
    { " APP_QRZLOG_STATUS", APP_QRZLOG_STATUS }, 
    { " APP_ZZA_CQ", APP_ZZA_CQ }, 
    { " APP_ZZA_ECARD", APP_ZZA_ECARD }, 
    { " APP_ZZA_EQSL_MSG", APP_ZZA_EQSL_MSG }, 
    { " APP_ZZA_ERROR", APP_ZZA_ERROR }, 
    { " APP_ZZA_MY_CONT", APP_ZZA_MY_CONT }, 
    { " APP_ZZA_MY_WAB", APP_ZZA_MY_WAB }, 
    { " APP_ZZA_NUMRECORDS", APP_ZZA_NUMRECORDS }, 
    { " APP_ZZA_OP", APP_ZZA_OP }, 
    { " APP_ZZA_OPERATOR", APP_ZZA_OPERATOR }, 
    { " APP_ZZA_OP_DESCR", APP_ZZA_OP_DESCR }, 
    { " APP_ZZA_PFX", APP_ZZA_PFX }, 
    { " APP_ZZA_QSL_ROUTE", APP_ZZA_QSL_ROUTE }, 
    { " APP_ZZA_QTH", APP_ZZA_QTH }, 
    { " APP_ZZA_QTH_DESCR", APP_ZZA_QTH_DESCR }, 
    { " ARRL_SECT", ARRL_SECT }, 
    { " AWARD_GRANTED", AWARD_GRANTED }, 
    { " AWARD_SUBMITTED", AWARD_SUBMITTED }, 
    { " A_INDEX", A_INDEX }, 
    { " BAND", BAND }, 
    { " BAND_RX", BAND_RX }, 
    { " CALL", CALL }, 
    { " CHECK", CHECK }, 
    { " CLASS", CLASS }, 
    { " CLUBLOG_QSO_UPLOAD_DATE", CLUBLOG_QSO_UPLOAD_DATE }, 
    { " CLUBLOG_QSO_UPLOAD_STATUS", CLUBLOG_QSO_UPLOAD_STATUS }, 
    { " CNTY", CNTY }, 
    { " CNTY_ALT", CNTY_ALT }, 
    { " COMMENT", COMMENT }, 
    { " COMMENT_INTL", COMMENT_INTL }, 
    { " CONT", CONT }, 
    { " CONTACTED_OP", CONTACTED_OP }, 
    { " CONTEST_ID", CONTEST_ID }, 
    { " COUNTRY", COUNTRY }, 
    { " COUNTRY_INTL", COUNTRY_INTL }, 
    { " CQZ", CQZ }, 
    { " CREATED_TIMESTAMP", CREATED_TIMESTAMP }, 
    { " CREDIT_GRANTED", CREDIT_GRANTED }, 
    { " CREDIT_SUBMITTED", CREDIT_SUBMITTED }, 
    { " DARC_DOK", DARC_DOK }, 
    { " DCL_QSLRDATE", DCL_QSLRDATE }, 
    { " DCL_QSLSDATE", DCL_QSLSDATE }, 
    { " DCL_QSL_RCVD", DCL_QSL_RCVD }, 
    { " DCL_QSL_SENT", DCL_QSL_SENT }, 
    { " DISTANCE", DISTANCE }, 
    { " DXCC", DXCC }, 
    { " EMAIL", EMAIL }, 
    { " EQSL_AG", EQSL_AG }, 
    { " EQSL_QSLRDATE", EQSL_QSLRDATE }, 
    { " EQSL_QSLSDATE", EQSL_QSLSDATE }, 
    { " EQSL_QSL_RCVD", EQSL_QSL_RCVD }, 
    { " EQSL_QSL_SENT", EQSL_QSL_SENT }, 
    { " EQ_CALL", EQ_CALL }, 
    { " FISTS", FISTS }, 
    { " FISTS_CC", FISTS_CC }, 
    { " FORCE_INIT", FORCE_INIT }, 
    { " FREQ", FREQ }, 
    { " FREQ_RX", FREQ_RX }, 
    { " GRIDSQUARE", GRIDSQUARE }, 
    { " GRIDSQUARE_EXT", GRIDSQUARE_EXT }, 
    { " GUEST_OP", GUEST_OP }, 
    { " HAMLOGEU_QSO_UPLOAD_DATE", HAMLOGEU_QSO_UPLOAD_DATE }, 
    { " HAMLOGEU_QSO_UPLOAD_STATUS", HAMLOGEU_QSO_UPLOAD_STATUS }, 
    { " HAMQTH_QSO_UPLOAD_DATE", HAMQTH_QSO_UPLOAD_DATE }, 
    { " HAMQTH_QSO_UPLOAD_STATUS", HAMQTH_QSO_UPLOAD_STATUS }, 
    { " HRDLOG_QSO_UPLOAD_DATE", HRDLOG_QSO_UPLOAD_DATE }, 
    { " HRDLOG_QSO_UPLOAD_STATUS", HRDLOG_QSO_UPLOAD_STATUS }, 
    { " IOTA", IOTA }, 
    { " IOTA_ISLAND_ID", IOTA_ISLAND_ID }, 
    { " ITUZ", ITUZ }, 
    { " K_INDEX", K_INDEX }, 
    { " LAT", LAT }, 
    { " LON", LON }, 
    { " LOTW_QSLRDATE", LOTW_QSLRDATE }, 
    { " LOTW_QSLSDATE", LOTW_QSLSDATE }, 
    { " LOTW_QSL_RCVD", LOTW_QSL_RCVD }, 
    { " LOTW_QSL_SENT", LOTW_QSL_SENT }, 
    { " MAX_BURSTS", MAX_BURSTS }, 
    { " MODE", MODE }, 
    { " MORSE_KEY_INFO", MORSE_KEY_INFO }, 
    { " MORSE_KEY_TYPE", MORSE_KEY_TYPE }, 
    { " MS_SHOWER", MS_SHOWER }, 
    { " MY_ALTITUDE", MY_ALTITUDE }, 
    { " MY_ANTENNA", MY_ANTENNA }, 
    { " MY_ANTENNA_INTL", MY_ANTENNA_INTL }, 
    { " MY_ARRL_SECT", MY_ARRL_SECT }, 
    { " MY_CITY", MY_CITY }, 
    { " MY_CITY_INTL", MY_CITY_INTL }, 
    { " MY_CNTY", MY_CNTY }, 
    { " MY_CNTY_ALT", MY_CNTY_ALT }, 
    { " MY_COUNTRY", MY_COUNTRY }, 
    { " MY_COUNTRY_INTL", MY_COUNTRY_INTL }, 
    { " MY_CQ_ZONE", MY_CQ_ZONE }, 
    { " MY_DARC_DOK", MY_DARC_DOK }, 
    { " MY_DXCC", MY_DXCC }, 
    { " MY_FISTS", MY_FISTS }, 
    { " MY_GRIDSQUARE", MY_GRIDSQUARE }, 
    { " MY_GRIDSQUARE_EXT", MY_GRIDSQUARE_EXT }, 
    { " MY_IOTA", MY_IOTA }, 
    { " MY_IOTA_ISLAND_ID", MY_IOTA_ISLAND_ID }, 
    { " MY_ITU_ZONE", MY_ITU_ZONE }, 
    { " MY_LAT", MY_LAT }, 
    { " MY_LON", MY_LON }, 
    { " MY_MORSE_KEY_INFO", MY_MORSE_KEY_INFO }, 
    { " MY_MORSE_KEY_TYPE", MY_MORSE_KEY_TYPE }, 
    { " MY_NAME", MY_NAME }, 
    { " MY_NAME_INTL", MY_NAME_INTL }, 
    { " MY_POSTAL_CODE", MY_POSTAL_CODE }, 
    { " MY_POSTAL_CODE_INTL", MY_POSTAL_CODE_INTL }, 
    { " MY_POTA_REF", MY_POTA_REF }, 
    { " MY_RIG", MY_RIG }, 
    { " MY_RIG_INTL", MY_RIG_INTL }, 
    { " MY_SIG", MY_SIG }, 
    { " MY_SIG_INFO", MY_SIG_INFO }, 
    { " MY_SIG_INFO_INTL", MY_SIG_INFO_INTL }, 
    { " MY_SIG_INTL", MY_SIG_INTL }, 
    { " MY_SOTA_REF", MY_SOTA_REF }, 
    { " MY_STATE", MY_STATE }, 
    { " MY_STREET", MY_STREET }, 
    { " MY_STREET_INTL", MY_STREET_INTL }, 
    { " MY_USACA_COUNTIES", MY_USACA_COUNTIES }, 
    { " MY_VUCC_GRIDS", MY_VUCC_GRIDS }, 
    { " MY_WWFF_REF", MY_WWFF_REF }, 
    { " NAME", NAME }, 
    { " NAME_INTL", NAME_INTL }, 
    { " NOTES", NOTES }, 
    { " NOTES_INTL", NOTES_INTL }, 
    { " NR_BURSTS", NR_BURSTS }, 
    { " NR_PINGS", NR_PINGS }, 
    { " OPERATOR", OPERATOR }, 
    { " OWNER_CALLSIGN", OWNER_CALLSIGN }, 
    { " PFX", PFX }, 
    { " POTA_REF", POTA_REF }, 
    { " PRECEDENCE", PRECEDENCE }, 
    { " PROGRAMID", PROGRAMID }, 
    { " PROGRAMVERSION", PROGRAMVERSION }, 
    { " PROP_MODE", PROP_MODE }, 
    { " PUBLIC_KEY", PUBLIC_KEY }, 
    { " QRZCOM_QSO_DOWNLOAD_DATE", QRZCOM_QSO_DOWNLOAD_DATE }, 
    { " QRZCOM_QSO_DOWNLOAD_STATUS", QRZCOM_QSO_DOWNLOAD_STATUS }, 
    { " QRZCOM_QSO_UPLOAD_DATE", QRZCOM_QSO_UPLOAD_DATE }, 
    { " QRZCOM_QSO_UPLOAD_STATUS", QRZCOM_QSO_UPLOAD_STATUS }, 
    { " QSLMSG", QSLMSG }, 
    { " QSLMSG_INTL", QSLMSG_INTL }, 
    { " QSLMSG_RCVD", QSLMSG_RCVD }, 
    { " QSLRDATE", QSLRDATE }, 
    { " QSLSDATE", QSLSDATE }, 
    { " QSL_RCVD", QSL_RCVD }, 
    { " QSL_RCVD_VIA", QSL_RCVD_VIA }, 
    { " QSL_SENT", QSL_SENT }, 
    { " QSL_SENT_VIA", QSL_SENT_VIA }, 
    { " QSL_VIA", QSL_VIA }, 
    { " QSO_COMPLETE", QSO_COMPLETE }, 
    { " QSO_DATE", QSO_DATE }, 
    { " QSO_DATE_OFF", QSO_DATE_OFF }, 
    { " QSO_RANDOM", QSO_RANDOM }, 
    { " QTH", QTH }, 
    { " QTH_INTL", QTH_INTL }, 
    { " REGION", REGION }, 
    { " RIG", RIG }, 
    { " RIG_INTL", RIG_INTL }, 
    { " RST_RCVD", RST_RCVD }, 
    { " RST_SENT", RST_SENT }, 
    { " RX_PWR", RX_PWR }, 
    { " SAT_MODE", SAT_MODE }, 
    { " SAT_NAME", SAT_NAME }, 
    { " SFI", SFI }, 
    { " SIG", SIG }, 
    { " SIG_INFO", SIG_INFO }, 
    { " SIG_INFO_INTL", SIG_INFO_INTL }, 
    { " SIG_INTL", SIG_INTL }, 
    { " SILENT_KEY", SILENT_KEY }, 
    { " SKCC", SKCC }, 
    { " SOTA_REF", SOTA_REF }, 
    { " SRX", SRX }, 
    { " SRX_STRING", SRX_STRING }, 
    { " STATE", STATE }, 
    { " STATION_CALLSIGN", STATION_CALLSIGN }, 
    { " STX", STX }, 
    { " STX_STRING", STX_STRING }, 
    { " SUBMODE", SUBMODE }, 
    { " SWL", SWL }, 
    { " TEN_TEN", TEN_TEN }, 
    { " TIME_OFF", TIME_OFF }, 
    { " TIME_ON", TIME_ON }, 
    { " TX_PWR", TX_PWR }, 
    { " UKSMG", UKSMG }, 
    { " USACA_COUNTIES", USACA_COUNTIES }, 
    { " USERDEFn", USERDEFn }, 
    { " VE_PROV", VE_PROV }, 
    { " VUCC_GRIDS", VUCC_GRIDS }, 
    { " WEB", WEB }, 
    { " WWFF_REF", WWFF_REF }, 
  };

  //! Current version knwon to compiler
  static const std::string VERSION = "3.1.6";
};