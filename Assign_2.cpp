#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
using namespace std;


bool Equals(const char* a, const char* b)
{
    return strcmp(a, b)==0;
}

bool StartsWith(const char* a, const char* b)
{
    int nb=strlen(b);
    return strncmp(a, b, nb)==0;
}

void Copy(char* a, const char* b, int n=0)
{
    if(n>0) {strncpy(a, b, n); a[n]=0;}
    else strcpy(a, b);
}

void AllocateAndCopy(char** a, const char* b)
{
    if(b==0) {*a=0; return;}
    int n=strlen(b);
    *a=new char[n+1];
    strcpy(*a, b);
}

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile
{
    FILE* file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH];
    int cur_ind, cur_line_size;

    InFile(const char* str) {file=0; if(str) file=fopen(str, "r"); cur_line_size=0; cur_ind=0; cur_line_num=0;}
    ~InFile(){if(file) fclose(file);}

    void SkipSpaces()
    {
        while(cur_ind<cur_line_size)
        {
            char ch=line_buf[cur_ind];
            if(ch!=' ' && ch!='\t' && ch!='\r' && ch!='\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char* str)
    {
        while(true)
        {
            SkipSpaces();
            while(cur_ind>=cur_line_size) {if(!GetNewLine()) return false; SkipSpaces();}

            if(StartsWith(&line_buf[cur_ind], str))
            {
                cur_ind+=strlen(str);
                return true;
            }
            cur_ind++;
        }
        return false;
    }

    bool GetNewLine()
    {
        cur_ind=0; line_buf[0]=0;
        if(!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size=strlen(line_buf);
        if(cur_line_size==0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char* GetNextTokenStr()
    {
        SkipSpaces();
        while(cur_ind>=cur_line_size) {if(!GetNewLine()) return 0; SkipSpaces();}
        return &line_buf[cur_ind];
    }

    void Advance(int num)
    {
        cur_ind+=num;
    }
};

struct OutFile
{
    FILE* file;
    OutFile(const char* str) {file=0; if(str) file=fopen(str, "w");}
    ~OutFile(){if(file) fclose(file);}

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s); fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
                : in_file(in_str), out_file(out_str), debug_file(debug_str)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType{
                IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
                ASSIGN, EQUAL, LESS_THAN,
                PLUS, MINUS, TIMES, DIVIDE, POWER,
                SEMI_COLON,
                LEFT_PAREN, RIGHT_PAREN,
                LEFT_BRACE, RIGHT_BRACE,
                ID, NUM,
                ENDFILE, ERROR,
                ANDA,
                INT_TYPE, REAL_TYPE,BOOL_TYPE // added for extended types : Assignment 2
              };

// Used for debugging only /////////////////////////////////////////////////////////
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
                "Anda",
                "IntType", "RealType", "BoolType" // added for extended types : Assignment 2
                
            };

struct Token
{
    TokenType type;
    char str[MAX_TOKEN_LEN+1];

    Token(){str[0]=0; type=ERROR;}
    Token(TokenType _type, const char* _str) {type=_type; Copy(str, _str);}
};

const Token reserved_words[]=
{
    Token(IF, "if"),
    Token(THEN, "then"),
    Token(ELSE, "else"),
    Token(END, "end"),
    Token(REPEAT, "repeat"),
    Token(UNTIL, "until"),
    Token(READ, "read"),
    Token(WRITE, "write"),
    Token(INT_TYPE, "int"),    // added for extended types : Assignment 2
    Token(REAL_TYPE, "real"),  // added for extended types : Assignment 2
    Token(BOOL_TYPE, "bool")   // added for extended types : Assignment 2
};
const int num_reserved_words=sizeof(reserved_words)/sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
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
    Token(ANDA,"&")
};
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch){return (ch>='0' && ch<='9');}
inline bool IsLetter(char ch){return ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'));}
inline bool IsLetterOrUnderscore(char ch){return (IsLetter(ch) || ch=='_');}

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type=ERROR;
    ptoken->str[0]=0;

    int i;
    char* s=pci->in_file.GetNextTokenStr();
    if(!s)
    {
        ptoken->type=ENDFILE;
        ptoken->str[0]=0;
        return;
    }

    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(StartsWith(s, symbolic_tokens[i].str))
            break;
    }

    if(i<num_symbolic_tokens)
    {
        if(symbolic_tokens[i].type==LEFT_BRACE)
        {
            pci->in_file.Advance(strlen(symbolic_tokens[i].str));
            if(!pci->in_file.SkipUpto(symbolic_tokens[i+1].str)) return;
            return GetNextToken(pci, ptoken);
        }
        ptoken->type=symbolic_tokens[i].type;
        Copy(ptoken->str, symbolic_tokens[i].str);
    }
    else if(IsDigit(s[0]))
    {
        int j=1;
        bool has_dot=false;
        // while(IsDigit(s[j])) j++;
        // modified to accept real numbers : Assignment 2
        while(IsDigit(s[j]) || (s[j]=='.' && !has_dot))
        {
            if(s[j]=='.') {
                has_dot=true;
            }
            j++;
        }

        ptoken->type=NUM;
        Copy(ptoken->str, s, j);
    }
    else if(IsLetterOrUnderscore(s[0]))
    {
        int j=1;
        while(IsLetterOrUnderscore(s[j])) j++;

        ptoken->type=ID;
        Copy(ptoken->str, s, j);

        for(i=0;i<num_reserved_words;i++)
        {
            if(Equals(ptoken->str, reserved_words[i].str))
            {
                ptoken->type=reserved_words[i].type;
                break;
            }
        }
    }

    int len=strlen(ptoken->str);
    if(len>0) pci->in_file.Advance(len);
}

////////////////////////////////////////////////////////////////////////////////////
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
// term -> anda { (*|/) anda }    left associative
// anda -> factor {(&) factor }
// factor -> newexpr { ^ newexpr }    right associative
// newexpr -> ( mathexpr ) | number | identifier

enum NodeKind{
                IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE,
                OPER_NODE, NUM_NODE, ID_NODE, DECL_NODE
             };

// Used for debugging only /////////////////////////////////////////////////////////
const char* NodeKindStr[]=
            {
                "If", "Repeat", "Assign", "Read", "Write",
                "Oper", "Num", "ID", "Decl"
            };

enum ExprDataType {VOID, INTEGER, REAL, BOOLEAN};

// Used for debugging only /////////////////////////////////////////////////////////
const char* ExprDataTypeStr[]=
            {
                "Void", "Integer", "Real" ,"Boolean"
            };

#define MAX_CHILDREN 3

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];
    TreeNode* sibling; // used for sibling statements only

    NodeKind node_kind;

    union{TokenType oper; int num; char* id; double realnum;}; // defined for expression/int/identifier only
    ExprDataType expr_data_type; // defined for expression/int/identifier only

    int line_num;

    TreeNode() {int i; for(i=0;i<MAX_CHILDREN;i++) child[i]=0; sibling=0; expr_data_type=VOID;}
};

