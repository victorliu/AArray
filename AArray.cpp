#include "AArray.h"
#include <sstream>
#include <fstream>
#include <cmath>

// static members
std::string AArray::serialized_null("null");
std::string AArray::serialized_true("true");
std::string AArray::serialized_false("false");
std::string AArray::serialized_Array("Array");
std::string AArray::serialized_keyval_sep("=>");
std::string AArray::include_string("include");
char AArray::serialized_quote = '\'';
char AArray::serialized_pair_sep = ',';
char AArray::serialized_array_begin = '{';
char AArray::serialized_array_end = '}';
char AArray::serialized_continuation = '.';
char AArray::inline_comment_begin = '#';

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
size_t AArray::count_int() const{
	return map_int.size();
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

void AArray::skip_whitespace(std::istream &is, size_t &line){
	//while(isspace(is.peek())){ is.ignore(1); }
	//is >> std::ws;
	// parser inspired by JSON_parser.c
	enum states{
		GO,//S_DEFAULT,           // default state, not in comment
		SL,//S_GOT_SLASH,         // got the first slash, possibly starting a comment
		LN,//S_IN_LINE_COMMENT,   // in a single line comment
		BK,//S_IN_BLOCK_COMMENT,  // in a block (multi-line) comment
		ST,//S_GOT_STAR           // got (possibly) ending star of block comment,
		// these two are not real states; reaching them exits the function
		//S_ERROR_SLASH,       // non space, need to put back a slash
		//S_ERROR,             // non space
		NR_STATES
	};
	enum actions{ // these must be negative (to be different from states)
		RET = -1, // return
		RPS = -2  // return and put back a slash character
	};
	enum classes{
		C_SLASH,
		C_HASH,
		C_STAR,
		C_EOL,
		C_SPACE,
		C_ETC,
		NR_CLASSES
	};
		
	static int ascii_class[64] = {
	// This array maps the lower 64 ASCII characters into character classes.
	// The remaining characters should be mapped to C_ETC.
		C_ETC,   C_ETC,   C_ETC,  C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
		C_ETC,   C_SPACE, C_EOL,  C_ETC,   C_ETC,   C_EOL,   C_ETC,   C_ETC,
		C_ETC,   C_ETC,   C_ETC,  C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
		C_ETC,   C_ETC,   C_ETC,  C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

		C_SPACE, C_ETC,   C_ETC,  C_HASH,  C_ETC,   C_ETC,   C_ETC,   C_ETC,
		C_ETC,   C_ETC,   C_STAR, C_ETC,   C_ETC,   C_ETC,   C_ETC, C_SLASH,
		C_ETC,   C_ETC,   C_ETC,  C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
		C_ETC,   C_ETC,   C_ETC,  C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC
	};

	
	/*
	State transition table:
	State\Char |  /   |  #   |  *   |  \n  | space (non \n) | anything else
	-----------------------------------------------------------------------
	       DEF | GOT/ | LINE | ERR  | DEF  | DEF            | ERR
	      GOT/ | LINE | ERR/ | BLOK | ERR/ | ERR/           | ERR/
	      LINE | LINE | LINE | LINE | DEF  | LINE           | LINE
	      BLOK | BLOK | BLOK | GOT* | BLOK | BLOK           | BLOK
	      GOT* | DEF  | BLOK | GOT* | BLOK | BLOK           | BLOK
	      ERR/ | ERR  | ERR  | ERR  | ERR  | ERR            | ERR
	       ERR | ERR  | ERR  | ERR  | ERR  | ERR            | ERR
	When hitting the error state, we may have to put back some chars into the stream
	*/
	
		
	static int state_transition_table[NR_STATES][NR_CLASSES] = {
	// The state transition table takes the current state and the current symbol,
	// and returns either a new state or an action.

	//       /    #    *   EOL space etc
	/*GO*/ {GO , LN , RET, GO , GO , RET},
	/*SL*/ {LN , RPS, BK , RPS, RPS, RPS},
	/*LN*/ {LN , LN , LN , GO , LN , LN },
	/*BK*/ {BK , BK , ST , BK , BK , BK },
	/*ST*/ {GO , BK , ST , BK , BK , BK }
	};

	int state = GO;
	while(1){
		char c = is.peek();
		int cclass = (c < 64) ? ascii_class[c] : C_ETC;
		state = state_transition_table[state][cclass];
		if(state < 0){
			if(RET == state){
				return;
			}else if(RPS == state){
				is.putback('/');
				return;
			}
		}else{
			is.ignore(1);
			if(C_EOL == cclass){
				++line;
				if('\r' == c){ // possibly need to eat a \n
					if(is.peek() == '\n'){
						is.ignore(1);
					}
				}
			}
		}
	}
	/*
	parse_state = S_DEFAULT;
	while(1){
		char c = is.peek();
		switch(parse_state){
		case S_DEFAULT:
			if('/' == c){
				parse_state = S_GOT_SLASH;
				is.ignore(1);
			}else if(('\0' != inline_comment_begin) && (inline_comment_begin == c)){
				parse_state = S_IN_LINE_COMMENT;
				is.ignore(1);
			}else if('\n' == c){
				is.ignore(1);
				line++;
			}else if('\r' == c){
				is.ignore(1);
				if('\n' == is.peek()){
					is.ignore(1);
				}
				line++;
			}else if(isspace(c)){
				is.ignore(1);
			}else{
				return;
			}
			break;
		case S_GOT_SLASH:
			if('/' == c){
				parse_state = S_IN_LINE_COMMENT;
				is.ignore(1);
			}else if('*' == c){
				parse_state = S_IN_BLOCK_COMMENT;
				is.ignore(1);
			}else{ // anything else, this slash was non-space, push it back
				is.putback('/');
				return;
			}
			break;
		case S_IN_LINE_COMMENT:
			if('\n' == c){
				is.ignore(1);
				line++;
				parse_state = S_DEFAULT;
			}else if('\r' == c){
				is.ignore(1);
				if('\n' == is.peek()){
					is.ignore(1);
				}
				parse_state = S_DEFAULT;
				line++;
			}else{ // anything else, stay in the comment
				is.ignore(1);
			}
			break;
		case S_IN_BLOCK_COMMENT:
			if('*' == c){
				parse_state = S_GOT_STAR;
				is.ignore(1);
			}else if('\n' == c){
				is.ignore(1);
				line++;
			}else if('\r' == c){
				is.ignore(1);
				if('\n' == is.peek()){
					is.ignore(1);
				}
				line++;
			}else if(isspace(c)){
				is.ignore(1);
			}else{ // anything else, stay in the comment
				is.ignore(1);
			}
			break;
		case S_GOT_STAR:
			if('*' == c){
				is.ignore(1);
			}else if('/' == c){
				parse_state = S_DEFAULT;
				is.ignore(1);
			}else if('\n' == c){
				is.ignore(1);
				parse_state = S_IN_BLOCK_COMMENT;
				line++;
			}else if('\r' == c){
				is.ignore(1);
				if('\n' == is.peek()){
					is.ignore(1);
				}
				parse_state = S_IN_BLOCK_COMMENT;
				line++;
			}else{ // anything else, still in block comment
				parse_state = S_IN_BLOCK_COMMENT;
				is.ignore(1);
			}
			break;
		default: // should never get here
			return;
		}
	}
	*/
/*
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
*/
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
bool AArray::read_quoted_string(std::istream &is, std::string &str, size_t &line){
	str.clear();
	if(is.peek() == serialized_quote){
		is.ignore(1);
	}else{
		is.setstate(is.rdstate() | std::ios_base::failbit);
		return false;
	}
	std::getline(is, str, serialized_quote);
	if(is.fail()){ return false; }
	if(str.size() == 0){ return true; }
	{ // count newlines in the string
		size_t pos = 0;
		size_t count = 0;
		while(std::string::npos != (pos = str.find('\n', pos))){
			line++;
			pos++;
		}
	}
	while('\\' == str[str.size()-1]){ // if this was an escaped quote, keep going
		str[str.size()-1] = serialized_quote;
		std::string next_piece;
		std::getline(is, next_piece, serialized_quote);
		{ // count newlines in the string
			size_t pos = 0;
			size_t count = 0;
			while(std::string::npos != (pos = next_piece.find('\n', pos))){
				line++;
				pos++;
			}
		}
		str += next_piece;
		if(is.fail()){ return false; }
	}
	return true;
}
bool AArray::parse_value(std::istream& is, AArray::Value &val, std::ostream *serr, size_t &line){
	// Determine the type of the value
	char nextchar = is.peek();
	if(serialized_quote == nextchar){ // string
		std::string value_str;
		if(!read_quoted_string(is, value_str, line)){
			if(NULL != serr){ (*serr) << "Error parsing string on line " << line << std::endl; }
			return false;
		}
		val = AArray::Value(value_str);
	}else if(serialized_true[0] == nextchar){ // bool true
		if(!match_string(is, serialized_true)){
			if(NULL != serr){ (*serr) << "Parse error on line " << line << std::endl; }
			return false;
		}
		val = AArray::Value(true);
	}else if(serialized_false[0] == nextchar){ // bool false
		if(!match_string(is, serialized_false)){
			if(NULL != serr){ (*serr) << "Parse error on line " << line << std::endl; }
			return false;
		}
		val = AArray::Value(false);
	}else if((serialized_Array.size() > 1 && serialized_Array[0] == nextchar) || (serialized_array_begin == nextchar)){ // Array
		val = AArray::Value(AArray());
		val.value.val_array->parse(is, serr, line);
		if(is.fail()){ return false; }
	}else if(serialized_null[0] == nextchar){ // null
		if(!match_string(is, serialized_null)){
			if(NULL != serr){ (*serr) << "Parse error on line " << line << std::endl; }
			return false;
		}
		// should already be null
		//(*newptr) = AArray::Value();
	}else if(include_string[0] == nextchar){
		if(!match_string(is, include_string)){ return false; }
		skip_whitespace(is, line);
		if(!match_char(is, '(')){ return false; }
		skip_whitespace(is, line);
		std::string filename;
		if(!read_quoted_string(is, filename, line)){ return false; }
		skip_whitespace(is, line);
		if(!match_char(is, ')')){ return false; }
		std::ifstream fin(filename.c_str());
		size_t fline = 1;
		if(!parse_value(fin, val, serr, fline)){
			if(NULL != serr){ (*serr) << " from file: " << filename << std::endl; }
		}
		fin.close();
	}else{ // assume a number
		// read in as a double first
		double num;
		is >> num;
		if(is.fail()){
			if(NULL != serr){ (*serr) << "Parse error on line " << line << std::endl; }
			return false;
		}
		
		// was it an integer?
		double intpart;
		if((0 == std::modf(num, &intpart)) && (intpart <= (double)INT_MAX) && (intpart >= (double)INT_MIN)){
			val = AArray::Value((int)intpart);
		}else{
			val = AArray::Value(num);
		}
	}
	return true;
}
bool AArray::parse(std::istream& is, std::ostream *serr, size_t line){
	clear();
	
	// can possibly optimize this with skipws, etc
	skip_whitespace(is, line);
	if(!match_string(is, serialized_Array)){ return false; }
	skip_whitespace(is, line);
	if(!match_char(is, serialized_array_begin)){ return false; }
	skip_whitespace(is, line);
	
	while((is.peek() != serialized_array_end) && is.good()){ // not empty array
		bool key_is_string = true;
		std::string key_str;
		size_t key_int;
		if(is.peek() == serialized_quote){ // string key
			if(!read_quoted_string(is, key_str, line)){
				if(NULL != serr){ (*serr) << "Invalid string key on line " << line << std::endl; }
				return false;
			}
		}else if(is.peek() == serialized_continuation){
			key_is_string = false;
			is.ignore(1);
			key_int = count_int();
		}else{ // numeric key
			is >> key_int;
			if(is.fail()){
				if(NULL != serr){ (*serr) << "Invalid key on line " << line << std::endl; }
				return false;
			}
			key_is_string = false;
		}
		
		// read in the => arrow
		skip_whitespace(is, line);
		if(!match_string(is, serialized_keyval_sep)){
			if(NULL != serr){ (*serr) << "Expected " << serialized_keyval_sep << " on line " << line << std::endl; }
			return false;
		}
		skip_whitespace(is, line);
		
		// add a blank entry with the key first
		AArray::Value *newptr;
		if(key_is_string){
			newptr = &(((*this)[key_str]) = AArray::Value());
//			std::cout << key_str << " " << line << std::endl;
		}else{
			newptr = &(((*this)[key_int]) = AArray::Value());
//			std::cout << key_int << " " << line << std::endl;
		}
		
		if(!parse_value(is, *newptr, serr, line)){
			return false;
		}
	
		// grab comma, optionally
		skip_whitespace(is, line);
		if(('\0' != serialized_pair_sep) && (is.peek() == serialized_pair_sep)){
			is.ignore(1);
		}
		skip_whitespace(is, line);
	}
	
	match_char(is, serialized_array_end);

	return true;
}

std::istream& operator>>(std::istream& is, AArray& arr)/* throw (ios_base::failure)*/{
	
	arr.parse(is, NULL, 1);
	return is;
}


bool operator==(const AArray::Value &a, const AArray::Value &b){
	if(a.type != b.type){ return false; }
	switch(a.type){
	case AArray::TYPE_ARRAY:
		return (*(a.value.val_array)) == (*(b.value.val_array));
	case AArray::TYPE_INT:
		return a.value.val_int == b.value.val_int;
	case AArray::TYPE_REAL:
		return a.value.val_real == b.value.val_real;
	case AArray::TYPE_STRING:
		return (*(a.value.val_str)) == (*(b.value.val_str));
	case AArray::TYPE_BOOL:
		return a.value.val_bool == b.value.val_bool;
	default: // null type
		return true;
	}
}
bool operator==(const AArray &a, const AArray &b){
	return (a.map_int == b.map_int) && (a.map_str == b.map_str);
}


void AArray::set_serialization_string_null(const char *str){ serialized_null = str; }
void AArray::set_serialization_string_bools(const char *str_true, const char *str_false){ serialized_true = str_true; serialized_false = str_false; }
void AArray::set_serialization_string_Array(const char *str){ serialized_Array = str; }
void AArray::set_serialization_string_keyval_sep(const char *str){ serialized_keyval_sep = str; }
void AArray::set_include_string(const char *str){ include_string = str; }
void AArray::set_serialization_char_quote(char c){ serialized_quote = c; }
void AArray::set_serialization_char_pair_sep(char c){ serialized_pair_sep = c; }
void AArray::set_serialization_char_array_delimiters(char begin, char end){ serialized_array_begin = begin; serialized_array_end = end; }
void AArray::set_inline_comment_char(char c){ inline_comment_begin = c; }
