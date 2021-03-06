#ifndef _AARRAY_H
#define _AARRAY_H

//          DO WHATEVER THE FUCK YOU WANT TO PUBLIC LICENSE
// 
// Copyright (C) 2009 Victor Liu
//  Stanford, CA, USA
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
// 
//          DO WHATEVER THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
// 
//  0. You just DO WHATEVER THE FUCK YOU WANT TO.

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <list>

// AArray
//  Description:
//   A PHP styled associative array for C++.
//  Details:
//   An associative array can behave like a vector, map, hashtable,
//   dictionary, list, set, etc. It is a set of key-value pairs where
//   the key can be either an integer or a std::string. Internally the
//   two types of keys are stored in different structures. In particular,
//   accessing an arbitrary integer key will allocate all uninitialized
//   integer keys below it to nulls.
//     The values can be a variety of types: null (uninitialized), bool,
//   int, real (double), string (std::string), or another AArray. In this
//   way, multidimensional arrays can be created, as well as tree structures,
//   and general hierarchical data structures.
//     One of the most powerful features is the serialization/deserialization
//   via stream operators. The entire AArray structure can be (human-readably)
//   serialized to a stream and then read back in again. This makes the
//   AArray an attractive lightweight structured data parser. Also, the
//   serialization format can be customized to provide a reasonable syntax
//   for hand editting.
//  Example:
//   AArray array;
//   array[3] = AArray::Value("foo"); // elements 0-2 are set to null
//   array["It's just another quote"] = AArray::Value(3.426);
//   array[2][3][4] = AArray::Value(true);
//   AArray array2(array); // deep copy

class AArray{
public:
	// The type of a value
	enum Type{
		TYPE_NULL,    // an empty value
		TYPE_ARRAY,   // another array
		TYPE_INT,     // integer
		TYPE_REAL,    // a real number (double
		TYPE_STRING,  // string
		TYPE_BOOL     // boolean value
	};
	enum KeyType{
		KEY_INT,
		KEY_STRING
	};
	class Key{
		KeyType t;
	public:
		int val_int;
		std::string val_str;
		explicit Key(int i):t(KEY_INT),val_int(i){}
		explicit Key(const std::string &s):t(KEY_STRING),val_str(s){}
		explicit Key(const char *s):t(KEY_STRING),val_str(s){}
		KeyType type() const{ return t; }
	};
	typedef std::list<Key> Path;
	struct Value{
		Type type;
		union{
			AArray *val_array;
			int val_int;
			double val_real;
			std::string *val_str;
			bool val_bool;
		} value;
		
		Value();
		explicit Value(const AArray &a);
		explicit Value(int n);
		explicit Value(double d);
		explicit Value(const std::string &s);
		explicit Value(const char *s);
		explicit Value(bool b);
		
		Value(const Value &a); // deep copy constructor
		~Value();

		Value& operator=(const Value &a);
		
		bool is_null() const;

		// Automatic promotion to an empty array
		Value& operator[](int index);
		Value& operator[](const std::string &key);
		Value& operator[](const char *key);

		size_t count() const; // returns 0 if this is not an array, otherwise array's count
		double numeric_value() const;
	};
	typedef Value value_type;
private:
	typedef std::vector<value_type> int_part_t;
	typedef std::map<std::string, value_type> str_part_t;
	int_part_t map_int;
	str_part_t map_str;
	
	// Preferences for how the serialized output should be formatted
	static std::string serialized_null;
	static std::string serialized_true;
	static std::string serialized_false;
	static std::string serialized_Array;
	static std::string serialized_keyval_sep;
	static std::string include_string;
	static char serialized_quote;
	static char serialized_pair_sep;
	static char serialized_array_begin, serialized_array_end;
	static char serialized_continuation;
	static char inline_comment_begin;
public:
	AArray();
	AArray(const AArray &a);
	
	// Note that the const versions deep-copy the value out
	value_type& operator[](int index);
	value_type  operator[](int index) const;
	value_type& operator[](const std::string &key);
	value_type  operator[](const std::string &key) const;
	value_type& operator[](const char *key);
	value_type  operator[](const char *key) const;
	
	void clear();
	size_t count() const; // total number of elements in the int and str parts
	void reserve(size_t num); // reserves a num elements in the int part
	bool is_set(int index) const;
	bool is_set(const std::string &key) const;
	bool is_set(const char *key) const;
	
	size_t count_int() const; // number of elements in the int part
	void get_str_keys(std::vector<std::string> &keys) const;
	
	bool parse(std::istream& is, std::ostream *serr, size_t starting_line_number = 1);
protected:
	bool parse_value(std::istream& is, AArray::Value &val, std::ostream *serr, size_t &line);
public:
	
	// Non-thread-safe members
	static void set_serialization_string_null(const char *str);        // [1]
	static void set_serialization_string_bools(const char *str_true,
	                                           const char *str_false); // [1]
	static void set_serialization_string_Array(const char *str);     // [1,2]
	static void set_include_string(const char *str);                   // [1]
	static void set_serialization_string_keyval_sep(const char *str);
	static void set_serialization_char_quote(char c);
	static void set_serialization_char_pair_sep(char c);               // [3]
	static void set_serialization_char_array_delimiters(char begin, char end);
	static char set_serialization_char_continuation(char c);           // [4]
	static void set_inline_comment_char(char c);                       // [5]
	// Footnotes to the above:
	// [1] The strings used in these functions must all start with
	//     different characters.
	// [2] Set to an empty string to disable outputting/inputting it.
	// [3] Set to \0 to disable outputting/inputting it.
	// [4] Set to \0 to disable list continuation during parsing. Cannot
	//     be the same as the quote character.
	// [5] Set to \0 to disable single character begun inline comments.
	//     Must be different from quote character.

	// Algorithms
	
	// Deeply iterates over all key-value pairs.
	// The pointer data is passed on to the function
	// If the function returns false, the iteration ends.
	typedef bool (*KeyValueFunction)(const Key &key,
	                                 Value &value, void *data);
	void for_each(KeyValueFunction func, void *data);
	// Deeply iterates over all key-value pairs while providing the absolute
	// key path for each value. data is passed on to the function
	// If the function returns false, the iteration ends.
	typedef bool (*PathValueFunction)(const Path &path,
	                                  Value &value, void *data);
	void for_each_path(PathValueFunction func, void *data);
private:
	static bool for_each_helper(AArray &A, AArray::KeyValueFunction func,
	                            void *data);
	static bool for_each_path_helper(AArray &A, AArray::Path &path,
	                                AArray::PathValueFunction func,
	                                void *data);


protected:
	static void AArray::skip_whitespace(std::istream &is, size_t &line);
	static bool read_quoted_string(std::istream &is, std::string &str, size_t &line);
protected:
	friend std::ostream& operator<<(std::ostream &os, const AArray &a);
	friend std::ostream& operator<<(std::ostream &os, 
	                                const AArray::value_type &a);
	friend bool operator==(const AArray &a, const AArray &b);
	std::ostream& print_r_helper(std::ostream &os, int level) const;
};

bool operator==(const AArray &a, const AArray &b);
bool operator==(const AArray::Value &a, const AArray::Value &b);
std::ostream& operator<<(std::ostream &os, const AArray::value_type &a);
std::ostream& operator<<(std::ostream &os, const AArray &a);
std::istream& operator>>(std::istream& is, AArray& arr);

#endif
