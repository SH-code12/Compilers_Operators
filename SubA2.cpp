/*
section : 4CS - S3,4
Names and IDs : 
 Malak Hisham 20221162
 Shahd Mohamed 20220533
 Malak Mohamed 20221160
*/
/*
{ Tiny test program }

int i;
int j;
int k;
real x;
real y;
real z;
bool b;
bool bi;
bool bii;

{ Initialization}
i := 10;
j := 20;
x := 2.5;
y := 4.0;

{Integer Arithmetic}
i := i + j;
j := j - 5;
k := i * j;
i := k / 10;
j := i ^ 2;

{ print integer calculation results }
write i;
write j;
write k;

{ real Arithmetic}
x := x + y;
y := y - 0.5;
z := x * y;
x := z / 2.0;
y := x ^ 2.0;

{ print real calculation results }
write x;
write y;
write z;

{ mixed Arithmetic }
z := i + x;
write z; 

z := j * y;
write z; 

{ Boolean Logic }
b := i < j;
bi := x < y;
bii := i = k;

bii := b & bi;

{ print boolean results }
write b;
write bi;
write bii;

{ condition}
if bii then
    i := i + 1;
    x := x * 2.0;
    
    
    write i;
    write x;
end;

{ Final Results }
write i;
write x;
write bii;
*/

// Added INT_TYPE / REAL_TYPE / BOOL_TYPE so the lexer recognizes type keywords
enum TokenType { INT_TYPE, REAL_TYPE, BOOL_TYPE };

// Used for debugging only: names for token kinds
// Debug names for the new type tokens so they show up clearly in debug output.
const char* TokenTypeStr[] = { "IntType", "RealType", "BoolType" };

// Reserved words updated to include type keywords
// Treat "int", "real", "bool" as reserved words (type keywords), not identifiers.
const Token reserved_words[] =
{
    Token(INT_TYPE, "int"),
    Token(REAL_TYPE, "real"),
    Token(BOOL_TYPE, "bool")
};

// - Modified numeric lexing to accept a single '.' so we can produce real literals
// - Reserved words now include type keywords so they get mapped to INT_TYPE/REAL_TYPE/BOOL_TYPE
void GetNextToken(CompilerInfo* pci, Token* ptoken)
{   // when facing a digit (could be int or real num)
    else if (IsDigit(s[0]))
    {
        int j = 1;
    bool has_dot = false;
    // modified to accept real numbers 
    while (IsDigit(s[j]) || (s[j] == '.' && !has_dot))
    {
        if (s[j] == '.') {
        has_dot = true;
        }
    j++;
    }

    ptoken->type = NUM;
    Copy(ptoken->str, s, j);
    }

}


// Node kinds: added DECL_NODE to represent declarations
enum NodeKind { DECL_NODE };
const char* NodeKindStr[] = { "Decl" }; // Added Decl for Assignment 2

// Expression data types: added REAL to represent floating-point values
enum ExprDataType { VOID, INTEGER, REAL, BOOLEAN }; // Added REAL for Assignment 2

// Used for debugging only /////////////////////////////////////////////////////////
const char* ExprDataTypeStr[] = { "Void", "Integer", "Real" ,"Boolean" };

// newexpr -> ( mathexpr ) | number | identifier
// - when number: detect integer vs real literal and store accordingly
TreeNode* NewExpr(CompilerInfo* pci, ParseInfo* ppi)
{
    
    // Compare the next token with the First() of possible statements
    if (ppi->next_token.type == NUM)
    {
        TreeNode* tree = new TreeNode;
    tree->node_kind = NUM_NODE;
    char* num_str = ppi->next_token.str;
    // determine if integer or real: if token contains '.', treat as REAL
    bool is_real = false;
    for (int i = 0; num_str[i] != 0; i++)
    {
        if (num_str[i] == '.')
    {
        is_real = true;
    break;
}
}
    if (is_real)
    {
        // Distinguish integer vs real numeric tokens at parse time:
       // - if token contains '.', treat it as REAL and store as double (realnum).
    tree->expr_data_type = REAL;
    tree->realnum = atof(num_str);
}
    else
    {
        // - otherwise parse and store as INTEGER (num).
    tree->num = 0; while (*num_str) tree->num = tree->num * 10 + ((*num_str++) - '0');
    tree->expr_data_type = INTEGER;
}
    // End ModIFIED
    tree->line_num = pci->in_file.cur_line_num;
    Match(pci, ppi, ppi->next_token.type);

    return tree;
}
}

