#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////

int GetPositiveInteger(char* s)
{
    if(*s==0) return -1;
    int v=0;
    while(*s)
    {
        if(*s<'0'||*s>'9') return -1;
        v=v*10+*s-'0';
        s++;
    }
    return v;
}

void GetAscii(unsigned char ch, char* ascii)
{
    if(ch<32 || ch>126) {ascii[0]=ascii[1]='0'; return;}
    ascii[1]=ch%16; ch=ch/16; ascii[0]=ch%16;
    if(ascii[0]<10) ascii[0]+='0'; else ascii[0]=ascii[0]-10+'A';
    if(ascii[1]<10) ascii[1]+='0'; else ascii[1]=ascii[1]-10+'A';
}

//////////////////////////////////////////////////////////////////////////////////////////

void Encode(int n, char* s[])
{
    if(n<5)
    {
        printf("Too few arguments. The command must include: filename, assignment ID, number of students, and student IDS.\n"
                   "Example: assign_encode ./myassign.txt 1 3 20120001 20120071 20120301\n");
        fflush(stdout); return;
    }

    char* input_file_name=s[1];

    if(*input_file_name==0) {printf("Incorrect file name (%s)\n", s[1]); fflush(stdout); return;}

    int assign_id=GetPositiveInteger(s[2]);
    int num_students=GetPositiveInteger(s[3]);

    if(assign_id<=0 || assign_id>9) {printf("Incorrect assignment ID (%s)\n", s[2]); fflush(stdout); return;}

    if(num_students<=0 || num_students>=10) {printf("Incorrect number of students (%s)\n", s[3]); fflush(stdout); return;}
    if(num_students+4!=n) {printf("The number of entered IDs does not equal to the entered number of students.\n"); fflush(stdout); return;}

    char output_file_name[1000];
    int i, cur=0;
    sprintf(output_file_name+cur, "./assign_%d", assign_id); cur+=10;
    for(i=0;i<num_students;i++)
    {
        int student_id=GetPositiveInteger(s[4+i]);
        if(student_id<20000000||student_id>21000000) {printf("Incorrect student ID (%s).\n", s[4+i]); fflush(stdout); return;}
        sprintf(output_file_name+cur, "_%d", student_id); cur+=9;
    }

    sprintf(output_file_name+cur, ".bin");

    char input_file_data[50*1024];
    char output_file_data[2*50*1024];

    FILE* input_file=fopen(input_file_name, "rb");
    if(input_file==0) {printf("Cannot open the input file (%s).\n", input_file_name); fflush(stdout); return;}

    int ind=0, cnt=fread(input_file_data, 1, 50*1024, input_file);
    if(cnt<=0) {printf("Cannot read the input file (%s).\n", input_file_name); fflush(stdout); return;}
    if(cnt>=15*1010) {printf("The input file size must be less than 15,000 characters.\n"); fflush(stdout); return;}

    int last_char=-1;

    for(i=0;i<cnt;i++)
    {
        int cur_char=input_file_data[i];
        if(cur_char+last_char==23) {last_char=-1; continue;}

        if(cur_char!=10 && cur_char!=13 && !(cur_char>=32 && cur_char<=126))
        {
            if(cur_char==9) {printf("TAB characters are not allowed. Convert each TAB to 4 spaces then try again.\n"); fflush(stdout); return;}
            else {printf("The character with ascii code (%d) is not allowed.\n", cur_char); fflush(stdout); return;}
        }

        GetAscii(cur_char, output_file_data+ind);
        ind+=2;
        last_char=cur_char;
    }
    output_file_data[ind]=0;

    fclose(input_file);

    FILE* output_file=fopen(output_file_name, "wt");
    if(output_file==0) {printf("Cannot open the output file (%s).\n", output_file_name); fflush(stdout); return;}

    fwrite(output_file_data, 1, ind, output_file);

    fclose(output_file);
}

//////////////////////////////////////////////////////////////////////////////////////////
class tinyProgram{
    private:
    int value;
public:
    void greet() {
        cout << "Hello from tinyProgram!" << endl;
    }
    void setValue(int v) {
        value = v;
    }
    int getValue() {
        return value;
    }
    // Constructor
    tinyProgram(int val = 0) : value(val) {}
    // Destructor
    ~tinyProgram() {
        cout << "tinyProgram with value " << value << " is being destroyed." << endl;
    }

    // overload the - operator
    tinyProgram operator-(const tinyProgram& other) {
        return tinyProgram(this->value - other.value);
    }
    // overload the * operator
    tinyProgram operator*(const tinyProgram& other) {
        return tinyProgram(this->value * other.value);
    }

