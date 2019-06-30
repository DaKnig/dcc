enum primitive {type_int, type_char};

struct type{
	enum {var,arr,func,ptr} general_type;
	union{
		enum primitive p;//the info about vars
		struct {//the info about arrays
			struct type* t;
			unsigned size;	//length of the array
		};
		struct {//the info about funcs
			struct type* ret_val;
			unsigned argc;
			struct type* argv;
		};
		struct type* dest;//the info about funcs
	};
};
