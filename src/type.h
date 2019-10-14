#ifndef TYPE_H
#define TYPE_H
#include <inttypes.h>
struct type{
	enum {var,arr,func,ptr} general_type;
	union{
		uint32_t id;//the info about vars - see decl_parser.c
		struct {//the info about arrays
			struct type* t;
			unsigned size;	//length of the array
		};
		struct {//the info about funcs
			struct type* ret_val;
			unsigned argc;
			struct type* argv;
		};
		struct {//the info about ptrs
			struct type* target;//target type
			uint32_t traits;//the traits applied to the
							//ptr itself - see decl_parser.c
		};
	};
};

void print_type(const struct type*);

#endif
