#include "cty1_reader.h"

#include "cty_data.h"
#include "main.h"
#include "status.h"

#include "pugixml.hpp"

#include <list>

// Constructor
cty1_reader::cty1_reader() {
	//ignore_processing_ = false;
	//current_entity_ = nullptr;
	//current_prefix_ = nullptr;
	//current_exception_ = nullptr;
	//current_invalid_ = nullptr;
	//current_zone_exc_ = nullptr;
	//current_entity_ = nullptr;
	//current_prefix_ = nullptr;
	data_ = nullptr;
	//file_ = nullptr;
}

// Destructor
cty1_reader::~cty1_reader() {
}

// Load data from specified file into and add each record to the std::map
bool cty1_reader::load_data(cty_data* data, std::istream& in, std::string& version) {
	fl_cursor(FL_CURSOR_WAIT);

	status_->progress(6, OT_PREFIX, "Loading Clublog.org", "steps");

	pugi::xml_document doc;
	doc.load(in);

	pugi::xml_node top = doc.document_element();
	status_->progress(1, OT_PREFIX);

	// Call the XML parser - calls back to the overides herein

	// Copy entities
	pugi::xml_node n_ents = top.child("entities");
	for (auto n_ent : n_ents.children()) {
		cty_entity* entity = new cty_entity;
		entity->dxcc_id_ = n_ent.child("adif").text().as_int();
		entity->name_ = n_ent.child("name").text().as_string();
		entity->nickname_ = n_ent.child("prefix").text().as_string();
		entity->deleted_ = n_ent.child("deleted").text().as_bool();
		entity->cq_zone_ = n_ent.child("cqz").text().as_int();
		entity->continent_ = n_ent.child("cont").text().as_string();
		entity->coordinates_.longitude = n_ent.child("long").text().as_double();
		entity->coordinates_.latitude = n_ent.child("lat").text().as_double();
		entity->time_validity_.start =
			xmldt2date(n_ent.child("start").text().as_string());
		entity->time_validity_.finish =
			xmldt2date(n_ent.child("end").text().as_string());
		data->add_entity(entity);
	}
	status_->progress(2, OT_PREFIX);

	// Copy exceptions
	pugi::xml_node n_excs = top.child("exceptions");
	for (auto n_exc : n_excs.children()) {
		std::string call = n_exc.child("call").text().as_string();
		cty_exception* exc = new cty_exception;
		exc->exc_type_ = cty_exception::EXC_OVERRIDE;
		exc->name_ = n_exc.child("entity").text().as_string();
		exc->dxcc_id_ = n_exc.child("adif").text().as_int();
		exc->cq_zone_ = n_exc.child("cqz").text().as_int();
		exc->continent_ = n_exc.child("cont").text().as_string();
		exc->coordinates_.longitude = n_exc.child("long").text().as_double();
		exc->coordinates_.latitude = n_exc.child("lat").text().as_double();
		exc->time_validity_.start =
			xmldt2date(n_exc.child("start").text().as_string());
		exc->time_validity_.finish =
			xmldt2date(n_exc.child("end").text().as_string());
		data->add_exception(call, exc);
	}
	status_->progress(3, OT_PREFIX);

	// Copy prefixes
	pugi::xml_node n_pfxs = top.child("prefixes");
	for (auto n_pfx : n_pfxs.children()) {
		std::string call = n_pfx.child("call").text().as_string();
		cty_prefix* pfx = new cty_prefix;
		pfx->name_ = n_pfx.child("entity").text().as_string();
		pfx->dxcc_id_ = n_pfx.child("adif").text().as_int();
		pfx->cq_zone_ = n_pfx.child("cqz").text().as_int();
		pfx->continent_ = n_pfx.child("cont").text().as_string();
		pfx->coordinates_.longitude = n_pfx.child("long").text().as_double();
		pfx->coordinates_.latitude = n_pfx.child("lat").text().as_double();
		pfx->time_validity_.start =
			xmldt2date(n_pfx.child("start").text().as_string());
		pfx->time_validity_.finish =
			xmldt2date(n_pfx.child("end").text().as_string());
		data->add_prefix(call, pfx);
	}
	status_->progress(4, OT_PREFIX);

	// Copy invalid operations
	pugi::xml_node n_invs = top.child("invalid_operations");
	for (auto n_inv : n_invs.children()) {
		std::string call = n_inv.child("call").text().as_string();
		cty_exception* inv = new cty_exception;
		inv->exc_type_ = cty_exception::EXC_INVALID;
		inv->time_validity_.start =
			xmldt2date(n_inv.child("start").text().as_string());
		inv->time_validity_.finish =
			xmldt2date(n_inv.child("end").text().as_string());
		data->add_exception(call, inv);
	}
	status_->progress(5, OT_PREFIX);

	// Copy zone exceptions
	pugi::xml_node n_zexs = top.child("zone_exceptions");
	for (auto n_zex : n_zexs.children()) {
		std::string call = n_zex.child("call").text().as_string();
		cty_exception* zex = new cty_exception;
		zex->exc_type_ = cty_exception::EXC_INVALID;
		zex->cq_zone_ = n_zex.child("zone").text().as_int();
		zex->time_validity_.start =
			xmldt2date(n_zex.child("start").text().as_string());
		zex->time_validity_.finish =
			xmldt2date(n_zex.child("end").text().as_string());
		data->add_exception(call, zex);
	}
	status_->progress(6, OT_PREFIX);
	return true;
}

// Get date in format %Y%m%d from XML date time value.
std::string cty1_reader::xmldt2date(std::string xml_date) {
	if (xml_date.length() == 0) return "*";
	std::string result = xml_date.substr(0, 4) + xml_date.substr(5, 2) + xml_date.substr(8, 2) +
		xml_date.substr(11, 2) + xml_date.substr(14, 2);
	return result;
}