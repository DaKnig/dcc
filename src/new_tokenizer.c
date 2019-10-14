//TOOD:
//- string concat

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <ctype.h>

#include <sys/times.h>

#include "C_keywords.h"

enum token_type{
    t_keyword, t_identifier, t_string, t_char, t_float,
    t_unknown, t_punctuator, t_bad_token, t_EOF, t_int
};
struct token{
	char* str;
	enum token_type t;
    union {
        unsigned long long number;
        double floating_number;
    };
    union {
        char string_prefix[4];
        char number_postfix[4];
    };
};
//how long is str in the buffers
// static unsigned buffer_size=1<<15;
// static struct token buffer[2];

// static unsigned int token=0;

struct context {
    FILE* file;
    int stack_size;
    int* stack_head;
    struct token buffer[2];
    unsigned buffer_size;
    int token;
    int stack[];
};
struct token* next(struct context* input);
void print_token(struct token* t);
struct context* create_ctx(FILE* f){
    struct context* c = malloc(sizeof(struct context)+5*sizeof(int));
    c->file=f;
    c->stack_size=5;
    c->stack_head=c->stack;
    c->buffer_size=1<<15;
    c->buffer[0].str=malloc(c->buffer_size);
    c->buffer[1].str=malloc(c->buffer_size);
    assert("malloc"&&c->buffer[0].str&&c->buffer[1].str);
    c->token=0;
    next(c);    // put one valid token into it
    return c;
};
static inline void token_push(struct context* ctx, int c){
    //use at your own risk
    if (ctx->stack_head-ctx->stack == ctx->stack_size)
        fprintf(stderr,"context stack size is reached");
    *ctx->stack_head=c;
    ctx->stack_head++;
}
static inline int token_pop(struct context* ctx) {
    if (ctx->stack_head==ctx->stack)
        fprintf(stderr,"popping empty context");
    ctx->stack_head--;
    return *ctx->stack_head;
}
static inline int token_getc(struct context* ctx) {
    if (ctx->stack_head==ctx->stack)
        return fgetc(ctx->file);
    else
        return token_pop(ctx);
}

static inline void token_ungetc(int c, struct context* ctx) {
    if (c!=EOF &&c!=(char)EOF)
        token_push(ctx,c);
}
static inline int token_feof(struct context* ctx) {
    if (ctx->stack_head==ctx->stack)
        return feof(ctx->file);
    else
        return 0;
}

struct token* get_punctuator(struct context* input){
    //reads a punctuator from the current position in `input`
    //returns NULL on fail, a static token otherwise
    char* s=input->buffer[input->token].str;
    input->buffer[input->token].t=t_punctuator;
    memset(s,0,4); //zero the first 4 bytes
    s[0]=token_getc(input);
    if(strchr("%+(;!&?*>#<|=.{})-~:/[]^,",s[0])==NULL){
        //the character is not a punctuator
        token_ungetc(s[0], input);
        input->buffer[input->token].t=t_unknown;
        return NULL;
    }
    switch(s[0]){
        case ':': case '?': case ';': case ',':
        case '(': case ')': case '[': case ']':
        case '{': case '}': case '~':
            //all those only apear in single-char punctuators
            goto success;
        case '#':
            s[1]=token_getc(input);
            goto skip_equals;
        case '.':
            s[1]=token_getc(input);
            if (s[1] != '.') {
                s++;
                goto push_char; //"."
            }
            //".."
            s[2]=token_getc(input);
            if (s[2] != '.') {  //".."
                token_ungetc(s[2],input);
                token_ungetc('.',input);
                s[1]='\0';  //now s=="."
            }
            goto success;   //"..."
        //other punctuator
    }
    s[1]=token_getc(input);
    if (s[1]=='=')
        goto success;
skip_equals:
    switch (s[0]){
        case '>': case '<':
            if (s[1]!=s[0]) //"<" or ">"
                break;
            s[2] = token_getc(input);
            if (s[2] !='=') {   //">>" or "<<"
                s++;
                break;
            }
            goto success;   //">>=" or "<<="
        case '-':
            if (s[1]=='>')
                goto success;  //"->"
                //fallthrough
        case '+': case '|': case '&': case '#':
            if (s[1]==s[0])
                goto success;  //stuff like "++"
        case '^': case '!': case '*': case '/':
        case '=': case '%': //s[1] != '='
            break;
        default:
        panic:
            fprintf(stderr,"INTERNAL ERROR 100");
            exit(1);
    }
    s++;
push_char:
    token_ungetc(*s,input);
    *s='\0';
success:
    input->token^=1;   //switch buffer
    return &input->buffer[input->token^1]; //return the current buffer
}

