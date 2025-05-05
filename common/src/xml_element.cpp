#include "xml_element.h"

// Constructor - creates an empty element
xml_element::xml_element() :
	xml_element(nullptr, "", "", nullptr)
{
}

// Creates a complete element (apart from children)
xml_element::xml_element(xml_element* parent, const string& name, const string& content, map<string, string>* attributes, xml_element::element_t type) {
	type_ = type;
	parent_ = parent;
	name_ = name;
	content_ = content;
	attributes_ = attributes;
	children_.clear();
}

// Destructor
xml_element::~xml_element()
{
	// Remove attributes (if any)
	if (attributes_ != nullptr) {
		attributes_->clear();
		delete attributes_;
	}
	// Deletes all child elements
	for (size_t i = 0; i < children_.size(); i++) {
		delete children_.at(i);
	}
	children_.clear();
}

// Add a child element
void xml_element::element(xml_element* element) {
	children_.push_back(element);
}

// Returns the count of child elements
int xml_element::count() {
	return children_.size();
}

// Returns a pointer to child element number i
xml_element* xml_element::child(int i) {
	return children_.at(i);
}

// Returns the pointer to the parent element
xml_element* xml_element::parent() {
	return parent_;
}

// Sets an attribute name=value
bool xml_element::attribute(string& name, string& value) {
	// Create attributes if they don't exist
	if (attributes_ == nullptr) {
		attributes_ = new map<string, string>;
	}
	if (attributes_->find(name) != attributes_->end()) {
		// Not allowed two attributes of the same name
		return false;
	}
	else {
		// Set attribute
		(*attributes_)[name] = value;
		return true;
	}
}

// Set the content
bool xml_element::content(string& content) {
	content_ += content;
	return true;
}

// Return the name
string xml_element::name() {
	return name_;
}

// Return a pointer to the attributes
map<string, string>* xml_element::attributes() {
	return attributes_;
}

// Return the content
string xml_element::content() {
	return content_;
}

// Return the number of descendants (and self)
int xml_element::descendants() {
	// Include self
	int result = 1;
	// For each child add its descendant count
	for (int i = 0; i < count(); i++) {
		result += child(i)->descendants();
	}
	return result;
}

// Return the type of element
xml_element::element_t xml_element::type() {
	return type_;
}