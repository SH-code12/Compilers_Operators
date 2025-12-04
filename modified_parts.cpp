#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;
/*
section : 4CS - S3,4
Names and IDs : 
 Malak Hisham 20221162
 Shahd Mohamed 20220533
 Malak Mohamed 20221160
*/
/*
{ Tiny test program for & operator }

a := 3 & 2;     { Expected 5 }
write a;

b := 2 & 6 - 4;     { Expected -36 }
write b;

c := 2 & 1 - 3;     { Expected 0  }
write c;

d := 2 - 1 & 4;    { Expected 17 }
write d;

e := 2 & 3 * 2;    { Expected -10 }
write e;

f := 3 & 2 ^ 2;    { Expected -7 }
write f;

g := (0-2) & 7 & (0-2);  { Expected 2021 }
write g;

h := 8 & 2 * 4 - 8; { Expected 232 }
write h;

i := 5 + 7 & (0-2);   { Expected 50 }
write i;

j := 3 & 2 & 1;    { Expected 24 }
write j;

k := 3 & 2 - 1 & 9 & 7 + 9 & 4;   { Expect -6281 }
write k;          

l := 8 * 0 + 3 & 5 * 7 & 9 & 10;   { Expect -14784 }
write l;        

m := 8 & 4 - 4 & 2 / 5 & 7 ;  { Expect 48 }
write m;          

n := (8 & 4 - 4 & 2) / 5 & 3 ; { Expect 2 }
write n;           

o := (4 + 3 ) & (4 / 2); { Expect 45 }
write o;           

p := ( 4 * 5 ) & (( 5 + 2 ) * 0);  { Expect 400 }
write p;           

q := 3 + 2 * 7 & 4 ^ 2 - 1; { Expect -412 }
write q         
*/

//Adding new ANDA token which is & for tokens
enum TokenType
{
                IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
                ASSIGN, EQUAL, LESS_THAN,
                PLUS, MINUS, TIMES, DIVIDE, POWER,
                SEMI_COLON,
                LEFT_PAREN, RIGHT_PAREN,
                LEFT_BRACE, RIGHT_BRACE,
                ID, NUM,
                ENDFILE, ERROR,
                ANDA // ANDA token for &
  };

// adding Anda & to tokens as string
const char* TokenTypeStr[]=
    {
    "If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
    "Assign", "Equal", "LessThan",
    "Plus", "Minus", "Times", "Divide", "Power",
    "SemiColon",
    "LeftParen", "RightParen",
    "LeftBrace", "RightBrace",
    "ID", "Num",
    "EndFile", "Error",
    "Anda" // Anda token string 
    
    };



// adding symbol equivalent to symbols 
const Token symbolic_tokens[]=
{
    Token(ASSIGN, ":="),
    Token(EQUAL, "="),
    Token(LESS_THAN, "<"),
    Token(PLUS, "+"),
    Token(MINUS, "-"),
    Token(TIMES, "*"),
    Token(DIVIDE, "/"),
    Token(POWER, "^"),
    Token(SEMI_COLON, ";"),
    Token(LEFT_PAREN, "("),
    Token(RIGHT_PAREN, ")"),
    Token(LEFT_BRACE, "{"),
    Token(RIGHT_BRACE, "}"),
    Token(ANDA,"&")  // Anda equivalent symbol &
};

/*adding new parser layer called anda between term and factor to have higher prescedence than * / 
and less prescendence than ^ 
*/


// Parser //////////////////////////////////////////////////////////////////////////

// program -> stmtseq
// stmtseq -> stmt { ; stmt }
// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
// ifstmt -> if exp then stmtseq [ else stmtseq ] end
// repeatstmt -> repeat stmtseq until expr
// assignstmt -> identifier := expr
// readstmt -> read identifier
// writestmt -> write expr
// expr -> mathexpr [ (<|=) mathexpr ]
// mathexpr -> term { (+|-) term }    left associative

// term -> anda { (*|/) anda }    left associative   ( make term call anda instead of factor as there is new layer added)

// anda -> factor {(&) factor}   left associative       (new layer for & op and calling factor )

// factor -> newexpr { ^ newexpr }    right associative
// newexpr -> ( mathexpr ) | number | identifier




// adding non terminal symbol anda to handle & operator 
// anda -> factor {(&) factor}   left associative 
TreeNode* Anda (CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode *left = Factor(pci, ppi); // anda is calling factor (factor has higher prescedence as it is hanling ^ op)
    while(ppi->next_token.type ==ANDA )
    {
    TreeNode *node = new TreeNode;  // new node in parse tree
    node->oper = ppi->next_token.type;  // set operator of the node by & 
    node->node_kind = OPER_NODE;  // set king of node in operator node
    Match(pci, ppi, ppi->next_token.type); // match the & symbol

    TreeNode *right = Factor(pci, ppi); // get right child of the tree of the operator
    node->child[0] = left; // set node left child
    node->child[1] = right; // set node right child 

    left = node; // left associativity  
    }
    return left; // return the tree
}

// term -> anda { (*|/) anda }    left associative
TreeNode* Term(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Term");

    TreeNode* tree=Anda(pci, ppi); // making the term calling (Anda) instead of (Factor)

    while(ppi->next_token.type==TIMES || ppi->next_token.type==DIVIDE)
    {
    TreeNode* new_tree=new TreeNode;
    new_tree->node_kind=OPER_NODE;
    new_tree->oper=ppi->next_token.type;
    new_tree->line_num=pci->in_file.cur_line_num;

    new_tree->child[0]=tree;
    Match(pci, ppi, ppi->next_token.type);
    new_tree->child[1]=Anda(pci, ppi); // making the term calling (Anda) instead of (Factor)

    tree=new_tree;
    }
    pci->debug_file.Out("End Term");
    return tree;
}

int Evaluate(TreeNode* node, SymbolTable* symbol_table, int* variables)
{
    if(node->node_kind==NUM_NODE) return node->num;
    if(node->node_kind==ID_NODE) return variables[symbol_table->Find(node->id)->memloc];

    int a=Evaluate(node->child[0], symbol_table, variables);
    int b=Evaluate(node->child[1], symbol_table, variables);

    if(node->oper==EQUAL) return a==b;
    if(node->oper==LESS_THAN) return a<b;
    if(node->oper==PLUS) return a+b;
    if(node->oper==MINUS) return a-b;
    if(node->oper==TIMES) return a*b;
    if(node->oper==DIVIDE) return a/b;
    if(node->oper==POWER) return Power(a,b);
    if(node->oper==ANDA) return Power(a,2) - (b*b); // add evaluation of  expression (a & b) by (a^2 - b^2)
    throw 0;
    return 0;
}