struct ParseInfo
{
    Token next_token;
};

void Match(CompilerInfo* pci, ParseInfo* ppi, TokenType expected_token_type)
{
    pci->debug_file.Out("Start Match");

    if(ppi->next_token.type!=expected_token_type) throw 0;
    GetNextToken(pci, &ppi->next_token);

    fprintf(pci->debug_file.file, "[%d] %s (%s)\n", pci->in_file.cur_line_num, ppi->next_token.str, TokenTypeStr[ppi->next_token.type]); fflush(pci->debug_file.file);
}

TreeNode* MathExpr(CompilerInfo*, ParseInfo*);


// newexpr -> ( mathexpr ) | number | identifier
TreeNode* NewExpr(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start NewExpr");

    // Compare the next token with the First() of possible statements
    if(ppi->next_token.type==NUM)
    {
        TreeNode* tree=new TreeNode;
        tree->node_kind=NUM_NODE;
        char* num_str=ppi->next_token.str;
        bool is_real = false;
        for(int i=0; num_str[i]!=0; i++)
        {
            if(num_str[i]=='.')
            {
                is_real = true; 
                break;
            }
        }
        if(is_real)
        {
            tree->expr_data_type = REAL;
            tree->realnum = atof(num_str);
        }
        else
        {
            tree->num=0; while(*num_str) tree->num=tree->num*10+((*num_str++)-'0');
            tree->expr_data_type = INTEGER;
        }
        tree->line_num=pci->in_file.cur_line_num;
        Match(pci, ppi, ppi->next_token.type);

        pci->debug_file.Out("End NewExpr");
        return tree;
    }

    if(ppi->next_token.type==ID)
    {
        TreeNode* tree=new TreeNode;
        tree->node_kind=ID_NODE;
        AllocateAndCopy(&tree->id, ppi->next_token.str);
        tree->line_num=pci->in_file.cur_line_num;
        Match(pci, ppi, ppi->next_token.type);

        pci->debug_file.Out("End NewExpr");
        return tree;
    }

    if(ppi->next_token.type==LEFT_PAREN)
    {
        Match(pci, ppi, LEFT_PAREN);
        TreeNode* tree=MathExpr(pci, ppi);
        Match(pci, ppi, RIGHT_PAREN);

        pci->debug_file.Out("End NewExpr");
        return tree;
    }

    throw 0;
    return 0;
}

