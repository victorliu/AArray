#include "AArray.h"
#include <sstream>
#include <cmath>

// static members
std::string AArray::serialized_null("null");
std::string AArray::serialized_true("true");
std::string AArray::serialized_false("false");
std::string AArray::serialized_Array("Array");
std::string AArray::serialized_keyval_sep("=>");
char AArray::serialized_quote = '\'';
char AArray::serialized_pair_sep = ',';
char AArray::serialized_array_begin = '{';
char AArray::serialized_array_end = '}';

AArray::Value::Value():type(TYPE_NULL){}

AArray::Value::Value(const AArray &a):type(TYPE_ARRAY){
	value.val_array = new AArray(a);
}
AArray::Value::Value(int n):type(TYPE_INT){
	value.val_int = n;
}
AArray::Value::Value(double d):type(TYPE_REAL){
	value.val_real = d;
}
AArray::Value::Value(const std::string &s):type(TYPE_STRING){
	value.val_str = new std::string(s);
}
AArray::Value::Value(const char *s):type(TYPE_STRING){
	value.val_str = new std::string(s);
}
AArray::Value::Value(bool b):type(TYPE_BOOL){
	value.val_bool = b;
}
	
AArray::Value::Value(const AArray::Value &a):type(a.type){
	switch(a.type){
	case TYPE_ARRAY:
		value.val_array = new AArray(*(a.value.val_array));
		break;
	case TYPE_STRING:
		value.val_str = new std::string(*(a.value.val_str));
		break;
	default:
		value = a.value;
		break;
	}
}
AArray::Value::~Value(){
	switch(type){
	case TYPE_ARRAY:
		delete value.val_array;
		break;
	case TYPE_STRING:
		delete value.val_str;
		break;
	default:
		break;
	}
}

AArray::Value& AArray::Value::operator=(const AArray::Value &a){
	if(this != &a){
		switch(type){
		case TYPE_ARRAY:
			delete value.val_array;
			break;
		case TYPE_STRING:
			delete value.val_str;
			break;
		default:
			break;
		}

		type = a.type;

		switch(a.type){
		case TYPE_ARRAY:
			value.val_array = new AArray(*(a.value.val_array));
			break;
		case TYPE_STRING:
			value.val_str = new std::string(*(a.value.val_str));
			break;
		default:
			value = a.value;
			break;
		}
	}
	return *this;
}

std::ostream& operator<<(std::ostream &os, const AArray::value_type &a){
	switch(a.type){
	case AArray::TYPE_NULL:
		os << AArray::serialized_null;
		break;
	case AArray::TYPE_ARRAY:
		os << AArray::serialized_Array;
		break;
	case AArray::TYPE_INT:
		os << a.value.val_int;
		break;
	case AArray::TYPE_REAL:
		os << a.value.val_real;
		break;
	case AArray::TYPE_STRING:
		// escape all single quotes
		{
			std::string strcopy(*(a.value.val_str));
			std::string escaped_quote("\\"); escaped_quote += AArray::serialized_quote;
			size_t qpos = 0;
			while(std::string::npos != (qpos = strcopy.find(AArray::serialized_quote, qpos))){
				strcopy.replace(qpos, 1, escaped_quote);
				qpos += 2;
			}
			os << AArray::serialized_quote << strcopy << AArray::serialized_quote;
		}
		break;
	case AArray::TYPE_BOOL:
		if(a.value.val_bool){ os << AArray::serialized_true; }else{ os << AArray::serialized_false; }
		break;
	default:
		os << "[unknown type]";
		break;
	}
	return os;
}

bool AArray::Value::is_null() const{ return TYPE_NULL == type; }