// void init(void) {
//     buffer[0].str=malloc(1<<15);
//     buffer[1].str=malloc(1<<15);
//     buffer[0].t=buffer[1].t=t_punctuator;
//     strcpy(buffer[0].str,";");
//     strcpy(buffer[1].str,";");
//         //this is done in order for the first token to be always `;`.
//         //this should not be a problem in a normal program.
// }

static inline int string_in_strings(char* s,const char** strings, int n){
    for (n--;n>=0;n--)
        if (strcmp(strings[n],s) == 0)
            return 1;
    return 0;
}

clock_t st_time;
clock_t en_time;
// void start_clock(){
//     st_time = times(NULL);
// }
// void end_clock(){
//     en_time = times(NULL);
//     fprintf(stderr,"%jd",(intmax_t)(en_time - st_time));
// }
static inline char
        get_escape_sequence(struct context* input, unsigned long long* sum) {
    //assumes that the leading \ has been consumed. puts the next char in *sum.
    //returns a char indicator
    *sum=0;
    char status;
        //'i' = invalid. 'x' = hex. '0' = octal. 's' = simple.
    char counter=0;
    int c=token_getc(input);
    switch(c) {
    case '\'': case '"': case '\\': case '?':       //'"'
        *sum = c;
        status = 's';
        break;
    //"simple escape sequence"
    case 'a' : *sum = '\a'; status = 's'; break;
    case 'b' : *sum = '\b'; status = 's'; break;
    case 'f' : *sum = '\f'; status = 's'; break;
    case 'n' : *sum = '\n'; status = 's'; break;
    case 'r' : *sum = '\r'; status = 's'; break;
    case 't' : *sum = '\t'; status = 's'; break;
    case 'v' : *sum = '\v'; status = 's'; break;
    //"octal escape sequence"
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
        do {
            *sum=*sum*8+c-'0';
            counter++;
            c=token_getc(input);
        } while ( c<='7' && c>='0' && counter<3
                && !token_feof(input));
        token_ungetc(c,input);
        status = '0';
        break;
    case 'x':
        *sum=0;
        c=token_getc(input);
        while(isxdigit(c) && !token_feof(input)) {
            if (isdigit(c))
                c-='0';
            else
                c=tolower(c)-'a'+10;
            *sum=*sum*16+c;
            c=token_getc(input);
        }
        token_ungetc(c,input);
        status ='x';
        break;
    default:
        *sum=c;
        status = 'i';
    }
    return status;
}
static inline struct token* get_string(struct context* input) {
    char* prefix=input->buffer[input->token].string_prefix;
    {   //try to get a string prefix
        memset(prefix,'\0',4);
        prefix[0]=token_getc(input);
        if (prefix[0]=='"') {   //no prefix //"
            prefix[0]='\0';
            goto get_s_char;
        } else if (!strchr("uUL",prefix[0])) {  //if it's not [\"uUL]
            goto stop_0;
        }
            //not a string because a string starts with [\"uUL]         //"
        prefix[1]=token_getc(input);
        if (prefix[1]=='"') {   //prefix is [uUL]           //"
            prefix[1]='\0';
            goto get_s_char;
        } else if (prefix[0]!='u') {    //if [UL][^"]                   //"
            goto stop_1;
        }
        prefix[2]=token_getc(input);
        prefix[3]='\0';
        if (strcmp(prefix,"u8\"")==0)
                //remove the useless '"'                    //'"'
            prefix[2]='\0';
        else {
            token_ungetc(prefix[2],input);
        stop_1:
            token_ungetc(prefix[1],input);
        stop_0:
            token_ungetc(prefix[0],input);
            prefix[0]='\0';
            return NULL;
        }
    }
get_s_char:
    {//s-char-sequence
    //WARNING - UNSAFE! ////
        char* s=input->buffer[input->token].str;
        *s=token_getc(input);
        while(!token_feof(input) && *s!='"')                    //'"'
            switch(*s) {
                unsigned long long c;
            case '\n':
                goto no_terminator;
                char type;
            case '\\':
                switch(type = get_escape_sequence(input,&c)) {
                case 'i':
                    fprintf(stderr,"unknown escape sequence \\%c\n",(char)c);
                    exit(1);
                case '0':
                case 'x':
                    if(c>0xff) {
                        if (prefix[0]=='L'||prefix[0]=='\0') {
                            fprintf(stderr,
                            "warning: %s escape sequence out of range"
                            " \\%llo \n",type=='0'?"octal":"hex",c);
                        } else {
                            fprintf(stderr,"no support for wide chars yet");
                            exit(1);
                        }
                    }
                    break;
                case 's':
                    break;  //continue to getting the next char
                }
                *s=(unsigned char) c;   //it's safest to convert to that
                    //fallthrough
            default:
                s++;
                *s=token_getc(input);
                continue;
            }

        if (*s=='"') { //string input was a success                 //'"'
            *s='\0';
        } else if(!token_feof(input)) {
        no_terminator:
            fprintf(stderr,"error:missing terminating \" character\n");
            exit(1);
        } else {
            fprintf(stderr,"INTERNAL ERROR 101\n");
            exit(1);
        }
    }
    input->buffer[input->token].t=t_string;
    input->token^=1;   //switch buffer
    return &input->buffer[input->token^1]; //return the current buffer
}
static inline struct token* get_char(struct context* input) {
    char* prefix=input->buffer[input->token].string_prefix;
    memset(prefix,'\0',4);
    prefix[0]=token_getc(input);
    if (prefix[0] == '\'')
        prefix[0] = '\0';
    else if (strchr("uUL",prefix[0])) {
        prefix[1]=token_getc(input);
        if (prefix[1] == '\'')  //if [uUL]"
            prefix[1] = '\0';
        else {  //not a string; input of the form [uUL][^"]
            token_ungetc(prefix[1],input);
            token_ungetc(prefix[0],input);
            prefix[0]='\0';
            return NULL;
        }
    } else {    //in this case , the first char is [^"uUL]
        token_ungetc(prefix[0],input);
        return NULL;
    }
        //get the char itself
    int c=token_getc(input);
    switch (c) {
        case '\'':
            fprintf(stderr,"error: empty character constant\n");
            exit(1);

            char type;
            unsigned long long sum;
        case '\\':
            type = get_escape_sequence(input,&sum);
            switch(type) {
                case 'i':
                    fprintf(stderr,"unknown escape sequence \\%c\n",
                                                                (char)sum);
                    exit(1);
                case '0':
                case 'x':
                    if (sum>0xff) {
                        if (prefix[0]=='L'||prefix[0]=='\0') {
                        //if the string's type is char or wchar_t (ASCII):
                            fprintf(stderr,
                                "warning: %s escape sequence out of range"
                                " \\%llo \n",type=='0'?"octal":"hex",sum);
                        } else
                            goto multibyte_char_warning;
                    }
                    c=sum;
                    break;
                case 's':
                    break;  //continue to getting the next char
            }
        default:
            break;
    }
    input->buffer[input->token].str[0]=(unsigned char) c;

    if (token_getc(input)!='\'') {
        fprintf(stderr,"missing closing '\nhint: ");
    multibyte_char_warning:
        fprintf(stderr,"no support for wide chars yet\n");
        exit(1);
    }
    input->buffer[input->token].t=t_char;
    input->token^=1;   //switch buffer
    return &input->buffer[input->token^1]; //return the current buffer
}
static inline struct token* get_keyword_or_identifier(struct context* input) {
    char* s=input->buffer[input->token].str;
    int c=token_getc(input);
    if ( !isalpha(c) && c!='_' ) {
            //not an identifier
        token_ungetc(c,input);
        return NULL;
    }
    while (isalnum(c) || c=='_') {
        *s++=c;
        c=token_getc(input);
    }
    token_ungetc(c,input);

    *s='\0';

    input->buffer[input->token].t=t_identifier;

    for (unsigned i=0; i<sizeof(keywords)/sizeof(*keywords); i++) {
        if (strcmp(input->buffer[input->token].str,keywords[i])==0) {
            input->buffer[input->token].t=t_keyword;
            break;
        }
    }

    input->token^=1;   //switch buffer
    return &input->buffer[input->token^1]; //return the current buffer
}
static inline struct token* get_number(struct context* input){
    int c=token_getc(input);
    if (!isdigit(c))    {  token_ungetc(c,input);   return NULL;  }
    unsigned i=0;
    bool is_float=false;
    bool got_e=false;
    bool got_p=false;

    input->buffer[input->token].str[1]='\0';

    while (isalnum(c) || c=='.' || c=='_') {
        if (i==input->buffer_size - 1) {
            fprintf(stderr,"the number starting with %s is too long."
                                    " exiting\n", input->buffer[input->token].str);
            exit(1);
        }
        if (c=='.') {
            if (!is_float)
                is_float=true;
            else {
                fprintf(stderr,"too many points in float constant\n");
                exit(1);
            }
        }
        if (c=='e'||c=='E')     got_e=true;
        if (c=='p'||c=='P')     is_float=got_p=true;
        input->buffer[input->token].str[i++]=c;
        c=token_getc(input);
    }
    token_ungetc(c,input);
    {   char tmp=input->buffer[input->token].str[1];
        if (got_e && tmp != 'x' && tmp != 'X')
            //a non-hex number that has got `e` means it's a float
            is_float=true;
    }
    char* suffix;
    errno=0;
    if (is_float) {
        {   double tmp = strtod(input->buffer[input->token].str,&suffix);
            input->buffer[input->token].floating_number=tmp;
        }
        if (errno) {
            perror("bad float token");
            exit(1);
        }
        switch(suffix[0]) {
            //in this implementation , long double is equivalent to double
        case 'f': case 'F':
            input->buffer[input->token].floating_number=(float)
                                input->buffer[input->token].floating_number;
            //fallthrough
        case 'l': case 'L':
            if (suffix[1] != '\0')  goto bad_float_suffix;
                //multiple characters in suffix
        case '\0':
            break;
        default:
        bad_float_suffix:
            //if the postfix is neither of the above and not empty...
            fprintf(stderr,"invalid suffix \"%s\" on floating constant\n",
                                                                suffix);
            exit(1);
        }
        memcpy(input->buffer[input->token].number_postfix,suffix,2); //copy the suffix
        input->buffer[input->token].t=t_float;
    } else {
        {   unsigned long long tmp=strtoull(input->buffer[input->token].str,&suffix,0);
            input->buffer[input->token].number=tmp;
        }
        if (errno) {
            perror("bad int token");
            exit(1);
        }
        bool has_u=false;
        bool has_l=false;
        bool has_ll=false;
        while (*suffix) {
            switch (*suffix) {
                case 'u': case 'U':
                    if (has_u)      goto bad_int_suffix;
                    else has_u=true;
                    break;
                case 'l': case 'L':
                    if (has_ll || has_l) goto bad_int_suffix;
                        //a valid suffix may have at most one of {l,ll}
                    if (suffix[0] != suffix[1]) //if it's ll or LL
                        has_l=true;
                    else
                        { has_ll=true; suffix++; }
                    break;
            default:
            bad_int_suffix:
                fprintf(stderr,"invalid suffix \"%s\" on integer constant\n",
                                                                suffix);
                exit(1);
            }
            suffix++;
        }
        int n=0;
        if (has_u)
            input->buffer[input->token].number_postfix[n++]='u';
        else if (has_l)
            input->buffer[input->token].number_postfix[n++]='l';
        else if (has_ll) {
            input->buffer[input->token].number_postfix[n++]='l';
            input->buffer[input->token].number_postfix[n++]='l';
        }
        input->buffer[input->token].number_postfix[n]='\0';
            //the suffix is now either u, ul, ull, l, ll or empty;
        input->buffer[input->token].t=t_int;
    }
    input->token^=1;   //switch buffer
    return &input->buffer[input->token^1]; //return the current buffer
}
struct token* next(struct context* input){
    //returns the next token if such exists. NULL if at EOF.
    struct token* result=NULL;
    struct token* (*get_token[])(struct context*) =
        { get_string, get_char, get_keyword_or_identifier, get_punctuator,
        get_number};
        //an array of all the "get_token" functions.
        //all of them return either a valid token or NULL if they fail.
	int c;
    do c=token_getc(input); while (isspace(c));
	token_ungetc(c,input);

