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

// Yes, that's right, no ending ;
#define GUARDED(call) if (!(call)) return false

static const std::string STATEMENT_TERMINATOR = ";\n";
static const std::string BLOCK = "    ";


void CppEmitter::indent() {
    indent_chars += 1;
}

void CppEmitter::dedent() {
    indent_chars -= 1;
}

bool CppEmitter::emitIndent() {
    for (int i = 0; i < indent_chars; ++i) {
        stream << BLOCK;
    }

    return stream;
}

bool CppEmitter::emit(Program const& program) {
    for (Module m: program.getModules()) {
        GUARDED(m.emit(this));
        stream << "\n";
    }

    if (!program.getModules().empty()) {
        stream << "\n";
    }

    for (Function const& function: program.getFunctions()) {
        emit(function.getPrototype());
        stream << ";\n";
    }

    if (!program.getFunctions().empty()) {
        stream << "\n";
    }

    for (Function const& function: program.getFunctions()) {
        GUARDED(function.emit(this));
    }

    if (program.getMain()) {
        GUARDED(program.getMain()->emit(this));
    }

    return stream;
}

bool CppEmitter::emitStatements(PointerList<Statement> const& node) {
    for (Statement const& s: node) {
        emitIndent();
        GUARDED(s.emit(this));
        stream << STATEMENT_TERMINATOR;
    }
    return stream;
}

bool CppEmitter::emitMain(Function const& main) {
    stream << "int main() {\n";
    indent();
        emitStatements(main.getBody());
    dedent();
    stream << "}\n";
    return stream;
}

bool CppEmitter::emit(Id const& id) {
    stream << id.getValue();
    return stream;
}

bool CppEmitter::emit(Integer const& num) {
    stream << num.getValue();
    return stream;
}

bool CppEmitter::emit(Float const& num) {
    stream << num.getValue();
    return stream;
}

bool CppEmitter::emit(Return const& node) {
    stream << "return";

    if (node.getExpression()) {
        stream << ' ';
        GUARDED(node.getExpression()->emit(this));
    }

    return stream;
}

bool CppEmitter::emit(Print const& node) {
    bool needsBraces =
        (dynamic_cast<SimpleExpression const*>(&node.getExpression()) == nullptr)
            &&
        (dynamic_cast<FunctionCall const*>(&node.getExpression()) == nullptr)
    ;

    stream << "std::cout << ";
    if (needsBraces) {
        stream << '(';
    }

    GUARDED(node.getExpression().emit(this));

    if (needsBraces) {
        stream << ')';
    }
    stream << " << std::endl";

    return stream;
}

bool CppEmitter::emit(Input const& node) {
    stream << "std::cout << \"";
    GUARDED(node.getVariable().emit(this));
    stream << "? \";\n";
    emitIndent();
    stream << "std::cin >> ";
    GUARDED(node.getVariable().emit(this));

    return stream;
}

bool CppEmitter::emit(Abort const&) {
    stream << "std::exit(1)";

    return stream;
}

bool CppEmitter::emit(Assert const& node) {
    stream << "assert(";
    GUARDED(node.getExpression().emit(this));
    stream << ")";

    return stream;
}

bool CppEmitter::emit(Loop const& loop) {
    stream << "do {\n";
    indent();
        emitStatements(loop.getBody());
    dedent();
    emitIndent();
    stream << "} while (";
    GUARDED(loop.getCondition().emit(this));
    stream << ")";

    return stream;
}

bool CppEmitter::emitBranchCase(BranchCase const& node) {
    emitBranchCondition(node.getCondition());
    stream << ") {\n";
    indent();
        emitStatements(node.getBody());
    dedent();
    emitIndent();
    stream << "}";

    return stream;
}

bool CppEmitter::emit(Branch const& branch) {
    auto &body = branch.getBody();
    auto &var = branch.getVar();

    stream << "if (";
    GUARDED(var.emit(this));

    if (!body.getCases().empty()) {
        BranchCase const& last = body.getCases().back();
        for (BranchCase const& cas: body.getCases()) {
            emitBranchCase(cas);
            if (&cas != &last) {
                stream << " else if (";
                GUARDED(var.emit(this));
            }
        }
    }

    if (!body.getElse()) {
        return stream;
    }

    stream << " else {\n";
    indent();
        emitStatements(*body.getElse());
    dedent();
    emitIndent();
    stream << "}";

    return stream;
}