AArray::Value& AArray::Value::operator[](int index){
	switch(type){
	case AArray::TYPE_ARRAY:
		return (*value.val_array)[index];
	case AArray::TYPE_STRING:
		delete value.val_str;
		// fall through
	default:
		type = TYPE_ARRAY;
		value.val_array = new AArray();
		return (*value.val_array)[index];
	}
}
AArray::Value& AArray::Value::operator[](const std::string &key){
	switch(type){
	case AArray::TYPE_ARRAY:
		return (*value.val_array)[key];
	case AArray::TYPE_STRING:
		delete value.val_str;
		// fall through
	default:
		type = TYPE_ARRAY;
		value.val_array = new AArray();
		return (*value.val_array)[key];
	}
}
AArray::Value& AArray::Value::operator[](const char *key){
	switch(type){
	case AArray::TYPE_ARRAY:
		return (*value.val_array)[key];
	case AArray::TYPE_STRING:
		delete value.val_str;
		// fall through
	default:
		type = TYPE_ARRAY;
		value.val_array = new AArray();
		return (*value.val_array)[key];
	}
}

double AArray::Value::numeric_value() const{
	switch(type){
	case AArray::TYPE_REAL:
		return value.val_real;
	case AArray::TYPE_INT:
		return (double)(value.val_int);
	case AArray::TYPE_BOOL:
		return (value.val_bool ? 1.0 : 0.0);
	case AArray::TYPE_STRING:
		// if can be made into a number then return it, else 0
	default:
		return 0.0;
	}
}

///////////////////////////

AArray::AArray(){
}

AArray::AArray(const AArray &a):
	map_int(a.map_int),
	map_str(a.map_str)
{
}

AArray::value_type& AArray::operator[](int index){
	if(index < 0){ index += map_int.size(); }
	if(index < 0){ // if still negative, then add some elements and return first
		map_int.insert(map_int.end(), -index, AArray::value_type());
		return map_int[0];
	}else{
		if(index < map_int.size()){
			return map_int[index];
		}else{
			size_t n_more = index+1 - map_int.size();
			map_int.insert(map_int.end(), n_more, AArray::value_type());
			return map_int[index];
		}
	}
	// return map_int[index];
}
AArray::value_type AArray::operator[](int index) const{
	if(index < 0){ index += map_int.size(); }
	if(index < 0 || (size_t)index >= map_int.size()){ return AArray::value_type(); }
	else{
		return map_int[index];
	}
	/*
	int_part_t::const_iterator i = map_int.find(index);
	if(map_int.end() == i){
		return AArray::value_type();
	}else{
		return AArray::value_type(i->second);
	}
	*/
}

AArray::value_type& AArray::operator[](const std::string &key){
	return map_str[key];
}
AArray::value_type AArray::operator[](const std::string &key) const{
	str_part_t::const_iterator i = map_str.find(key);
	if(map_str.end() == i){
		return AArray::value_type();
	}else{
		return AArray::value_type(i->second);
	}
}
AArray::value_type& AArray::operator[](const char *key){
	return map_str[key];
}
AArray::value_type AArray::operator[](const char *key) const{
	str_part_t::const_iterator i = map_str.find(key);
	if(map_str.end() == i){
		return AArray::value_type();
	}else{
		return AArray::value_type(i->second);
	}
}

void AArray::clear(){
	map_int.clear();
	map_str.clear();
}
size_t AArray::count() const{
	return map_int.size() + map_str.size();
}
bool AArray::is_set(int index) const{
	//return map_int.find(index) != map_int.end();
	return map_int.size() > index;
}
bool AArray::is_set(const std::string &key) const{
	return map_str.find(key) != map_str.end();
}
bool AArray::is_set(const char *key) const{
	return map_str.find(key) != map_str.end();
}
void AArray::get_str_keys(std::vector<std::string> &keys) const{
	keys.erase(keys.begin(), keys.end());
	keys.reserve(map_str.size());
	for(str_part_t::const_iterator i = map_str.begin(); i != map_str.end(); ++i){
		keys.push_back(i->first);
	}
}
std::ostream& operator<<(std::ostream &os, const AArray &a){
	os << AArray::serialized_Array << std::endl;
	return a.print_r_helper(os, 0);
}

