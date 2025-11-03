#ifndef __EQSL_HANDLER__
#define __EQSL_HANDLER__

#include "url_handler.h"
#include "fields.h"

#include <deque>
#include <queue>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <thread>
#include <atomic>
#include <mutex>



class record;
class book;
class Fl_Help_View;
class Fl_Window;
typedef size_t item_num_t;
typedef size_t qso_num_t;


	// eQSL throttling - 10s
	const double EQSL_THROTTLE = 10.0;
	const char EQSL_TIMEFORMAT[] = "%Y%m%d";

	//! Default fields to use when sending QSOs to eQSL.cc
	const field_list EQSL_FIELDS = {
		"QSO_DATE",
		"TIME_ON",
	    "TIME_OFF",
		"BAND",
		"MODE", 
		"CALL",
		"RST_SENT",
		"QSL_MSG"
	};

	//! This class manages the requirements for uploading data to and downloading data from eQSL.cc
	class eqsl_handler
	{
	public:
		//! Response for an eQSL request
		enum response_t {
			ER_OK,         //!< Request successful
			ER_SKIPPED,    //!< Request skipped
			ER_THROTTLED,  //!< Request throttled by eQSL.cc
			ER_FAILED,     //!< Request failed
			ER_HTML_ERR,   //!< HTML error
			ER_DUPLICATE   //!< Duplicate request
		};
		//! Data required to std::queue requests into upload std::thread.
		struct request_t {
			//! Index of QSO record in full logbook.
			qso_num_t record_num;
			//! Force upload.
			bool force;
			//! Default constructor.
			request_t()
				: record_num(0)
				, force(false)
			{
			}
			//! Constructor when posting request.
			request_t(item_num_t n, bool f)
				: record_num(n)
				, force(f)
			{}
		};
		//! Request std::queue control
		enum queue_control_t {
			EQ_PAUSE,      //!< pause
			EQ_START,      //!< start
			EQ_ABANDON     //!< abandon
		};
		//! Request std::queue
		typedef std::queue<request_t> queue_t;
		////! Information required when removing entry from std::queue
		//struct dequeue_param_t {
		//	//! Pointer to the request std::queue.
		//	queue_t* std::queue;
		//	//! Pointer to this eqsl_handler.
		//	eqsl_handler* handler;
		//	//! Default constructor.
		//	dequeue_param_t() {
		//		std::queue = nullptr;
		//		handler = nullptr;
		//	}
		//	//! Constructor to create an instance.
		//	dequeue_param_t(queue_t* q, eqsl_handler* h)
		//		: std::queue(q)
		//		, handler(h)
		//	{
		//	}
		//};
		//! Upload response.
		struct upload_response_t {
			//! Response status.
			response_t status;
			//! Text showing response message.
			std::string error_message;
			//! HTML text.
			std::string html;
			//! QSO record whose upload request the response is for.
			record* qso;
			//! Default constructor.
			upload_response_t() {
				status = ER_OK;
				error_message = "";
				html = "";
				qso = nullptr;
			}
		};
	public:
		//! Constructor.
		eqsl_handler();
		//! Destructor.
		~eqsl_handler();

		//! enqueue a request to fetch a qsl card.
		
		//! \param record_num index of QSO record.
		//! \param force make the request even if already have some data.
		void enqueue_request(qso_num_t record_num, bool force = false);
		//! Download the data from eqsl into the data stream \p adif.
		bool download_eqsl_log(std::stringstream* adif);
		//! Control the scheduling from the request std::queue.
		
		//! \param control One of EQ_START, EQ_PAUSE or EQ_ABANDON.
		void enable_fetch(queue_control_t control);
		//! Upload the QSO records from the extracted logbook.
		bool upload_eqsl_log(book* book);
		//! Is there active fetching? Returns true if there is, false if not.
		bool requests_queued();
		//! Get the local card file-name.
		
		//! \param record QSO record 
		//! \param use_default use the default station callsign.
		//! \return location to store the image in local filestore.
		std::string card_filename_l(record* record, bool use_default = false);
		//! Does the file exist and is it a valid PNG file?
		
		//! \param filename local QSL card image.
		//! \return true if it exists and is valid PNG.
		bool card_file_valid(std::string& filename);
		//! Enqueue single record - passes to upload std::thread.
		bool upload_single_qso(qso_num_t record_num);

	protected:
		//! Timer callback: 1 second ticker
		static void cb_ticker(void* v);

		//! Remove a request from the download std::queue
		void dequeue_request();

		//! Perform the eQSL card image request. 
		response_t request_eqsl(request_t request);
		//! Get the remote filename of the card
		
		//! A request is made to eQSL.cc and the response is scanned for the card image filename.
		//! \param record QSO record.
		//! \param filename returns the filename.
		//! \param filetype returns the filetype language code/
		//! \return response structure.
		response_t card_filename_r(record* record, std::string& filename, std::string& filetype);
		//! Download the card image to local file-store.
		
		//! \param remote_filename Filename at eQSL.cc.
		//! \param local_filename Filename on local network.
		//! \return response structure.
		response_t download(std::string remote_filename, std::string local_filename);
		//! Get user details from internal QSL server database.
		
		//! \param username returns username.
		//! \param password returns password.
		//! \param last_access returns date of last eQSL upload.
		//! \param qsl_message returns default message for QSO uploads.
		//! \param swl_message returns default message for SWL QSL uploads.
		//! \param confirmed returns whether only confirmed requests are required.
		//! \return true if username and password are available, false if not.
		bool user_details(std::string* username, std::string* password, std::string* last_access, 
			std::string* qsl_message, std::string* swl_message, bool* confirmed);
		//! Get the filename of the data to be downloaded from eQSL.cc.
		
		//! \param filename returns filename.
		//! \return response structure.
		response_t adif_filename(std::string& filename);
		//! Download the data.
		
		//! \param filename remote filename to fetch.
		//! \param adif datastream to receive downloaded data.
		//! \return response structure.
		response_t download_adif(std::string& filename, std::stringstream* adif);
		//! Generate std::list of adif fields for sending to eQSL.cc.
		void set_adif_fields();
		//! Generate data for POST FORM fields.
		void form_fields(std::vector<url_handler::field_pair>&);
		//! Parse the warning message for user readable format.
		std::map<std::string, std::string> parse_warning(std::string text);
		//! Callback from request std::thread.
		static void cb_upload_done(void* v);
		//! Upload QSO record \p qso on eQSL request std::thread.
		bool th_upload_qso(record* qso);
		//! Start eQSL request std::thread.
		static void thread_run(eqsl_handler* that);

		//! Process \p response from eQSL.cc.
		bool upload_done(upload_response_t* response);
		//! Open the Help Viewer to display the response from eQSL.
		void display_response(std::string response);
		//! Progress the image downloads
		void progress_download();

	protected:
		//! The card inage request std::queue between main and request threads.
		queue_t request_queue_;
		//! Request std::queue is allowed to empty
		bool empty_queue_enable_;
		//! Username
		std::string username_;
		//! Password
		std::string password_;
		//! Thread to run eQSL.cc requests in.
		std::thread* th_upload_;
		//! Enable for threads - normally true and std::set false when closing ZZALOG.
		std::atomic<bool> run_threads_;
		//! Queue for uplaoding QSO records.
		std::queue<record*> upload_queue_;
		//! Semaphore to lock upload std::queue while enqueuing and dequeueing uploads. 
		std::mutex upload_lock_;
		//! Upload response.
		std::atomic<upload_response_t*> upload_response_;
		//! Set of field names used in QSO uploads.
		field_list adif_fields_;

		//! Image download count.
		
		//! Incremented when request enqueued.
		//! Set to zero when dequeuing last request.
		int download_count_;

		//! Image download throttle count.
		
		//! Set to zero when a request dequeued.
		//! Incremented on 1 second tick
		int tick_count_;

		// Window and Help viewer for displaying response
		Fl_Window* help_window_;        //!< Window launched when displaying response from e!SL.cc
		Fl_Help_View* help_viewer_;     //!< Rudimentary HTML display widget for such response.

	};

#endif
