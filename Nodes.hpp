#ifndef NODES_H
#define NODES_H

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

#include <vector>
#include <iostream>
#include <memory>
#include <functional>
#include <unordered_set>

namespace monicelli {

enum class Type {
    INT,
    CHAR,
    FLOAT,
    BOOL,
    DOUBLE,
    VOID
};

std::ostream& operator<<(std::ostream &stream, const Type &type);


template <class T>
class Pointer: public std::unique_ptr<T> {
public:
    Pointer(T *p = nullptr): std::unique_ptr<T>(p) {}
};


class Emittable {
public:
    virtual void emit(std::ostream &stream, int indent = 0) = 0;
};


class Statement: public Emittable {
public:
    virtual ~Statement() {}
};


class SemiExpression: public Emittable {
public:
    virtual ~SemiExpression() {}
};


class Expression: public Emittable {
public:
    virtual ~Expression() {}
};


template<class T>
class PointerList: public std::vector<T*> {
public:
    virtual ~PointerList() {
        for (T *element: *this) {
            delete element;
        }
    }
};


class StatementList: public PointerList<Statement>, public Emittable {
public:
    virtual void emit(std::ostream &stream, int indent = 0);
};


template <class T>
class ListEmittable: public PointerList<T>, public Emittable {
public:
    virtual void emit(std::ostream &stream, int indent = 0) {
        if (this->size() > 0) {
            T *last = this->back(); // TODO wut?
            for (T *e: *this) {
                e->emit(stream);
                if (e != last) {
                    stream << getSeparator();
                }
            }
        }
    }

protected:
    virtual std::string getSeparator() const {
        return ", ";
    }
};


class ExpressionList: public ListEmittable<Expression> {
};


class SimpleExpression: public Expression {
};


class Id: public SimpleExpression {
public:
    explicit Id(std::string *c): value(c) {}
    virtual ~Id() {}

    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<std::string> value;
};


class IdList: public ListEmittable<Id> {
};


class Number: public SimpleExpression {
public:
    virtual void emit(std::ostream &stream, int indent = 0) {}
};


class Integer: public Number {
public:
    Integer(long i): value(i) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    long value;
};


class Float: public Number {
public:
    Float(double f): value(f) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    double value;
};


class Return: public Statement {
public:
    explicit Return(Expression *e): expression(e) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Expression> expression;
};


class Loop: public Statement {
public:
    Loop(StatementList *b, Expression *c): body(b), condition(c) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<StatementList> body;
    Pointer<Expression> condition;
};


class VarDeclaration: public Statement {
public:
    VarDeclaration(Id *n, Type t, bool p, Expression *i):
        name(n), point(p), init(i), type(t) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Id> name;
    bool point;
    Pointer<Expression> init;
    Type type;
};


class Assignment: public Statement {
public:
    Assignment(Id *n, Expression *v): name(n), value(v) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Id> name;
    Pointer<Expression> value;
};


class Print: public Statement {
public:
    explicit Print(Expression *e): expression(e) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Expression> expression;
};


class Input: public Statement {
public:
    explicit Input(Id *v): variable(v) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Id> variable;
};


class Abort: public Statement {
public:
    virtual void emit(std::ostream &stream, int indent = 0);
};


class Assert: public Statement {
public:
    explicit Assert(Expression *e): expression(e) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Expression> expression;
};


class FunctionCall: public Statement, public Expression {
public:
    FunctionCall(Id *n, ExpressionList *a): name(n), args(a) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Id> name;
    Pointer<ExpressionList> args;
};


class BranchCase: public Emittable {
public:
    BranchCase(SemiExpression *c, StatementList *b): condition(c), body(b) {}
    virtual ~BranchCase() {}

    virtual void emit(std::ostream &stream, int indent = 0);
private:
    Pointer<SemiExpression> condition;
    Pointer<StatementList> body;
};


typedef PointerList<BranchCase> BranchCaseList;


class Branch: public Statement {
public:
    struct Body {
    public:
        Body(BranchCaseList *c, StatementList *e = nullptr): cases(c), els(e) {}

    private:
        Pointer<BranchCaseList> cases;
        Pointer<StatementList> els;

        friend class Branch;
    };

    Branch(Id *v, Branch::Body *b): var(v), body(b) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Id> var;
    Pointer<Branch::Body> body;
};


class Main: public Emittable {
public:
    Main(StatementList *s): body(s) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<StatementList> body;
};


class FunArg: public Emittable {
public:
    FunArg(Id *n, Type t, bool p): name(n), type(t), pointer(p) {}
    virtual ~FunArg() {}