// factor -> newexpr { ^ newexpr }    right associative
TreeNode* Factor(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Factor");

    TreeNode* tree=NewExpr(pci, ppi);

    if(ppi->next_token.type==POWER)
    {
        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=OPER_NODE;
        new_tree->oper=ppi->next_token.type;
        new_tree->line_num=pci->in_file.cur_line_num;

        new_tree->child[0]=tree;
        Match(pci, ppi, ppi->next_token.type);
        new_tree->child[1]=Factor(pci, ppi);

        pci->debug_file.Out("End Factor");
        return new_tree;
    }
    pci->debug_file.Out("End Factor");
    return tree;
}

TreeNode* Anda (CompilerInfo* pci, ParseInfo* ppi)
{
    TreeNode *left = Factor(pci, ppi);
    while(ppi->next_token.type ==ANDA )
    {
        TreeNode *node = new TreeNode;
        node->oper = ppi->next_token.type;
        node->node_kind = OPER_NODE;
        Match(pci, ppi, ppi->next_token.type);

        TreeNode *right = Factor(pci, ppi);
        node->child[0] = left;
        node->child[1] = right;

        left = node;
    }
    return left;
}

// term -> factor { (*|/) factor }    left associative
TreeNode* Term(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Term");

    TreeNode* tree=Anda(pci, ppi);

    while(ppi->next_token.type==TIMES || ppi->next_token.type==DIVIDE)
    {
        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=OPER_NODE;
        new_tree->oper=ppi->next_token.type;
        new_tree->line_num=pci->in_file.cur_line_num;

        new_tree->child[0]=tree;
        Match(pci, ppi, ppi->next_token.type);
        new_tree->child[1]=Anda(pci, ppi);

        tree=new_tree;
    }
    pci->debug_file.Out("End Term");
    return tree;
}

// mathexpr -> term { (+|-) term }    left associative
TreeNode* MathExpr(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start MathExpr");

    TreeNode* tree=Term(pci, ppi);

    while(ppi->next_token.type==PLUS || ppi->next_token.type==MINUS)
    {
        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=OPER_NODE;
        new_tree->oper=ppi->next_token.type;
        new_tree->line_num=pci->in_file.cur_line_num;

        new_tree->child[0]=tree;
        Match(pci, ppi, ppi->next_token.type);
        new_tree->child[1]=Term(pci, ppi);

        tree=new_tree;
    }
    pci->debug_file.Out("End MathExpr");
    return tree;
}

// expr -> mathexpr [ (<|=) mathexpr ]
TreeNode* Expr(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Expr");

    TreeNode* tree=MathExpr(pci, ppi);

    if(ppi->next_token.type==EQUAL || ppi->next_token.type==LESS_THAN)
    {
        TreeNode* new_tree=new TreeNode;
        new_tree->node_kind=OPER_NODE;
        new_tree->oper=ppi->next_token.type;
        new_tree->line_num=pci->in_file.cur_line_num;

        new_tree->child[0]=tree;
        Match(pci, ppi, ppi->next_token.type);
        new_tree->child[1]=MathExpr(pci, ppi);

        pci->debug_file.Out("End Expr");
        return new_tree;
    }
    pci->debug_file.Out("End Expr");
    return tree;
}

// writestmt -> write expr
TreeNode* WriteStmt(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start WriteStmt");

    TreeNode* tree=new TreeNode;
    tree->node_kind=WRITE_NODE;
    tree->line_num=pci->in_file.cur_line_num;

    Match(pci, ppi, WRITE);
    tree->child[0]=Expr(pci, ppi);

    pci->debug_file.Out("End WriteStmt");
    return tree;
}

// readstmt -> read identifier
TreeNode* ReadStmt(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start ReadStmt");

    TreeNode* tree=new TreeNode;
    tree->node_kind=READ_NODE;
    tree->line_num=pci->in_file.cur_line_num;

    Match(pci, ppi, READ);
    if(ppi->next_token.type==ID) AllocateAndCopy(&tree->id, ppi->next_token.str);
    Match(pci, ppi, ID);

    pci->debug_file.Out("End ReadStmt");
    return tree;
}

// assignstmt -> identifier := expr
TreeNode* AssignStmt(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start AssignStmt");

    TreeNode* tree=new TreeNode;
    tree->node_kind=ASSIGN_NODE;
    tree->line_num=pci->in_file.cur_line_num;

    if(ppi->next_token.type==ID) AllocateAndCopy(&tree->id, ppi->next_token.str);
    Match(pci, ppi, ID);
    Match(pci, ppi, ASSIGN); tree->child[0]=Expr(pci, ppi);

    pci->debug_file.Out("End AssignStmt");
    return tree;
}

TreeNode* StmtSeq(CompilerInfo*, ParseInfo*);

