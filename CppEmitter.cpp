/*
 * Monicelli: an esoteric language compiler
 * 
 * Copyright (C) 2014 Stefano Sanfilippo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>

#include "Nodes.hpp"
#include "CppEmitter.hpp"
#include "Pointers.hpp"

using namespace monicelli;


static const std::string STATEMENT_TERMINATOR = ";\n";
static const std::string BLOCK = "    ";


void CppEmitter::indent() {
    indent_chars += 1;
}

void CppEmitter::dedent() {
    indent_chars -= 1;
}

void CppEmitter::emitIndent() {
    for (int i = 0; i < indent_chars; ++i) {
        stream << BLOCK;
    }
}

void CppEmitter::emit(Program const& program) {
    for (Module m: program.getModules()) {
        m.emit(this);
        stream << "\n";
    }

    if (!program.getModules().empty()) {
        stream << "\n";
    }

    for (Function *function: program.getFunctions()) {
        emitFunctionSignature(*function);
        stream << ";\n";
    }

    if (!program.getFunctions().empty()) {
        stream << "\n";
    }

    for (Function *function: program.getFunctions()) {
        function->emit(this);
    }

    if (program.getMain()) {
        program.getMain()->emit(this);
    }
}

void CppEmitter::emitStatements(PointerList<Statement> const& node) {
    for (Statement *s: node) {
        emitIndent();
        s->emit(this);
        stream << STATEMENT_TERMINATOR;
    }
}

void CppEmitter::emit(Main const& main) {
    stream << "int main() {\n";
    indent();
        emitStatements(main.getBody());
    dedent();
    stream << "}\n";
}

void CppEmitter::emit(Id const& id) {
    stream << id.getValue();
}

void CppEmitter::emit(Integer const& num) {
    stream << num.getValue();
}

void CppEmitter::emit(Float const& num) {
    stream << num.getValue();
}

void CppEmitter::emit(Return const& node) {
    stream << "return";

    if (node.getExpression()) {
        stream << ' ';
        node.getExpression()->emit(this);
    }
}

void CppEmitter::emit(Print const& node) {
    bool needsBraces =
        (dynamic_cast<SimpleExpression const*>(&node.getExpression()) == nullptr)
            &&
        (dynamic_cast<FunctionCall const*>(&node.getExpression()) == nullptr)
    ;

    stream << "std::cout << ";
    if (needsBraces) {
        stream << '(';
    }

    node.getExpression().emit(this);

    if (needsBraces) {
        stream << ')';
    }
    stream << " << std::endl";
}

void CppEmitter::emit(Input const& node) {
    stream << "std::cout << \"";
    node.getVariable().emit(this);
    stream << "? \";\n";
    emitIndent();
    stream << "std::cin >> ";
    node.getVariable().emit(this);
}

void CppEmitter::emit(Abort const&) {
    stream << "std::exit(1)";
}

void CppEmitter::emit(Assert const& node) {
    stream << "assert(";
    node.getExpression().emit(this);
    stream << ")";
}

void CppEmitter::emit(Loop const& loop) {
    stream << "do {\n";
    indent();
        emitStatements(loop.getBody());
    dedent();
    emitIndent();
    stream << "} while (";
    loop.getCondition().emit(this);
    stream << ")";
}

void CppEmitter::emit(BranchCase const& node) {
    node.getCondition().emit(this);
    stream << ") {\n";
    indent();
        emitStatements(node.getBody());
    dedent();
    emitIndent();
    stream << "}";
}

void CppEmitter::emit(Branch const& branch) {
    auto &body = branch.getBody();
    auto &var = branch.getVar();

    stream << "if (";
    var.emit(this);

    if (body.getCases().size() > 0) {
        BranchCase *last = body.getCases().back();
        for (BranchCase *cas: body.getCases()) {
            cas->emit(this);
            if (cas != last) {
                stream << " else if (";
                var.emit(this);
            }
        }
    }

    if (!body.getElse()) {
        return;
    }

    stream << " else {\n";
    indent();
        emitStatements(*body.getElse());
    dedent();
    emitIndent();
    stream << "}";
}

void CppEmitter::emit(Assignment const& assignment) {
    assignment.getName().emit(this);
    stream << " = ";
    assignment.getValue().emit(this);
}


void CppEmitter::emitFunctionArglist(PointerList<Expression> const& args) {
    Expression *last = args.back();
    for (Expression const* arg: args) {
        arg->emit(this);
        if (arg != last) {
            stream << ", ";
        }
    }
}


void CppEmitter::emit(FunctionCall const& funcall) {
    funcall.getName().emit(this);
    stream << "(";
    emitFunctionArglist(funcall.getArgs());
    stream << ")";
}

void CppEmitter::emit(Function const& function) {
    emitFunctionSignature(function);
    stream << " {\n";
    indent();
        emitStatements(function.getBody());
    dedent();
    stream << "}\n\n";
}

std::ostream& operator<<(std::ostream &stream, Type const& type) {
    switch (type) {
        case Type::INT:
            stream << "int";
            break;
        case Type::CHAR:
            stream << "char";
            break;
        case Type::FLOAT:
            stream << "float";
            break;
        case Type::BOOL:
            stream << "bool";
            break;
        case Type::DOUBLE:
            stream << "double";
            break;
        case Type::VOID:
            stream << "void";
            break;
    }

    return stream;
}

void CppEmitter::emitFunctionParams(PointerList<FunArg> const& funargs) {
    FunArg *last = funargs.back();

    for (FunArg const* funarg: funargs) {
        stream << funarg->getType() << (funarg->isPointer()? "* ": " ");
        funarg->getName().emit(this);
        if (funarg != last) {
            stream << ", ";
        }
    }
}

void CppEmitter::emit(Module const& module) {
    bool system = (module.getType() == Module::SYSTEM);
    stream << "#include " << (system? '<': '"') << module.getName() << (system? '>': '"');
}

void CppEmitter::emitFunctionSignature(Function const& function) {
    stream << function.getType() << ' ';
    function.getName().emit(this);
    stream << "(";
    emitFunctionParams(function.getArgs());
    stream << ")";
}

void CppEmitter::emit(VarDeclaration const& decl) {
    stream << decl.getType() << ' ';
    if (decl.isPointer()) stream << '*';
    decl.getId().emit(this);

    if (decl.getInitializer()) {
        stream << " = ";
        decl.getInitializer()->emit(this);
    }
}

void CppEmitter::emitExp(char const* symbol, ExpNode const& node) {
    node.getLeft().emit(this);
    stream << ' ' << symbol << ' ';
    node.getRight().emit(this);
}

void CppEmitter::emitSemiExp(char const* symbol, SemiExpNode const& node) {
    bool braces = (dynamic_cast<SimpleExpression const*>(&node.getLeft()) == nullptr);

    stream << ' ' << symbol << ' ';
    if (braces) stream << "(";    
    node.getLeft().emit(this);
    if (braces) stream << ")";
}

void CppEmitter::emit(ExpLt const& node) {
    emitExp("<", node);
}

void CppEmitter::emit(ExpGt const& node) {
    emitExp(">", node);
}

void CppEmitter::emit(ExpLte const& node) {
    emitExp("<=", node);
}

void CppEmitter::emit(ExpGte const& node) {
    emitExp(">=", node);
}

void CppEmitter::emit(ExpPlus const& node) {
    emitExp("+", node);
}

void CppEmitter::emit(ExpMinus const& node) {
    emitExp("-", node);
}

void CppEmitter::emit(ExpTimes const& node) {
    emitExp("*", node);
}

void CppEmitter::emit(ExpDiv const& node) {
    emitExp("/", node);
}

void CppEmitter::emit(ExpShl const& node) {
    emitExp("<<", node);
}

void CppEmitter::emit(ExpShr const& node) {
    emitExp(">>", node);
}

void CppEmitter::emit(SemiExpEq const& node) {
    emitSemiExp("==", node);
}

void CppEmitter::emit(SemiExpLt const& node) {
    emitSemiExp("<", node);
}

void CppEmitter::emit(SemiExpGt const& node) {
    emitSemiExp(">", node);
}

void CppEmitter::emit(SemiExpLte const& node) {
    emitSemiExp("<=", node);
}

void CppEmitter::emit(SemiExpGte const& node) {
    emitSemiExp(">=", node);
}

void CppEmitter::emit(SemiExpPlus const& node) {
    emitSemiExp("+", node);
}

void CppEmitter::emit(SemiExpMinus const& node) {
    emitSemiExp("-", node);
}

void CppEmitter::emit(SemiExpTimes const& node) {
    emitSemiExp("*", node);
}

void CppEmitter::emit(SemiExpDiv const& node) {
    emitSemiExp("/", node);
}

void CppEmitter::emit(SemiExpShl const& node) {
    emitSemiExp("<<", node);
}

void CppEmitter::emit(SemiExpShr const& node) {
    emitSemiExp(">>", node);
}