    virtual void emit(std::ostream &stream, int indent = 0);

private:
    Pointer<Id> name;
    Type type;
    bool pointer;
};


typedef ListEmittable<FunArg> FunArgList;


class Function: public Emittable {
public:
    Function(Id *n, Type r, FunArgList *a, StatementList *b):
        name(n), type(r), args(a), body(b) {}
    virtual ~Function() {}

    virtual void emit(std::ostream &stream, int indent = 0);
    void emitSignature(std::ostream &stream, int indent = 0);

private:
    Pointer<Id> name;
    Type type;
    Pointer<FunArgList> args;
    Pointer<StatementList> body;
};


class Module: public Emittable {
public:
    enum Type {
        SYSTEM, USER
    };

    Module(const std::string &n, Type s): name(n), type(s) {}
    virtual ~Module() {}

    bool operator==(const Module &other) const noexcept {
        return (name == other.name) && (type == other.type);
    }

    size_t hash() const noexcept {
        return std::hash<std::string>()(name) ^ std::hash<bool>()(type);
    }

    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::string name;
    Type type;
};

} // namespace

namespace std {

template<>
class hash<monicelli::Module> {
public:
    size_t operator ()(const monicelli::Module &e) const noexcept {
        return e.hash();
    }
};

}

namespace monicelli {

class Program: public Emittable {
public:
    virtual void emit(std::ostream &stream, int indent = 0);

    void setMain(Main *m) {
        main = Pointer<Main>(m);
    }

    void addFunction(Function *f) {
        functions.push_back(f);
    }

    void addModule(Module *m) {
        modules.insert(std::move(*m));
        delete m;
    }

private:
    Pointer<Main> main;
    PointerList<Function> functions;
    std::unordered_set<Module> modules;
};


class ExpNode: public Expression {
public:
    ExpNode(Expression *l, Expression *r): left(l), right(r) {}
    virtual void emit(std::ostream &stream, int indent = 0);

protected:
    virtual std::string getSym() = 0;

private:
    Pointer<Expression> left;
    Pointer<Expression> right;
};


class ExpLt: public ExpNode {
public:
    ExpLt(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return "<";
    }
};


class ExpGt: public ExpNode {
public:
    ExpGt(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return ">";
    }
};


class ExpLte: public ExpNode {
public:
    ExpLte(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return "<=";
    }
};


class ExpGte: public ExpNode {
public:
    ExpGte(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return ">=";
    }
};


class ExpPlus: public ExpNode {
public:
    ExpPlus(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return "+";
    }
};


class ExpMinus: public ExpNode {
public:
    ExpMinus(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return "-";
    }
};


class ExpTimes: public ExpNode {
public:
    ExpTimes(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return "*";
    }
};


class ExpDiv: public ExpNode {
public:
    ExpDiv(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return "/";
    }
};


class ExpShl: public ExpNode {
public:
    ExpShl(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return "<<";
    }
};


class ExpShr: public ExpNode {
public:
    ExpShr(Expression *l, Expression *r): ExpNode(l, r) {}

protected:
    virtual std::string getSym() {
        return ">>";
    }
};


class SemiExpNode: public SemiExpression {
public:
    SemiExpNode(Expression *l): left(l) {}
    virtual void emit(std::ostream &stream, int indent = 0);

protected:
    virtual std::string getSym() = 0;

private:
    Pointer<Expression> left;
};



class SemiExpEq: public SemiExpNode {
public:
    SemiExpEq(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "==";
    }
};


class SemiExpLt: public SemiExpNode {
public:
    SemiExpLt(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "<";
    }
};


class SemiExpGt: public SemiExpNode {
public:
    SemiExpGt(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return ">";
    }
};


class SemiExpLte: public SemiExpNode {
public:
    SemiExpLte(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "<=";
    }
};


class SemiExpGte: public SemiExpNode {
public:
    SemiExpGte(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return ">=";
    }
};


class SemiExpPlus: public SemiExpNode {
public:
    SemiExpPlus(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "+";
    }
};


class SemiExpMinus: public SemiExpNode {
public:
    SemiExpMinus(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "-";
    }
};


class SemiExpTimes: public SemiExpNode {
public:
    SemiExpTimes(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "*";
    }
};


class SemiExpDiv: public SemiExpNode {
public:
    SemiExpDiv(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "/";
    }
};


class SemiExpShl: public SemiExpNode {
public:
    SemiExpShl(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return "<<";
    }
};


class SemiExpShr: public SemiExpNode {
public:
    SemiExpShr(Expression *l): SemiExpNode(l) {}

protected:
    virtual std::string getSym() {
        return ">>";
    }
};

} // namespace

#endif