    // overload the ^ operator
    tinyProgram operator^(const tinyProgram& other) const {
        int base = value;
        int exp  = other.value;
        if (exp < 0) {
            return tinyProgram(0);
        } 
        int result = 1;
        for (int i = 0; i < exp; ++i) {
            result *= base; 
        }
        return tinyProgram(result);
    }

    // overload the & operator
    tinyProgram operator&(const tinyProgram& other) {
        return tinyProgram((this->value * value) - (other.value * other.value));
    }

    // overload the / operator
    tinyProgram operator/(const tinyProgram& other) {
        if(other.value == 0) {
            cout << "Error: Division by zero!" << endl;
            return tinyProgram(0); 
        }
        return tinyProgram(this->value / other.value);
    }


    // overload the << operator for easy output
    friend ostream& operator<<(ostream& os, const tinyProgram& tp) {
        os << "tinyProgram(value=" << tp.value << ")";
        return os;
    }

};
//////////////////////////////////////////////////////////////////////////////////////////
// Global variable to hold the input pointer
static const char* input_ptr;
// Type alias for long long
using ll = long long;
///////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations
static void skip_spaces();
static bool parse_number(ll &out_value);
static ll parse_expression();
static ll parse_primary();
static ll parse_powlevel();
///////////////////////////////////////////////////////////////////////////////////////////////
// Function to skip spaces, tabs, and newlines
static void skip_spaces()
{
    while (*input_ptr == ' ' || *input_ptr == '\t' || *input_ptr == '\n' || *input_ptr == '\r'){
            ++input_ptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////

// Function to parse a number from the input
// returns true if a number was read, false otherwise
static bool parse_number(ll &out_value)
{
    skip_spaces();
    const char* start = input_ptr;
    //  here accept digits only
    if (*input_ptr < '0' || *input_ptr > '9') {
        return false;
    }
    ll temp = 0;
    while (*input_ptr >= '0' && *input_ptr <= '9')
    {
        temp = temp * 10 + (int)(*input_ptr - '0');
        ++input_ptr;
    }
    out_value = temp;
    (void)start;
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////

// parse primary: number, parenthesis, or unary +/- primary
static ll parse_primary()
{
    skip_spaces();
    if (*input_ptr == '+' || *input_ptr == '-')
    {
        // + or -
        char sign = *input_ptr;
        ++input_ptr;
        ll val = parse_primary();
        return (sign == '-') ? -val : val;
    }
    else if (*input_ptr == '(')
    {
        ++input_ptr; 
        ll val = parse_expression();
        skip_spaces();
        if (*input_ptr == ')') {
            ++input_ptr;
        } 
        return val;
    }
    else
    {
        long long num = 0;
        if (parse_number(num)) return num;

        // if not a number or parenthesis, treat as zero to avoid infinite loop
        if (*input_ptr != 0) {
            ++input_ptr;
        }
        return 0;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////

// Function to compute integer power
static ll int_pow(ll base, ll exp)
{
    if (exp < 0) {
        return 0;
    } 
    ll result = 1;
    while (exp > 0)
    {
        if (exp & 1LL) {
            result = result * base;
        }
        base = base * base;
        exp >>= 1LL;
    }
    return result;
}

// Function to compute integer power
static ll parse_powlevel()
{
    ll left = parse_primary();
    skip_spaces();
    if (*input_ptr == '^')
    {
        ++input_ptr; 
        // compute right side first
        ll right = parse_powlevel();
        // ^ as exponentiation: left ^ right = left^(right)
        ll val = int_pow(left, right);
        return val;
    }
    return left;
}
///////////////////////////////////////////////////////////////////////////////////////////////

// top-level expression
static ll parse_expression()
{
    return parse_powlevel(); // will replaced with low level parsing later
}

// Evaluate a single expression string
static ll evaluate_expression(const char* expr)
{
    input_ptr = expr;
    ll result = parse_expression();
    skip_spaces();
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[])
{
        // Encode(argc, argv);
        // tinyProgram a(15);
        // tinyProgram b(5);


        // tinyProgram d = a - b;
        // cout << "Result of a - b: " << d << endl;

        // tinyProgram e = a * b;
        // cout << "Result of a * b: " << e << endl;

        // tinyProgram f = a ^ b;
        // cout << "Result of a ^ b: " << f << endl;

        // tinyProgram g = a & b;
        // cout << "Result of a & b: " << g << endl;

        // tinyProgram h = a / b;
        // cout << "Result of a / b: " << h << endl;

        // Test Power Operator
        const char* expr = "3 ^ 2 "; // should be (3^2) = 3^2 = 9
        ll result = evaluate_expression(expr);
        cout << "Result of expression '" << expr << "' is: " << result << endl;



    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