    unsigned i=0;
    while (result==NULL && i<sizeof(get_token)/sizeof(*get_token)
            && !token_feof(input)) {
        //try each function in order
        result = get_token[i++](input);
    }

    //until you find one that works
	if(token_feof(input)) {
        input->buffer[input->token].t=t_EOF;
        return 	&input->buffer[input->token^1]; //because in this case token doesn't flip
    } else if (result==NULL) {
        //if couldn't find any token, issue lexer error
        char next_char=token_getc(input);
        fprintf(stderr,"can't parse token starting with %c"
        					" ('\\x%x')\n", next_char,next_char);
        exit(1);
    // } else if (result->t==t_string && buffer[input->token].t==t_string) {
    //     // string concat
    //     if (strlen(result->str)+strlen(buffer[input->token].str)>=buffer_size)
    //         fprintf(stderr,"warning: string is too long\n");
    //     strncat(buffer[input->token].str, result->str,
    //                                 buffer_size-1-strlen(buffer[input->token].str));
    //     //
    } else {
        printf ("next ");
        print_token(&input->buffer[input->token]);
        return &input->buffer[input->token];  //return the previous token
    }
}
struct token* peek(struct context* input) {
    printf ("peek ");
    print_token(&input->buffer[input->token^1]);
    return &input->buffer[input->token^1];  //return the previous token
}
void print_token(struct token* t) {
    if (t==NULL)
        printf("NULL");
    switch (t->t) {
        case t_punctuator:
        	printf("punct: ");
            puts(t->str);     break;
        case t_keyword:
        	printf("kw: ");
            puts(t->str);     break;
        case t_identifier:
        	printf("id: ");
            puts(t->str);     break;
        case t_string:
            if (t->string_prefix[0]!='\0')
                printf("prefix: %s\t string:", t->string_prefix);
            printf("\"%s\"\n",t->str);
            break;
        case t_char:
            if (t->string_prefix[0]!='\0')
                printf("prefix: %s\t string:", t->string_prefix);
            printf("'%s'\n",t->str);
            break;
        case t_unknown:
            printf("UNKNOWN");
            break;
        case t_EOF:
            break;
        case t_int:
            printf("(int)%llu\n",t->number);
            break;
        case t_float:
            printf("(float)%lf\n",t->floating_number);
            break;
        case t_bad_token:
            fprintf(stderr,"INTERNAL ERROR 102");
            exit(1);
        default:
            fprintf(stderr,"INTERNAL ERROR 103");
            exit(1);
    }
}

void expect_token(const char* expected, struct context* input){
	//eat up the next token. crash if no match.
	char* delim = next(input)->str;
	if (strcmp(delim,expected)!=0){
		fprintf(stderr,"expected `%s` before `%s`\n",expected,delim);
		exit(1);
	}
}