std::ostream& AArray::print_r_helper(std::ostream &os, int level) const{
	static const char *indent_str = "  ";
	static const char *half_indent_str = " ";

	std::stringstream spaces;
	for(int i = 0; i < level; ++i){ spaces << indent_str; }
	
	os << spaces.str() << half_indent_str << serialized_array_begin << std::endl;
	
	size_t index = 0;
	for(int_part_t::const_iterator i = map_int.begin(); i != map_int.end(); ++i, ++index){
		os << spaces.str() << indent_str << index/*i->first*/ << ' ' << serialized_keyval_sep << ' ';
		os << *i; //i->second;
		if(TYPE_ARRAY == i->type/*i->second.type*/){
			os << std::endl;
			i->/*i->second.*/value.val_array->print_r_helper(os, level+1);
		}
		if('\0' != serialized_pair_sep){
			os << serialized_pair_sep;
		}
		os << std::endl;
	}
	for(str_part_t::const_iterator i = map_str.begin(); i != map_str.end(); ++i){
		os << spaces.str() << indent_str << serialized_quote << i->first << serialized_quote << ' ' << serialized_keyval_sep << ' ';
		os << i->second;
		if(TYPE_ARRAY == i->second.type){
			os << std::endl;
			i->second.value.val_array->print_r_helper(os, level+1);
		}
		if('\0' != serialized_pair_sep){
			os << serialized_pair_sep;
		}
		os << std::endl;
	}
	
	os << spaces.str() << half_indent_str << serialized_array_end;
	return os;
}