// declstmt -> (int|real|bool) identifier
// DeclStmt: parse declarations like `int x;`, `real y;`, `bool b;`
// Parse declarations and create DECL_NODE with expr_data_type storing declared type.
// 
TreeNode* DeclStmt(CompilerInfo* pci, ParseInfo* ppi)
{
    

    TokenType t = ppi->next_token.type; // INT_TYPE, REAL_TYPE, or BOOL_TYPE
    Match(pci, ppi, t);

    if (ppi->next_token.type != ID) throw 0;

    TreeNode* tree = new TreeNode;
    tree->node_kind = DECL_NODE;
    AllocateAndCopy(&tree->id, ppi->next_token.str);
    // store declared type in expr_data_type (reuse enum)
    if (t == INT_TYPE) tree->expr_data_type = INTEGER;
    else if (t == REAL_TYPE) tree->expr_data_type = REAL;
    else tree->expr_data_type = BOOLEAN;

    tree->line_num = pci->in_file.cur_line_num;
    Match(pci, ppi, ID);

    
    return tree;
}

// stmtseq -> stmt { ; stmt }
// StmtSeq: accept declarations first then statements
// - This enforces a simple "declare before use" discipline.
TreeNode* StmtSeq(CompilerInfo* pci, ParseInfo* ppi)
{
    

    TreeNode* first_tree = nullptr, * last_tree = nullptr;
    // Start MODIFIED
    // enforce declaraion rules first 
    while (ppi->next_token.type == INT_TYPE || ppi->next_token.type == REAL_TYPE || ppi->next_token.type == BOOL_TYPE) {
        TreeNode* node = DeclStmt(pci, ppi);

    if (first_tree == nullptr)
    first_tree = node;
    else
    last_tree->sibling = node;
    last_tree = node;
    Match(pci, ppi, SEMI_COLON);
}
    // enforce that only accept the non-declarative statments 
    while (ppi->next_token.type == IF || ppi->next_token.type == REPEAT ||
        ppi->next_token.type == ID || ppi->next_token.type == READ || ppi->next_token.type == WRITE)
    {
        TreeNode* next_tree = Stmt(pci, ppi);
    if (first_tree == nullptr) {
        first_tree = next_tree;
}
    else {
        last_tree->sibling = next_tree;
}
    last_tree = next_tree;
    Match(pci, ppi, SEMI_COLON);
}
    
    return first_tree;
}

// Print AST with correct numeric formatting for reals
void PrintTree(TreeNode* node, int sh = 0)
{
    // handle specific node kinds real number printing
    else if (node->node_kind == NUM_NODE) {
        if (node->expr_data_type == REAL)
    printf("[%g]", node->realnum); // Print real number
    else
    printf("[%d]", node->num);     // Print integer
}
}

struct VariableInfo
{
    int var_type;
};
// 0=int,1=real,2=bool // added for extended types

// Insert new variable with declared type; prevent duplicate declarations.
// Store declared type (0=int,1=real,2=bool) in symbol table entry.
void Insert(const char* name, int line_num, int var_type) // added var_type for extended types : Assignment 2
{
    // If variable already declared, print duplicate declaration error and return
    if (Find(name)) { printf("ERROR Duplicate declaration of %s\n", name); return; }
    LineLocation* lineloc = new LineLocation;
    lineloc->line_num = line_num;
    lineloc->next = 0;
    int h = Hash(name);
    VariableInfo* vi = new VariableInfo;
    // initialize VariableInfo fields Assignment 2
    vi->head_line = vi->tail_line = lineloc;
    vi->next_var = var_info[h];
    vi->memloc = num_vars++;
    vi->var_type = var_type; // store type code
    AllocateAndCopy(&vi->name, name);
    var_info[h] = vi;
}

