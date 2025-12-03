#include<iostream>
using namespace std;
class Program{
    private:
    int value;
public:
    void greet() {
        cout << "Hello from Program!" << endl;
    }
    void setValue(int v) {
        value = v;
    }
    int getValue() {
        return value;
    }
    // Constructor
    Program(int val = 0) : value(val) {}
    // Destructor
    ~Program() {
        cout << "Program with value " << value << " is being destroyed." << endl;
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
static ll parse_pow();
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
    skip_spaces();
    if (exp < 0)
    {
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
static ll parse_pow()
{
    ll left = parse_primary();
    skip_spaces();
    if (*input_ptr == '^')
    {
        ++input_ptr; 
        // compute right side first
        ll right = parse_pow();
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
    return parse_addsub(); // will replaced with low level parsing later
}

// Evaluate a single expression string
static ll evaluate_expression(const char* expr)
{
    input_ptr = expr;
    ll result = parse_expression();
    skip_spaces();
    return result;
}
// parse and : pow [& pow]
static ll parse_and(){

    skip_spaces();
    ll val = parse_pow();
    skip_spaces();
    while(*input_ptr++ == '&'){
        skip_spaces();
        ll val2 = parse_pow();
        val = (val * val) - (val2 * val2);
    }
    return val;
}

// parse muldiv : and [(*|/) and]
static ll parse_muldiv(){
    skip_spaces();
    ll val = parse_and();
    skip_spaces();
    while(*input_ptr=='*'|| *input_ptr == '/'){
        skip_spaces();
        ll val2 = parse_and();
        if(*input_ptr++ == '*'){
            val = val * val2;
        }
        else
            {
                input_ptr++;
                val = val / val2;
            }
    }
    return val;
}


static ll parse_addsub(){
    // skip spacces
    skip_spaces();
    // left side 
    ll val = parse_muldiv();
    while(*input_ptr=='+'|| *input_ptr == '-'){
        // skip spaces
        skip_spaces();
        // right side 
        ll val2 = parse_muldiv();
        // apply operation + or -
        if(*input_ptr++ == '+'){
            val = val + val2;
        }
        else
           {input_ptr++;
           val = val - val2;
           }
    }
    return val;
}