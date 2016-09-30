#include <iostream>
#include <string>
#include <iomanip>

#include "lambda.hh"

using namespace std;

#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2(x)

#define ASSERT_EQ(STUFF1, STUFF2) \
    { \
        string tmp1 = STUFF1; \
        string tmp2 = STUFF2; \
        if (tmp1 != tmp2) {   \
            throw "Error in line " TOSTRING(__LINE__) ":\n" \
                  "    evaluating " #STUFF1 "\n"   \
                  "    expected value: " + tmp2 + "\n"   \
                  "    actual value:   " +tmp1; \
        } \
    }

#define TESTCASE_START(TITLE, SCORE, RUN) \
    cout << "(" << setfill(' ') << setw(2) << SCORE << "%)  " << TITLE << endl; \
    score_add = SCORE; \
    try {  \
        if (RUN) {

#define TESTCASE_END \
            cout << setw(60) << setfill('-') << "PASS" << endl;\
            score += score_add; \
        } else { \
            cout << setw(60) << setfill('-') << "SKIP" << endl;\
        } \
    } catch (string str) { \
        cout << str << endl; \
        cout << setw(60) << setfill('-') << "FAIL" << endl; \
    }


template<typename ...rest_t>
string _(string first, string second, rest_t... rest) {
    return _("("+first+" "+second+")", rest...);
}

template<>
string _(string first, string second) {
    lambda l("("+first+" "+second+")");
    l.reduce();
    return l.to_string();
}

int main() {
    int score = 0, score_add;
    TESTCASE_START("Simple expressions", 20, true)
        ASSERT_EQ(lambda("\\a.a").to_string(), "\\a.a");
        ASSERT_EQ(lambda("\\a.(a a)").to_string(), "\\a.(a a)");
        ASSERT_EQ(lambda("\\a.\\b.b").to_string(), "\\a.\\b.b");
        ASSERT_EQ(lambda("(\\a.a \\b.b)").to_string(), "(\\a.a \\b.b)");
   TESTCASE_END;
   TESTCASE_START("Complicated expressions", 40, true)
        ASSERT_EQ(lambda("(\\a.\\b.(b a) (\\c.c \\d.d))").to_string(),
                    "(\\a.\\b.(b a) (\\c.c \\d.d))");
        ASSERT_EQ(lambda("(\\a.\\b.(b a) (\\c.c \\d.d))").to_string(),
                    "(\\a.\\b.(b a) (\\c.c \\d.d))");
        ASSERT_EQ(lambda("(\\a.\\b.(b a) (\\c.c \\d.d))").to_string(),
                    "(\\a.\\b.(b a) (\\c.c \\d.d))");
    TESTCASE_END;
    TESTCASE_START("One application to evaluate", 10, true)
        lambda l1("(\\b.b \\a.a)");
        l1.reduce();
        ASSERT_EQ(l1.to_string(), "\\a.a");
        lambda l2("(\\c.\\a.c \\b.b)");
        l2.reduce();
        ASSERT_EQ(l2.to_string(), "\\a.\\b.b");
        l2.reduce();
        ASSERT_EQ(l2.to_string(), "\\a.\\b.b");
        lambda l3("\\a.(\\b.b a)");
        l3.reduce();
        ASSERT_EQ(l3.to_string(), "\\a.a");
        lambda l4("\\a.(\\b.(b b) a)");
        l4.reduce();
        ASSERT_EQ(l4.to_string(), "\\a.(a a)");
    TESTCASE_END;
    TESTCASE_START("Multiple applications to evaluate", 20, true)
        lambda l1("((\\c.c \\b.b) \\a.a)");
        l1.reduce();
        ASSERT_EQ(l1.to_string(), "\\a.a");
        lambda l2("(\\c.c (\\b.b \\a.a))");
        l2.reduce();
        ASSERT_EQ(l2.to_string(), "\\a.a");
        lambda l3("((\\d.\\c.(c d) \\a.(a a)) \\b.b)");
        l3.reduce();
        ASSERT_EQ(l3.to_string(), "\\a.(a a)");
        lambda l4("\\a.((\\e.(e e) \\c.c) (\\d.d \\b.(a a)))");
        l4.reduce();
        ASSERT_EQ(l4.to_string(), "\\a.\\b.(a a)");
    TESTCASE_END;
    TESTCASE_START("Name collision", 10, true)
        lambda l1("((\\b.\\b.b \\c.c) \\a.(a a))");
        l1.reduce();
        ASSERT_EQ(l1.to_string(), "\\a.(a a)");
        lambda l2("((\\a.\\a.(\\a.a a) \\a.a) \\a.((a a) a))");
        l2.reduce();
        ASSERT_EQ(l2.to_string(), "\\a.((a a) a)");
    TESTCASE_END;
    TESTCASE_START("Name normalisation", 10, false)
        ASSERT_EQ(lambda("\\x.x").to_string(), "\\a.a");
        ASSERT_EQ(lambda("\\x.\\y.x").to_string(), "\\a.\\b.a");
        ASSERT_EQ(lambda("(\\x.\\y.(y x) \\z.z)").to_string(), "(\\a.\\c.(c a) \\b.b)");
        ASSERT_EQ(lambda("\\b.\\a.b").to_string(), "\\a.\\b.a");
        ASSERT_EQ(lambda("\\x.\\x.x").to_string(), "\\a.\\b.b");
        ASSERT_EQ(lambda("\\b.\\b.\\a.(a b)").to_string(), "\\a.\\b.\\c.(c b)");
    TESTCASE_END;
    TESTCASE_START("Proper evaluation order", 0, true)
        lambda l("((\\x.\\y.x \\a.a) (\\c.(c c) \\b.(b b)))");
        l.reduce();
        ASSERT_EQ(l.to_string(), "\\a.a");
    TESTCASE_END;
    TESTCASE_START("Boolean logic (grand integrated test)", 0, true)
        string True = "\\a.\\b.a";
        string False ="\\a.\\b.b";
        string If = "\\a.\\b.\\c.((c a) b)";
        string Id = "\\a.a";
        string Not = _(If, False, True);
        string Or = _(If, True);
        string And = _(If, Id, _(True, False));
        string Xor = _(If, Not, Id);

        ASSERT_EQ(_(Not, True), False);
        ASSERT_EQ(_(Not, False), True);

        ASSERT_EQ(_(Or, True, True), True);
        ASSERT_EQ(_(Or, True, False), True);
        ASSERT_EQ(_(Or, False, True), True);
        ASSERT_EQ(_(Or, False, False), False);

        ASSERT_EQ(_(And, True, True), True);
        ASSERT_EQ(_(And, True, False), False);
        ASSERT_EQ(_(And, False, True), False);
        ASSERT_EQ(_(And, False, False), False);

        ASSERT_EQ(_(Xor, True, True), False);
        ASSERT_EQ(_(Xor, True, False), True);
        ASSERT_EQ(_(Xor, False, True), True);
        ASSERT_EQ(_(Xor, False, False), False);
    TESTCASE_END;
    cout << endl
         << "Your ESTIMATED total score is " << score << "%." << endl
         << "This DOES NOT include any possible penalties for memory" << endl
         << "leaks, undefined behaviour or bad code in general." << endl;
    return 0;
}