//  arithmetic : numeric (int/real), resulting type REAL if any operand REAL
void Analyze(TreeNode* node, SymbolTable* symbol_table)
{
    int i;
    // Handle declarations first: add var to symbol table with declared type
    if (node->node_kind == DECL_NODE)
    {
        // store var_type as 0=int,1=real,2=bool
    int var_type = (node->expr_data_type == INTEGER ? 0 : (node->expr_data_type == REAL ? 1 : 2));
    symbol_table->Insert(node->id, node->line_num, var_type);
}

    // other unmodified code skipped 

    // Type checks for operators: arithmetic only for int/real; no arithmetic on bool
    if (node->node_kind == OPER_NODE)
    {
        ExprDataType left = node->child[0]->expr_data_type;
    ExprDataType right = node->child[1]->expr_data_type;
    if (node->oper == ANDA)
    {
        if (left != BOOLEAN || right != BOOLEAN) printf("ERROR & applied to non-boolean\n");
    node->expr_data_type = BOOLEAN;
}
    else if (node->oper == EQUAL || node->oper == LESS_THAN)
    {
        // equality/less-than allowed on numeric types only
    if ((left != INTEGER && left != REAL) || (right != INTEGER && right != REAL)) printf("ERROR comparison on non-numeric\n");
    node->expr_data_type = BOOLEAN;
}
    else
    {
        // arithmetic operators
    if (left == BOOLEAN || right == BOOLEAN) printf("ERROR arithmetic on boolean\n");
    if (left == REAL || right == REAL) node->expr_data_type = REAL; else node->expr_data_type = INTEGER;
}
}
   
    if (node->node_kind == WRITE_NODE && (node->child[0]->expr_data_type != INTEGER && node->child[0]->expr_data_type != REAL && node->child[0]->expr_data_type != BOOLEAN))
    printf("ERROR Write works only for numeric types\n");
    if (node->node_kind == ASSIGN_NODE)
    {
        VariableInfo* vi = symbol_table->Find(node->id);
    if (!vi) printf("ERROR Assign to undeclared var %s\n", node->id);
}
    if (node->sibling) Analyze(node->sibling, symbol_table);
}

// Runtime Value type holds typed values (INTEGER/REAL/BOOLEAN).
struct Value
{
    ExprDataType type;
    int ival;
    double rval;
    bool bval;
};

// - performs numeric promotion (int -> double) for mixed arithmetic/comparisons,
// - computes in double and returns REAL if any operand REAL,
// - preserves INTEGER when both operands are integer (truncating cast when needed),
// - handles boolean AND and boolean equality distinctly.
Value Evaluate(TreeNode* node, SymbolTable* symbol_table, Value* variables)
{
    if (node->node_kind == NUM_NODE)
    {
        // handle real number evaluation
    Value v;
    if (node->expr_data_type == REAL) { v.type = REAL; v.rval = node->realnum; }
    else { v.type = INTEGER; v.ival = node->num; }
    return v;
}
    if (node->node_kind == ID_NODE)
    {
        VariableInfo* vi = symbol_table->Find(node->id);
    if (!vi) throw 0;
    return variables[vi->memloc];
}
    // evaluate children
    Value a = Evaluate(node->child[0], symbol_table, variables);
    Value b = Evaluate(node->child[1], symbol_table, variables);
    // equality '='
    if (node->oper == EQUAL)
    {
        // handle equality for numeric and boolean types
    Value r; r.type = BOOLEAN; r.bval = false;
    if ((a.type == INTEGER || a.type == REAL) && (b.type == INTEGER || b.type == REAL))
    {
        double av = (a.type == REAL ? a.rval : (double)a.ival);
    double bv = (b.type == REAL ? b.rval : (double)b.ival);
    r.bval = (fabs(av - bv) < 1e-9);
    return r;
}
    if (a.type == BOOLEAN && b.type == BOOLEAN)
    {
        r.bval = (a.bval == b.bval);
    return r;
}
    throw 0;
}
    // less-than '<'
    if (node->oper == LESS_THAN)
    {
        // handle less-than for numeric types only
    Value r; r.type = BOOLEAN;
    double av = (a.type == REAL ? a.rval : (double)a.ival);
    double bv = (b.type == REAL ? b.rval : (double)b.ival);
    r.bval = (av < bv);
    return r;
}
    // arithmetic: + - * / ^
    if (node->oper == PLUS || node->oper == MINUS || node->oper == TIMES || node->oper == DIVIDE || node->oper == POWER)
    {
        double av = (a.type == REAL ? a.rval : (double)a.ival);
    double bv = (b.type == REAL ? b.rval : (double)b.ival);
    double rv = 0.0;
   // skippeed code for operators evaluation
    Value r;
    if (a.type == REAL || b.type == REAL) { r.type = REAL; r.rval = rv; }
    else { r.type = INTEGER; r.ival = (int)rv; }
    return r;
}
    // boolean AND '&'
    if (node->oper == ANDA)
    {
        Value r; r.type = BOOLEAN;
    r.bval = a.bval && b.bval;
    return r;
}
    // unexpected operator/type combination
    throw 0;
}

