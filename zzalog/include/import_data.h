#ifndef __IMPORT_DATA__
#define __IMPORT_DATA__

#include "book.h"

#include <string>
#include <vector>
#include <ctime>



class book;

	//! This class inherits book and provides the additional functionality required
	//! for importing additional data to the main log. 
	
	//! Pointers to imported Records are initially held in this book
	//! and pointers copied or records merged into the main log.
	//! Any problems encountered are refered to the 
	//! user for resolution.
	class import_data : public book
	{
	public:
		//! Purpose of imported data.
		enum update_mode_t : uchar {
			NONE,            //!< Book has no data.
			EXISTING,        //!< Use existing update mode.
			FILE_IMPORT,     //!< Data manually loaded from a file (assume new records are correct).
			FILE_UPDATE,     //!< Data to be merged from a file (i.e. check it exists).
			EQSL_UPDATE,     //!< Data downloaded from eQSL.cc.
			LOTW_UPDATE,     //!< Data downloaded from arrl.org/lotw.
			QRZCOM_UPDATE,   //!< Data downloaded from QRZ.com.
			DATAGRAM,        //!< Data received from WSJT-X datagram.
			CLIPBOARD,       //!< Data pasted from clipboard.
			SINGLE_ADIF,     //!< Import a single ADIF.
			OQRS,            //!< Book contains a std::list of OQRS QSL requests
		};

	public:
		// Contsructor.
		import_data();
		// Destructor.
		~import_data();

		// public methods
	public:
		//! Delete the mismatch record in the update.
		
		//! \param notify true updates a count of discarded records.
		void discard_update(bool notify);
		//! Accept the record from the update
		void accept_update();
		//! Combine the record from the log and the update
		void merge_update();
		//! Add the update record to the log
		void save_update();
		//! Start or continue analysing the update data
		void update_book();

		//! Stop importing.
		//! 
		//! \param immediate true causes the import to be abandoned immediately.
		//! false causes the update to be wound down gradually.
		void stop_update(bool immediate);
		//! Returns true if update the update is complete.
		bool update_complete();
		//! download data from QSL \p server and returns true if successful.
		bool download_data(update_mode_t server);
		//! Merge the data according to the update \p mode.
		void merge_data(update_mode_t mode = EXISTING);
		//! Load data for import
		//! 
		//! \param filename file to read the data for importing.
		//! \param mode controls how the import is handled.
		//! \return true if the load is successful.
		bool load_data(std::string filename, update_mode_t mode);
		//! Load data from a stream \p adif for \p mode purpose.
		void load_stream(std::stringstream& adif, update_mode_t mode);
		//! Load record \p qso for \p mode purpose.
		void load_record(record* qso, update_mode_t mode = SINGLE_ADIF);
		//! Returns last record loaded
		record* last_record_loaded();

	protected:
		//! Finish the update process: housekeeping.
		void finish_update(bool merged = true);
		//! Convert the update \p record.
		
		//! Where an update has come from a QSL server, 
		//! some ADIF fields are renamed to the viewpoiint of this
		//! log not the server.
		void convert_update(record* record);

	protected:
		//! a record update query is in progress
		bool update_in_progress_;
		//! the update is a new record
		bool update_is_new_;
		//! What the purpose of the update is.
		update_mode_t update_mode_;
		//! Number of records in import
		int number_to_import_;
		//! The number of records that have been modified as a result of inport
		int number_modified_;
		//! The number of records that matched in an import update
		int number_matched_;
		//! The number of records checked
		int number_checked_;
		//! The number of new records added
		int number_added_;
		//! The number of rejected
		int number_rejected_;
		//! The number of records with clublog data modified
		//! as a result of LotW QSL information update
		int number_clublog_;
		//! The number of SWL records added
		int number_swl_;
		//! Close is pending
		bool close_pending_;
		//! Last added record number in full log.
		qso_num_t last_added_number_;
		//! Last record loaded
		record* last_record_loaded_;

	};
#endif

