#ifndef NODES_H
#define NODES_H

#include <vector>
#include <iostream>
#include <memory>

namespace monicelli {

enum class Type {
    INT,
    CHAR,
    FLOAT,
    BOOL,
    DOUBLE
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
    std::unique_ptr<std::string> value;
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
    std::unique_ptr<Expression> expression;
};


class Loop: public Statement {
public:
    Loop(StatementList *b, Expression *c): body(b), condition(c) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<StatementList> body;
    std::unique_ptr<Expression> condition;
};


class VarDeclaration: public Statement {
public:
    VarDeclaration(Id *n, Type t, bool p, Expression *i):
        name(n), point(p), init(i), type(t) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<Id> name;
    bool point;
    std::unique_ptr<Expression> init;
    Type type;
};


class Assignment: public Statement {
public:
    Assignment(Id *n, Expression *v): name(n), value(v) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<Id> name;
    std::unique_ptr<Expression> value;
};


class Print: public Statement {
public:
    explicit Print(Expression *e): expression(e) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<Expression> expression;
};


class Input: public Statement {
public:
    explicit Input(Id *v): variable(v) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<Id> variable;
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
    std::unique_ptr<Expression> expression;
};


class FunctionCall: public Statement, public Expression {
public:
    FunctionCall(Id *n, ExpressionList *a): name(n), args(a) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<Id> name;
    std::unique_ptr<ExpressionList> args;
};


class BranchCase: public Emittable {
public:
    BranchCase(SemiExpression *c, StatementList *b): condition(c), body(b) {}
    virtual ~BranchCase() {}

    virtual void emit(std::ostream &stream, int indent = 0);
private:
    std::unique_ptr<SemiExpression> condition;
    std::unique_ptr<StatementList> body;
};


typedef PointerList<BranchCase> BranchCaseList;


class Branch: public Statement {
public:
    Branch(Id *v, BranchCaseList *c, StatementList *e):
        var(v), cases(c), els(e) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<Id> var;
    std::unique_ptr<BranchCaseList> cases;
    std::unique_ptr<StatementList> els;
};


class Main: public Emittable {
public:
    Main(StatementList *s): body(s) {}
    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<StatementList> body;
};


class Function: public Emittable {
public:
    Function(Id *n, IdList *a, StatementList *b):
        name(n), args(a), body(b) {}
    virtual ~Function() {}

    virtual void emit(std::ostream &stream, int indent = 0);

private:
    std::unique_ptr<Id> name;
    std::unique_ptr<IdList> args;
    std::unique_ptr<StatementList> body;
};

class Program: public Emittable {
public:
    virtual void emit(std::ostream &stream, int indent = 0);

    void setMain(Main *m) {
        main = std::unique_ptr<Main>(m);
    }

    void addFunction(Function *f) {
        functions.push_back(f);
    }

private:
    std::unique_ptr<Main> main;
    PointerList<Function> functions;
};


class ExpNode: public Expression {
public:
    ExpNode(Expression *l, Expression *r): left(l), right(r) {}
    virtual void emit(std::ostream &stream, int indent = 0);

protected:
    virtual std::string getSym() = 0;

private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
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
    std::unique_ptr<Expression> left;
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