// RunProgram (interpreter): statements execution, read/write/assign semantics
void RunProgram(TreeNode* node, SymbolTable* symbol_table, Value* variables)
{
    if (node->node_kind == IF_NODE)
    {
        Value cond = Evaluate(node->child[0], symbol_table, variables);
    // ensure condition is boolean
    if (cond.type != BOOLEAN) printf("ERROR If condition not boolean\n");
    if (cond.bval) RunProgram(node->child[1], symbol_table, variables);
    else if (node->child[2]) RunProgram(node->child[2], symbol_table, variables);
}
    if (node->node_kind == ASSIGN_NODE)
    {
        // evaluate right-hand side
    Value v = Evaluate(node->child[0], symbol_table, variables);
    VariableInfo* vi = symbol_table->Find(node->id);
    if (!vi) { printf("ERROR Assign to undeclared %s\n", node->id); }
    else
    {
        // enforce no cross-type assign except int->real
    if (vi->var_type == 2)
    {
        if (v.type != BOOLEAN) printf("ERROR Assigning non-boolean to boolean %s\n", node->id);
    else variables[vi->memloc] = v;
}
    else if (vi->var_type == 0)
    {
        // integer variable: RHS must be integer 
    if (v.type != INTEGER) printf("ERROR Assigning non-integer to int %s\n", node->id);
    else variables[vi->memloc] = v;
}
    else if (vi->var_type == 1)
    {
        if (v.type == BOOLEAN) printf("ERROR Assigning boolean to real %s\n", node->id);
    else
    {
    // real variable: accept integer (convert) or real; boolean forbidden
    // convert int to real if needed
    if (v.type == INTEGER) { Value vv; vv.type = REAL; vv.rval = v.ival; variables[vi->memloc] = vv; }
    else variables[vi->memloc] = v;
}
}
}
}
    if (node->node_kind == READ_NODE)
    {
        VariableInfo* vi = symbol_table->Find(node->id);
    if (!vi) { printf("ERROR Read undeclared %s\n", node->id); }
    else
    {
        // Read uses the declared format; boolean read treats nonzero as true.
    if (vi->var_type == 2) { int t; scanf("%d", &t); variables[vi->memloc].type = BOOLEAN; variables[vi->memloc].bval = (t != 0); }
    else if (vi->var_type == 0) { int t; scanf("%d", &t); variables[vi->memloc].type = INTEGER; variables[vi->memloc].ival = t; }
    else { double t; scanf("%lf", &t); variables[vi->memloc].type = REAL; variables[vi->memloc].rval = t; }
}
}
    if (node->node_kind == WRITE_NODE)
    {
        Value v = Evaluate(node->child[0], symbol_table, variables);
    if (v.type == INTEGER) printf("Val: %d\n", v.ival);
    else if (v.type == REAL) printf("Val: %g\n", v.rval);
    else printf("Val: %d\n", v.bval);
}
    if (node->node_kind == REPEAT_NODE)
    {
        do
    {
        RunProgram(node->child[0], symbol_table, variables);
} while (!Evaluate(node->child[1], symbol_table, variables).bval);
}
    if (node->sibling) RunProgram(node->sibling, symbol_table, variables);
}