// repeatstmt -> repeat stmtseq until expr
TreeNode* RepeatStmt(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start RepeatStmt");

    TreeNode* tree=new TreeNode;
    tree->node_kind=REPEAT_NODE;
    tree->line_num=pci->in_file.cur_line_num;

    Match(pci, ppi, REPEAT); tree->child[0]=StmtSeq(pci, ppi);
    Match(pci, ppi, UNTIL); tree->child[1]=Expr(pci, ppi);

    pci->debug_file.Out("End RepeatStmt");
    return tree;
}

// ifstmt -> if exp then stmtseq [ else stmtseq ] end
TreeNode* IfStmt(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start IfStmt");

    TreeNode* tree=new TreeNode;
    tree->node_kind=IF_NODE;
    tree->line_num=pci->in_file.cur_line_num;

    Match(pci, ppi, IF); tree->child[0]=Expr(pci, ppi);
    Match(pci, ppi, THEN); tree->child[1]=StmtSeq(pci, ppi);
    if(ppi->next_token.type==ELSE) {Match(pci, ppi, ELSE); tree->child[2]=StmtSeq(pci, ppi);}
    Match(pci, ppi, END);

    pci->debug_file.Out("End IfStmt");
    return tree;
}
// declstmt -> (int|real|bool) identifier
TreeNode* DeclStmt(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start DeclStmt");

    TokenType t = ppi->next_token.type; // INT_TYPE, REAL_TYPE, or BOOL_TYPE
    Match(pci, ppi, t);

    if(ppi->next_token.type!=ID) throw 0;

    TreeNode* tree=new TreeNode;
    tree->node_kind = DECL_NODE;
    AllocateAndCopy(&tree->id, ppi->next_token.str);
    // store declared type in expr_data_type (reuse enum)
    if(t==INT_TYPE) tree->expr_data_type = INTEGER;
    else if(t==REAL_TYPE) tree->expr_data_type = REAL;
    else tree->expr_data_type = BOOLEAN;

    tree->line_num = pci->in_file.cur_line_num;
    Match(pci, ppi, ID);

    pci->debug_file.Out("End DeclStmt");
    return tree;
}
// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
TreeNode* Stmt(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start Stmt");

    // Compare the next token with the First() of possible statements
    TreeNode* tree=0;
    if(ppi->next_token.type==IF) tree=IfStmt(pci, ppi);
    else if(ppi->next_token.type==REPEAT) tree=RepeatStmt(pci, ppi);
    else if(ppi->next_token.type==ID) tree=AssignStmt(pci, ppi);
    else if(ppi->next_token.type==READ) tree=ReadStmt(pci, ppi);
    else if(ppi->next_token.type==WRITE) tree=WriteStmt(pci, ppi);
    else throw 0;

    pci->debug_file.Out("End Stmt");
    return tree;
}

// stmtseq -> stmt { ; stmt }
TreeNode* StmtSeq(CompilerInfo* pci, ParseInfo* ppi)
{
    pci->debug_file.Out("Start StmtSeq");

    TreeNode *first_tree =nullptr, *last_tree =nullptr;

    // enforce declaraion rules first 
    while(ppi->next_token.type==INT_TYPE||ppi->next_token.type==REAL_TYPE|| ppi->next_token.type==BOOL_TYPE){
        TreeNode* node = DeclStmt(pci, ppi);

        if(first_tree==nullptr)
            first_tree = node;
        else
            last_tree->sibling = node;
        last_tree = node;

        Match(pci, ppi, SEMI_COLON);  
      }

      // enforce that only accept the non-declarative statments 

    while(ppi->next_token.type==IF || ppi->next_token.type==REPEAT ||
        ppi->next_token.type==ID || ppi->next_token.type==READ ||ppi->next_token.type==WRITE )
    {
        TreeNode* next_tree=Stmt(pci, ppi);
        if(first_tree==nullptr)
            first_tree = next_tree;
        else
            last_tree->sibling = next_tree;
        last_tree = next_tree;

        Match(pci, ppi, SEMI_COLON);
    }

    pci->debug_file.Out("End StmtSeq");
    return first_tree;
}

// program -> stmtseq
TreeNode* Parse(CompilerInfo* pci)
{
    ParseInfo parse_info;
    GetNextToken(pci, &parse_info.next_token);

    TreeNode* syntax_tree=StmtSeq(pci, &parse_info);

    if(parse_info.next_token.type!=ENDFILE)
        pci->debug_file.Out("Error code ends before file ends");

    return syntax_tree;
}

