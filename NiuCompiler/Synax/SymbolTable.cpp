#include "SymbolTable.h"


bool SymbolTable::AddSymbol(const Symbol& s)
{
    if (table.count(s.id))return 0;
    table[s.id] = s;
    return 1;
}

SymbolStack::SymbolStack()
{
    AddStack();
}

void SymbolStack::AddStack()
{   
    stack.push_back(new SymbolTable);
}

void SymbolStack::PopStack()
{
    auto& v = stack.back();
    stack.pop_back();
    delete v;
}

bool SymbolStack::PushSymbol(const Symbol& s)
{
    if (IsLocalSymbolExist(s.id))return 0;
    stack.back()->AddSymbol(s);
    return 1;
}

bool SymbolStack::IsLocalSymbolExist(const string& s)
{
    if (stack.back()->table.count(s))return 1;
    return 0;
}

bool SymbolStack::IsGlobalSymbolExist(const string& s)
{
    if (GetSymbol(s).type.type!= SYMBOL_VOID)return 1;
    return 0;
}

Symbol& SymbolStack::GetSymbol(const string& s)
{
    static Symbol empty = { SYMBOL_VOID };
    for (auto itr = stack.rbegin(); itr != stack.rend(); ++itr)
    {
        auto& table = *itr;
        if (table->table.count(s))return table->table[s];
    }
    return empty;
}

SymbolStack::~SymbolStack()
{
    for (auto* node : stack)delete node;
}

bool Symbol::operator==(const Symbol&s) const
{
    return s.id == id;
}