static void skip_whitespace(std::istream &is, size_t *line){
	//while(isspace(is.peek())){ is.ignore(1); }
	if(NULL == line){
		is >> std::ws;
	}else{
		char c;
		while(isspace(c = is.peek())){
			// newlines can be \n, \r, or \r\n
			if(c == '\n'){
				is.ignore(1);
				(*line)++;
			}else if(c == '\r'){
				is.ignore(1);
				if('\n' == is.peek()){
					is.ignore(1);
				}
				(*line)++;
			}else{
				is.ignore(1);
			}
		}
	}
}
static bool match_char(std::istream &is, char c){
	if(is.peek() == c){
		is.ignore(1);
		return true;
	}else{
		is.setstate(is.rdstate() | std::ios_base::failbit);
		return false;
	}
}
static bool match_string(std::istream &is, const char *str){
	size_t nstr = strlen(str);
	while(nstr){
		if(match_char(is, *str)){
			++str;
			--nstr;
		}else{ return false; }
	}
	return true;
}
static bool match_string(std::istream &is, const std::string &str){
	for(size_t i = 0; i < str.size(); ++i){
		if(!match_char(is, str[i])){ return false; }
	}
	return true;
}
// expected to have the quote delimiters still sitting in the input stream
static bool read_quoted_string(std::istream &is, std::string &str, char quote_char, size_t *line){
	str.clear();
	if(quote_char == is.peek()){
		is.ignore(1);
	}else{
		is.setstate(is.rdstate() | std::ios_base::failbit);
		return false;
	}
	std::getline(is, str, quote_char);
	if(is.fail()){ return false; }
	if(str.size() == 0){ return true; }
	if(NULL != line){ // count newlines in the string
		size_t pos = 0;
		size_t count = 0;
		while(std::string::npos != (pos = str.find('\n', pos))){
			(*line)++;
			pos++;
		}
	}
	while('\\' == str[str.size()-1]){ // if this was an escaped quote, keep going
		str[str.size()-1] = quote_char;
		std::string next_piece;
		std::getline(is, next_piece, quote_char);
		if(NULL != line){ // count newlines in the string
			size_t pos = 0;
			size_t count = 0;
			while(std::string::npos != (pos = next_piece.find('\n', pos))){
				(*line)++;
				pos++;
			}
		}
		str += next_piece;
		if(is.fail()){ return false; }
	}
	return true;
}
bool AArray::parse(std::istream& is, size_t *error_line_number){
	clear();
	
	// can possibly optimize this with skipws, etc
	skip_whitespace(is, error_line_number);
	if(!match_string(is, serialized_Array)){ return false; }
	skip_whitespace(is, error_line_number);
	if(!match_char(is, serialized_array_begin)){ return false; }
	skip_whitespace(is, error_line_number);
	
	while((is.peek() != serialized_array_end) && is.good()){ // not empty array
		bool key_is_string = true;
		std::string key_str;
		size_t key_int;
		if(is.peek() == serialized_quote){ // string key
			if(!read_quoted_string(is, key_str, serialized_quote, error_line_number)){ return false; }
		}else{ // numeric key
			is >> key_int;
			key_is_string = false;
		}
		
		// read in the => arrow
		skip_whitespace(is, error_line_number);
		if(!match_string(is, serialized_keyval_sep)){ return false; }
		skip_whitespace(is, error_line_number);
		
		// add a blank entry with the key first
		AArray::Value *newptr;
		if(key_is_string){
			newptr = &(((*this)[key_str]) = AArray::Value());
//			std::cout << key_str << " " << *error_line_number << std::endl;
		}else{
			newptr = &(((*this)[key_int]) = AArray::Value());
//			std::cout << key_int << " " << *error_line_number << std::endl;
		}
		
		// Determine the type of the value
		char nextchar = is.peek();
		if(serialized_quote == nextchar){ // string
			std::string value_str;
			if(!read_quoted_string(is, value_str, serialized_quote, error_line_number)){ return false; }
			(*newptr) = AArray::Value(value_str);
		}else if(serialized_true[0] == nextchar){ // bool true
			if(!match_string(is, serialized_true)){ return false; }
			(*newptr) = AArray::Value(true);
		}else if(serialized_false[0] == nextchar){ // bool false
			if(!match_string(is, serialized_false)){ return false; }
			(*newptr) = AArray::Value(false);
		}else if((serialized_Array.size() > 1 && serialized_Array[0] == nextchar) || (serialized_array_begin == nextchar)){ // Array
			(*newptr) = AArray::Value(AArray());
			newptr->value.val_array->parse(is, error_line_number);
			if(is.fail()){ return false; }
		}else if(serialized_null[0] == nextchar){ // null
			if(!match_string(is, serialized_null)){ return false; }
			// should already be null
			//(*newptr) = AArray::Value();
		}else{ // assume a number
			// read in as a double first
			double num;
			is >> num;
			if(is.fail()){ return false; }
			
			// was it an integer?
			double intpart;
			if((0 == std::modf(num, &intpart)) && (intpart <= (double)INT_MAX) && (intpart >= (double)INT_MIN)){
				(*newptr) = AArray::Value((int)intpart);
			}else{
				(*newptr) = AArray::Value(num);
			}
		}
	
		// grab comma, optionally
		skip_whitespace(is, error_line_number);
		if(('\0' != serialized_pair_sep) && (is.peek() == serialized_pair_sep)){
			is.ignore(1);
		}
		skip_whitespace(is, error_line_number);
	}
	
	match_char(is, serialized_array_end);

	return true;
}

std::istream& operator>>(std::istream& is, AArray& arr)/* throw (ios_base::failure)*/{
	arr.parse(is, NULL);
	return is;
}


void AArray::set_serialization_string_null(const char *str){ serialized_null = str; }
void AArray::set_serialization_string_bools(const char *str_true, const char *str_false){ serialized_true = str_true; serialized_false = str_false; }
void AArray::set_serialization_string_Array(const char *str){ serialized_Array = str; }
void AArray::set_serialization_string_keyval_sep(const char *str){ serialized_keyval_sep = str; }
void AArray::set_serialization_char_quote(char c){ serialized_quote = c; }
void AArray::set_serialization_char_pair_sep(char c){ serialized_pair_sep = c; }
void AArray::set_serialization_char_array_delimiters(char begin, char end){ serialized_array_begin = begin; serialized_array_end = end; }