void PrintTree(TreeNode* node, int sh=0)
{
    int i, NSH=3;
    for(i=0;i<sh;i++) printf(" ");

    printf("[%s]", NodeKindStr[node->node_kind]);

    if(node->node_kind==OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
    else if(node->node_kind==NUM_NODE) {
        if(node->expr_data_type == REAL)
            printf("[%g]", node->realnum); // Print real number
        else
            printf("[%d]", node->num);     // Print integer
    }    
else if(node->node_kind==ID_NODE || node->node_kind==READ_NODE || node->node_kind==ASSIGN_NODE) printf("[%s]", node->id);

    if(node->expr_data_type!=VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);

    printf("\n");

    for(i=0;i<MAX_CHILDREN;i++) if(node->child[i]) PrintTree(node->child[i], sh+NSH);
    if(node->sibling) PrintTree(node->sibling, sh);
}

void DestroyTree(TreeNode* node)
{
    int i;

    if(node->node_kind==ID_NODE || node->node_kind==READ_NODE || node->node_kind==ASSIGN_NODE)
        if(node->id) delete[] node->id;

    for(i=0;i<MAX_CHILDREN;i++) if(node->child[i]) DestroyTree(node->child[i]);
    if(node->sibling) DestroyTree(node->sibling);

    delete node;
}

////////////////////////////////////////////////////////////////////////////////////
// Analyzer ////////////////////////////////////////////////////////////////////////

const int SYMBOL_HASH_SIZE=10007;

struct LineLocation
{
    int line_num;
    LineLocation* next;
};

struct VariableInfo
{
    char* name;
    int memloc;
    int var_type; // 0=int,1=real,2=bool // added for extended types : Assignment 2
    LineLocation* head_line; // the head of linked list of source line locations
    LineLocation* tail_line; // the tail of linked list of source line locations
    VariableInfo* next_var; // the next variable in the linked list in the same hash bucket of the symbol table
};

struct SymbolTable
{
    int num_vars;
    VariableInfo* var_info[SYMBOL_HASH_SIZE];

    SymbolTable() {num_vars=0; int i; for(i=0;i<SYMBOL_HASH_SIZE;i++) var_info[i]=0;}

    int Hash(const char* name)
    {
        int i, len=strlen(name);
        int hash_val=11;
        for(i=0;i<len;i++) hash_val=(hash_val*17+(int)name[i])%SYMBOL_HASH_SIZE;
        return hash_val;
    }

    VariableInfo* Find(const char* name)
    {
        int h=Hash(name);
        VariableInfo* cur=var_info[h];
        while(cur)
        {
            if(Equals(name, cur->name)) return cur;
            cur=cur->next_var;
        }
        return 0;
    }
// Insert a new variable. If already exists, print error and return
    void Insert(const char* name, int line_num,int var_type) // added var_type for extended types : Assignment 2
    {
        // Assignment 2
        if(Find(name)) { printf("ERROR Duplicate declaration of %s\n", name); return; }
        LineLocation* lineloc=new LineLocation;
        lineloc->line_num=line_num;
        lineloc->next=0;

        int h=Hash(name);
        VariableInfo* vi=new VariableInfo;
        vi->head_line=vi->tail_line=lineloc;
        vi->next_var=var_info[h];
        vi->memloc=num_vars++;
        vi->var_type=var_type; // added for extended types : Assignment 2
        AllocateAndCopy(&vi->name, name);
        var_info[h]=vi;
    }
    //     VariableInfo* prev=0;
    //     VariableInfo* cur=var_info[h];

    //     while(cur)
    //     {
    //         if(Equals(name, cur->name))
    //         {
    //             // just add this line location to the list of line locations of the existing var
    //             cur->tail_line->next=lineloc;
    //             cur->tail_line=lineloc;
    //             return;
    //         }
    //         prev=cur;
    //         cur=cur->next_var;
    //     }

    //     VariableInfo* vi=new VariableInfo;
    //     vi->head_line=vi->tail_line=lineloc;
    //     vi->next_var=0;
    //     vi->memloc=num_vars++;
    //     AllocateAndCopy(&vi->name, name);

    //     if(!prev) var_info[h]=vi;
    //     else prev->next_var=vi;
    // }

    void Print()
    {
        int i;
        for(i=0;i<SYMBOL_HASH_SIZE;i++)
        {
            VariableInfo* curv=var_info[i];
            while(curv)
            {
                printf("[Var=%s][Mem=%d]", curv->name, curv->memloc);
                LineLocation* curl=curv->head_line;
                while(curl)
                {
                    printf("[Line=%d]", curl->line_num);
                    curl=curl->next;
                }
                printf("\n");
                curv=curv->next_var;
            }
        }
    }

    void Destroy()
    {
        int i;
        for(i=0;i<SYMBOL_HASH_SIZE;i++)
        {
            VariableInfo* curv=var_info[i];
            while(curv)
            {
                LineLocation* curl=curv->head_line;
                while(curl)
                {
                    LineLocation* pl=curl;
                    curl=curl->next;
                    delete pl;
                }
                VariableInfo* p=curv;
                curv=curv->next_var;
                delete p;
            }
            var_info[i]=0;
        }
    }
};



void Analyze(TreeNode* node, SymbolTable* symbol_table)
{
    int i;

    // Handle declarations first: add var to symbol table with declared type
    if(node->node_kind==DECL_NODE)
    {
        // store var_type as 0=int,1=real,2=bool
        int var_type = (node->expr_data_type==INTEGER?0:(node->expr_data_type==REAL?1:2));
        symbol_table->Insert(node->id, node->line_num, var_type);
    }

    // IDs must be declared earlier
    if(node->node_kind==ID_NODE || node->node_kind==READ_NODE || node->node_kind==ASSIGN_NODE)
    {
        VariableInfo* vi=symbol_table->Find(node->id);
        if(!vi) printf("ERROR Undeclared variable %s at line %d\n", node->id, node->line_num);
    }


    for(i=0;i<MAX_CHILDREN;i++) if(node->child[i]) Analyze(node->child[i], symbol_table);


    if(node->node_kind==OPER_NODE)
    {
        if(node->oper==EQUAL || node->oper==LESS_THAN) node->expr_data_type=BOOLEAN;
        else node->expr_data_type=INTEGER; // temporary, refined below
    }
    else if(node->node_kind==ID_NODE)
    {
        VariableInfo* vi=symbol_table->Find(node->id);
        if(vi) node->expr_data_type = (vi->var_type==0?INTEGER:(vi->var_type==1?REAL:BOOLEAN));
    }
    else if(node->node_kind==NUM_NODE)
    {
        // expr_data_type set during NewExpr
    }


    // Type checks for operators: arithmetic only for int/real; no arithmetic on bool
    if(node->node_kind==OPER_NODE)
    {
        ExprDataType left=node->child[0]->expr_data_type;
        ExprDataType right=node->child[1]->expr_data_type;


        if(node->oper==ANDA)
        {
            if(left!=BOOLEAN || right!=BOOLEAN) printf("ERROR & applied to non-boolean\n");
            node->expr_data_type=BOOLEAN;
        }
        else if(node->oper==EQUAL || node->oper==LESS_THAN)
        {
            // equality/less-than allowed on numeric types only

            if((left!=INTEGER && left!=REAL) || (right!=INTEGER && right!=REAL)) printf("ERROR comparison on non-numeric\n");
            node->expr_data_type=BOOLEAN;
        }
        else
        {
        // arithmetic operators

            if(left==BOOLEAN || right==BOOLEAN) printf("ERROR arithmetic on boolean\n");
            if(left==REAL || right==REAL) node->expr_data_type=REAL; else node->expr_data_type=INTEGER;
        }
    }
    if(node->node_kind==IF_NODE && node->child[0]->expr_data_type!=BOOLEAN) printf("ERROR If test must be BOOLEAN\n");
    if(node->node_kind==REPEAT_NODE && node->child[1]->expr_data_type!=BOOLEAN) printf("ERROR Repeat test must be BOOLEAN\n");
    if(node->node_kind==WRITE_NODE && (node->child[0]->expr_data_type!=INTEGER && node->child[0]->expr_data_type!=REAL)) printf("ERROR Write works only for numeric types\n");
    if(node->node_kind==ASSIGN_NODE)
    {
        VariableInfo* vi=symbol_table->Find(node->id);
        // enforce boolean to boolean 
        if(vi->var_type==2&& node->child[0]->expr_data_type!=BOOLEAN){
            printf("Cannot assign non-boolean to boolean datatype");
        }
        // enforce int to int
        if(vi->var_type==0&& node->child[0]->expr_data_type!=INTEGER){
            printf("Cannot assign non-integar to int datatype");
        }
        // allow int or real to be stored in real
        if(vi->var_type==1&& node->child[0]->expr_data_type==BOOLEAN){
            printf("Cannot assign boolean to real datatype");
        }
        if(!vi) printf("ERROR Assign to undeclared var %s\n", node->id);
    }
    if(node->sibling) Analyze(node->sibling, symbol_table);

}
////////////////////////////////////////////////////////////////////////////////////
// Code Generator //////////////////////////////////////////////////////////////////
struct Value
{
    ExprDataType type;
    int ival;
    double rval;
    bool bval;
};

Value Evaluate(TreeNode* node, SymbolTable* symbol_table, Value* variables)
{
    if(node->node_kind==NUM_NODE)
    {
        Value v;
        if(node->expr_data_type==REAL) { v.type = REAL; v.rval = node->realnum; }
        else { v.type = INTEGER; v.ival = node->num; }
        return v;
    }
    if(node->node_kind==ID_NODE)
    {
        VariableInfo* vi=symbol_table->Find(node->id);
        if(!vi) throw 0;
        return variables[vi->memloc];
    }

    // evaluate children
    Value a = Evaluate(node->child[0], symbol_table, variables);
    Value b = Evaluate(node->child[1], symbol_table, variables);

    // equality '='
    if(node->oper==EQUAL)
    {
        Value r; r.type = BOOLEAN; r.bval = false;
        if((a.type==INTEGER || a.type==REAL) && (b.type==INTEGER || b.type==REAL))
        {
            double av = (a.type==REAL ? a.rval : (double)a.ival);
            double bv = (b.type==REAL ? b.rval : (double)b.ival);
            r.bval = (fabs(av - bv) < 1e-9);
            return r;
        }
        if(a.type==BOOLEAN && b.type==BOOLEAN)
        {
            r.bval = (a.bval == b.bval);
            return r;
        }
        throw 0;
    }

    // less-than '<'
    if(node->oper==LESS_THAN)
    {
        Value r; r.type = BOOLEAN;
        double av = (a.type==REAL ? a.rval : (double)a.ival);
        double bv = (b.type==REAL ? b.rval : (double)b.ival);
        r.bval = (av < bv);
        return r;
    }

    // arithmetic: + - * / ^
    if(node->oper==PLUS || node->oper==MINUS || node->oper==TIMES || node->oper==DIVIDE || node->oper==POWER)
    {
        double av = (a.type==REAL ? a.rval : (double)a.ival);
        double bv = (b.type==REAL ? b.rval : (double)b.ival);
        double rv = 0.0;
        if(node->oper==PLUS) rv = av + bv;
        else if(node->oper==MINUS) rv = av - bv;
        else if(node->oper==TIMES) rv = av * bv;
        else if(node->oper==DIVIDE) rv = av / bv;
        else if(node->oper==POWER) rv = pow(av, bv);

        Value r;
        if(a.type==REAL || b.type==REAL) { r.type = REAL; r.rval = rv; }
        else { r.type = INTEGER; r.ival = (int)rv; }
        return r;
    }

    // boolean AND '&'
    if(node->oper==ANDA)
    {
        Value r; r.type = BOOLEAN;
        r.bval = a.bval && b.bval;
        return r;
    }

    // unexpected operator/type combination
    throw 0;
}


// Value Evaluate(TreeNode* node, SymbolTable* symbol_table, Value* variables)
// {
//     if(node->node_kind==NUM_NODE)
//     {
//         Value v;
//         if(node->expr_data_type==REAL) { v.type= REAL; v.rval=node->realnum; }
//         else { v.type=INTEGER; v.ival=node->num; }
//         return v;
//     }
//     if(node->node_kind==ID_NODE)
//     {
//         VariableInfo* vi=symbol_table->Find(node->id);
//         if(!vi) throw 0;
//         return variables[vi->memloc];
//     }


//     Value a=Evaluate(node->child[0], symbol_table, variables);
//     Value b=Evaluate(node->child[1], symbol_table, variables);


//     // handle operators
//     if(node->oper==EQUAL)
//     {
//         Value r; r.type=BOOLEAN; r.bval=false;
//         if((a.type==INTEGER || a.type==REAL) && (b.type==INTEGER || b.type==REAL))
//         {
//             double av=(a.type==REAL? a.rval : (double)a.ival);
//             double bv=(b.type==REAL? b.rval : (double)b.ival);
//             r.bval = fabs(av - bv) < 1e-9;
//             return r;
//         }
//         if (a.type == BOOLEAN && b.type == BOOLEAN)
//         {
//             r.bval = (a.bval == b.bval);
//             return r;
//         }
        
//         throw 0;
//     }
//     if(node->oper==LESS_THAN)
//     {
//         Value r; r.type=BOOLEAN;
//         double av=(a.type==REAL? a.rval : (double)a.ival);
//         double bv=(b.type==REAL? b.rval : (double)b.ival);
//         r.bval = av < bv;
//         return r;
//     }
//     if(node->oper==PLUS || node->oper==MINUS || node->oper==TIMES || node->oper==DIVIDE || node->oper==POWER)
//     {
//         // arithmetic only for int/real
//         double av=(a.type==REAL? a.rval : (double)a.ival);
//         double bv=(b.type==REAL? b.rval : (double)b.ival);

//         Value r;
//         if(a.type==REAL || b.type==REAL) r.type=REAL; else r.type=INTEGER;
//         double rv=0.0;
//         if(node->oper==PLUS) rv=av + bv;
//         else if(node->oper==MINUS) rv=av - bv;
//         else if(node->oper==TIMES) rv=av * bv;
//         else if(node->oper==DIVIDE) rv=av / bv;
//         else if(node->oper==POWER) rv = pow(av, bv);
//     }
//     if(node->oper==ANDA)
//     {
//         Value r; r.type=BOOLEAN;
//         r.bval = a.bval && b.bval;
//         return r;
//     }
//         // should not reach here
//     throw 0;
//     // return a default to silence compiler (unreachable)
//     Value vv; vv.type = INTEGER; vv.ival = 0; return vv;
// }




void RunProgram(TreeNode* node, SymbolTable* symbol_table, Value* variables)
{
if(node->node_kind==IF_NODE)
{
Value cond=Evaluate(node->child[0], symbol_table, variables);
if(cond.type!=BOOLEAN) printf("ERROR If condition not boolean\n");
if(cond.bval) RunProgram(node->child[1], symbol_table, variables);
else if(node->child[2]) RunProgram(node->child[2], symbol_table, variables);
}
if(node->node_kind==ASSIGN_NODE)
{
Value v=Evaluate(node->child[0], symbol_table, variables);
VariableInfo* vi=symbol_table->Find(node->id);
if(!vi) { printf("ERROR Assign to undeclared %s\n", node->id); }
else
{
// enforce no cross-type assign except int->real
if(vi->var_type==2)
{
if(v.type!=BOOLEAN) printf("ERROR Assigning non-boolean to boolean %s\n", node->id);
else variables[vi->memloc]=v;
}
else if(vi->var_type==0)
{
if(v.type!=INTEGER) printf("ERROR Assigning non-integer to int %s\n", node->id);
else variables[vi->memloc]=v;
}
else if(vi->var_type==1)
{
if(v.type==BOOLEAN) printf("ERROR Assigning boolean to real %s\n", node->id);
else
{
// convert int to real if needed
if(v.type==INTEGER) { Value vv; vv.type=REAL; vv.rval=v.ival; variables[vi->memloc]=vv; }
else variables[vi->memloc]=v;
}
}
}
}
if(node->node_kind==READ_NODE)
{
VariableInfo* vi=symbol_table->Find(node->id);
if(!vi) { printf("ERROR Read undeclared %s\n", node->id); }
else
{
if(vi->var_type==2) { int t; scanf("%d", &t); variables[vi->memloc].type=BOOLEAN; variables[vi->memloc].bval = (t!=0); }
else if(vi->var_type==0) { int t; scanf("%d", &t); variables[vi->memloc].type=INTEGER; variables[vi->memloc].ival=t; }
else { double t; scanf("%lf", &t); variables[vi->memloc].type=REAL; variables[vi->memloc].rval=t; }
}
}
if(node->node_kind==WRITE_NODE)
{
Value v=Evaluate(node->child[0], symbol_table, variables);
if(v.type==INTEGER) printf("Val: %d\n", v.ival);
else if(v.type==REAL) printf("Val: %g\n", v.rval);
else printf("Val: %d\n", v.bval);
}
if(node->node_kind==REPEAT_NODE)
{
do
{
RunProgram(node->child[0], symbol_table, variables);
}
while(!Evaluate(node->child[1], symbol_table, variables).bval);
}
if(node->sibling) RunProgram(node->sibling, symbol_table, variables);
}

void RunProgram(TreeNode* syntax_tree, SymbolTable* symbol_table)
{
    int i;
    Value* variables=new Value[symbol_table->num_vars];
    for(i=0;i<symbol_table->num_vars;i++) { variables[i].type=INTEGER; variables[i].ival=0; }
    RunProgram(syntax_tree, symbol_table, variables);
    delete[] variables;
}

////////////////////////////////////////////////////////////////////////////////////
// Scanner and Compiler ////////////////////////////////////////////////////////////

void StartCompiler(CompilerInfo* pci)
{
    TreeNode* syntax_tree=Parse(pci);

    SymbolTable symbol_table;
    Analyze(syntax_tree, &symbol_table);

    printf("Symbol Table:\n");
    symbol_table.Print();
    printf("---------------------------------\n"); fflush(NULL);

    printf("Syntax Tree:\n");
    PrintTree(syntax_tree);
    printf("---------------------------------\n"); fflush(NULL);

    printf("Run Program:\n");
    RunProgram(syntax_tree, &symbol_table);
    printf("---------------------------------\n"); fflush(NULL);

    symbol_table.Destroy();
    DestroyTree(syntax_tree);
}

////////////////////////////////////////////////////////////////////////////////////
// Scanner only ////////////////////////////////////////////////////////////////////

void StartScanner(CompilerInfo* pci)
{
    Token token;

    while(true)
    {
        GetNextToken(pci, &token);
        printf("[%d] %s (%s)\n", pci->in_file.cur_line_num, token.str, TokenTypeStr[token.type]); fflush(NULL);
        if(token.type==ENDFILE || token.type==ERROR) break;
    }
}

////////////////////////////////////////////////////////////////////////////////////

int main()
{
    printf("Start main()\n"); fflush(NULL);

    CompilerInfo compiler_info("input.txt", "output.txt", "debug.txt");

    StartCompiler(&compiler_info);

    printf("End main()\n"); fflush(NULL);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
