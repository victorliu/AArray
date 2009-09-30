#include "AArray.h"
#include <sstream>

typedef AArray::value_type o;

bool key_val_printer(const AArray::Key &key,  AArray::Value &value, void *data){
	std::ostream &os = *(static_cast<std::ostream*>(data));
	if(key.type() == AArray::KEY_INT){
		os << key.val_int;
	}else{
		os << key.val_str;
	}
	os << " : " << value << std::endl;
}
bool path_val_printer(const AArray::Path &path,  AArray::Value &value, void *data){
	std::ostream &os = *(static_cast<std::ostream*>(data));
	for(AArray::Path::const_iterator key = path.begin(); key != path.end(); ++key){
		if(key->type() == AArray::KEY_INT){
			os << key->val_int;
		}else{
			os << key->val_str;
		}
		os << '/';
	}
	os << value << std::endl;
}

int main(){
	AArray::set_serialization_string_keyval_sep("=");
	AArray::set_serialization_char_array_delimiters('{', '}');
	AArray::set_serialization_string_Array("");
	AArray::set_serialization_char_pair_sep('\0');

	// Generate some array
	AArray a;
	a[2] = o("bla'h");
	a["bar"] = o(AArray());
	a[3] = o(1.2);
	a["foo"] = o(true);
	a[1] = o(40);

	a["bar"][3] = o("hello");

	// Output it
	std::cout << std::endl << a << std::endl;
	std::stringstream ss;
	ss << std::endl << a << std::endl;

	// modify it randomly
	srand(time(0));
	std::string modified_output(ss.str());
	
	// remove all commas (should be ok)
	{
		size_t pos = 0;
		while(std::string::npos != (pos = modified_output.find(',', pos))){
			modified_output.replace(pos, 1, " ");
		}
	}
	//modified_output[rand()%modified_output.size()] = ' ';
	//modified_output[31] = ' ';

	// Read it back in
	std::istringstream iss(modified_output);
	AArray arr;
	if(!arr.parse(iss, &std::cerr)){
		std::cout << "___" << std::endl << modified_output << "~~~" << std::endl;
	}else{
		std::cout << std::endl << arr << std::endl;
	}
	
	// (deep) copy constructor
	AArray bar(arr);
	std::cout << std::endl << bar << std::endl;
	
	std::cout << (bar == arr) << (a == arr) << std::endl;
	
	bar.for_each(&key_val_printer, static_cast<void*>(&std::cout));
	bar.for_each_path(&path_val_printer, static_cast<void*>(&std::cout));
	
	return 0;
	/*
	// Repeat the test to check for memory leaks;
	for(int i = 100; i >= 0; --i){
		AArray a;
		a[2] = o("blah");
		a["bar"] = o(AArray());
		a[3] = o(1.2);
		a["foo"] = o(true);
		a[1] = o(40);

		a["bar"][3] = o();

		for(int j = 100000; j >= 0; --j){
			a[3]["bar"] = o(0);
			a[3] = o();
		}

		if(i == 0){
			std::cout << std::endl << a << std::endl;
		}else{
			std::cout << i << " ";
		}
	}
	*/
	return 0;
}