bool CppEmitter::emit(Assignment const& assignment) {
    GUARDED(assignment.getName().emit(this));
    stream << " = ";
    GUARDED(assignment.getValue().emit(this));

    return stream;
}


bool CppEmitter::emitFunctionArglist(PointerList<Expression> const& args) {
    if (args.empty()) return stream;

    Expression const& last = args.back();
    for (Expression const& arg: args) {
        GUARDED(arg.emit(this));
        if (&arg != &last) {
            stream << ", ";
        }
    }

    return stream;
}


bool CppEmitter::emit(FunctionCall const& funcall) {
    GUARDED(funcall.getName().emit(this));
    stream << "(";
    emitFunctionArglist(funcall.getArgs());
    stream << ")";

    return stream;
}

bool CppEmitter::emit(Function const& function) {
    emit(function.getPrototype());
    stream << " {\n";
    indent();
        emitStatements(function.getBody());
    dedent();
    stream << "}\n\n";

    return stream;
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
        case Type::UNKNOWN:
            stream << "???????????";
            break;
    }

    return stream;
}

bool CppEmitter::emitFunctionParams(PointerList<FunArg> const& funargs) {
    if (funargs.empty()) return stream;

    FunArg const& last = funargs.back();
    for (FunArg const& funarg: funargs) {
        stream << funarg.getType() << (funarg.isPointer()? "* ": " ");
        GUARDED(funarg.getName().emit(this));
        if (&funarg != &last) {
            stream << ", ";
        }
    }

    return stream;
}

bool CppEmitter::emit(Module const& module) {
    bool system = (module.getType() == Module::SYSTEM);
    stream << "#include " << (system? '<': '"') << module.getName() << (system? '>': '"');

    return stream;
}

bool CppEmitter::emit(FunctionPrototype const& proto) {
    if (proto.getName().getValue() == "main") {
        stream << "int ";
    } else {
        stream << proto.getType() << ' ';
    }
    GUARDED(proto.getName().emit(this));
    stream << "(";
    emitFunctionParams(proto.getArgs());
    stream << ")";

    return stream;
}

bool CppEmitter::emit(VarDeclaration const& decl) {
    stream << decl.getType() << ' ';
    if (decl.isPointer()) stream << '*';
    GUARDED(decl.getId().emit(this));

    if (decl.getInitializer()) {
        stream << " = ";
        GUARDED(decl.getInitializer()->emit(this));
    }

    return stream;
}

std::ostream& operator<<(std::ostream &stream, Operator op) {
    switch (op) {
        case Operator::PLUS:
            stream << '+';
            break;
        case Operator::MINUS:
            stream << '-';
            break;
        case Operator::TIMES:
            stream << '*';
            break;
        case Operator::DIV:
            stream << '/';
            break;
        case Operator::SHL:
            stream << "<<";
            break;
        case Operator::SHR:
            stream << ">>";
            break;
        case Operator::LT:
            stream << '<';
            break;
        case Operator::GT:
            stream << '>';
            break;
        case Operator::GTE:
            stream << ">=";
            break;
        case Operator::LTE:
            stream << "<=";
            break;
        case Operator::EQ:
            stream << "==";
            break;
    }

    return stream;
}

bool CppEmitter::emit(BinaryExpression const& node) {
    GUARDED(node.getLeft().emit(this));
    stream << ' ' << node.getOperator() << ' ';
    GUARDED(node.getRight().emit(this));

    return stream;
}

bool CppEmitter::emitBranchCondition(SemiExpression const& node) {
    bool braces = (dynamic_cast<SimpleExpression const*>(&node.getLeft()) == nullptr);

    stream << ' ' << node.getOperator() << ' ';
    if (braces) stream << "(";    
    GUARDED(node.getLeft().emit(this));
    if (braces) stream << ")";

    return stream;
}

