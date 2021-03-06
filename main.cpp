#include <iostream>
#include <string>
#include <stack>
#include <set>
#include <vector>
#include <algorithm>
#include <utility>
#define lambda '$'

class Automaton{
private:
    std::set<int> states;

    int startState;
    int finalState;

    std::set<char> alphabet;

    std::vector<int> transitionTable[256][256];

    int numberOfStates = 0;
    int numberOfTransitions = 0;

    static int lastID;
public:

    Automaton(){
        alphabet.insert(lambda);
    }

    void addState(){
        states.insert(lastID);
        numberOfStates++;
        lastID++;
    }

    void addTransition(int start, char letter, int final){
        transitionTable[start][(int) letter].push_back(final);
        numberOfTransitions++;
    }

    void setStart(int id){
        startState = id;
    }

    void setFinal(int id){
        finalState = id;
    }

    void addLetter(char c){
        alphabet.insert(c);
    }

    int getID(){
        return lastID;
    }

    void print(){
        std::cout<<"#states"<<std::endl;
        for(auto i: states){
            std::cout<<'s'<<i<<std::endl;
        }

        std::cout<<"#initial\ns"<<startState<<std::endl;

        std::cout<<"#accepting\ns"<<finalState<<std::endl;

        std::cout<<"#alphabet\n";
        for(auto i: alphabet){
            if(i != lambda)
                std::cout<<i<<std::endl;
        }

        std::cout<<"#transitions"<<std::endl;
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                if(alphabet.find((char)j) != alphabet.end()){
                    if(!transitionTable[i][j].empty()){
                        std::cout<<'s'<<i<<":"<<(char) j<<">";
                        for(auto k: transitionTable[i][j]){
                            std::cout<<'s'<<k<<",";
                        }
                        std::cout<<"\b \n";
                    }

                }
            }
        }
    }
/*
    void convertToNFA(){
        for(int i = 0; i < lastID; i++){
            if(!transitionTable[i]['~'].empty()){
                // compute a vector of edges that end in the first state
                std::vector<std::pair<int, int>> incoming;
                for(int j = 0; j < lastID; j++){
                    for(int k = 0; k < 256; k++){
                        if(std::find(transitionTable[j][k].begin(), transitionTable[j][k].end(), i) != transitionTable[j][k].end()){
                            incoming.push_back(std::make_pair(j, k));
                        }
                    }
                }
                for(auto j: transitionTable[i]['~']){
                    for(auto k: incoming){
                        addTransition(k.first, k.second, j);
                    }
                }
                transitionTable[i]['~'].clear();
            }
        }
    }
    */

    friend Automaton& Connect(Automaton&, Automaton&);
    friend Automaton& Union(Automaton&, Automaton&);
    friend Automaton& Star(Automaton&);
};

int Automaton::lastID = 0;

Automaton& Connect(Automaton &a, Automaton &b){
    Automaton *c = new Automaton;

    c->alphabet = a.alphabet;
    c->alphabet.insert(b.alphabet.begin(), b.alphabet.end());

    c->startState = a.startState;
    c->finalState = b.finalState;

    c->numberOfStates = a.numberOfStates + b.numberOfStates;
    c->states = a.states;
    c->states.insert(b.states.begin(), b.states.end());

    for(int i = 0; i < 256; i++)
        for(int j = 0; j < 256; j++){
            c->transitionTable[i][j] = a.transitionTable[i][j];
            c->transitionTable[i][j].insert(c->transitionTable[i][j].end(),
                                            b.transitionTable[i][j].begin(), b.transitionTable[i][j].end());
        }

    c->addTransition(a.finalState, lambda, b.startState);

    return *c;
}

Automaton& Union(Automaton &a, Automaton &b){
    Automaton *c = new Automaton;

    c->alphabet = a.alphabet;
    c->alphabet.insert(b.alphabet.begin(), b.alphabet.end());

    c->addState();
    c->startState = c->getID() - 1;

    c->addState();
    c->finalState = c->getID() - 1;

    c->numberOfStates += a.numberOfStates + b.numberOfStates;
    c->states.insert(a.states.begin(), a.states.end());
    c->states.insert(b.states.begin(), b.states.end());

    for(int i = 0; i < 256; i++)
        for(int j = 0; j < 256; j++){
            c->transitionTable[i][j] = a.transitionTable[i][j];
            c->transitionTable[i][j].insert(c->transitionTable[i][j].end(),
                                            b.transitionTable[i][j].begin(), b.transitionTable[i][j].end());
        }

    c->addTransition(c->startState, lambda, a.startState);
    c->addTransition(c->startState, lambda, b.startState);

    c->addTransition(a.finalState, lambda, c->finalState);
    c->addTransition(b.finalState, lambda, c->finalState);

    return *c;
}

Automaton& Star(Automaton &a){
    Automaton *c = new Automaton;

    c->alphabet = a.alphabet;

    c->addState();
    c->startState = c->getID() - 1;
    c->addState();
    c->finalState = c->getID() - 1;

    c->numberOfStates += a.numberOfStates;
    c->states.insert(a.states.begin(), a.states.end());

    for(int i = 0; i < 256; i++)
        for(int j = 0; j < 256; j++){
            c->transitionTable[i][j] = a.transitionTable[i][j];
        }

    c->addTransition(c->startState, lambda, a.startState);
    c->addTransition(c->startState, lambda, c->finalState);
    c->addTransition(c->finalState, lambda, a.startState);
    c->addTransition(a.finalState, lambda, c->finalState);

    return *c;
}

class RegularExpression{
private:
    std::stack<Automaton> opStack;
    std::string exp;
    int precedence[256];

    Automaton matcher;

    bool isOperator(char c){
        return (c == '+' || c == '|' || c == '*');
    }

    bool isBracket(char c){
        return (c == '(' || c == ')');
    }

    bool isCharacter(char c){
        return (!isOperator(c) && !isBracket(c));
    }

    void initPrecedence(){
        for(int i = 0; i < 256; i++)
            if((char) i == '+')
                precedence[i] = 2;
            else if((char) i == '|')
                precedence[i] = 1;
            else if((char) i == '*')
                precedence[i] = 3;
            else
                precedence[i] = 0;
    }

    void infixToPostfix(){
        std::stack<char> s;

        std::string result = "";

        exp.push_back(')');
        s.push('(');

        for(int i = 0; i < exp.length(); i++){
            if(isCharacter(exp[i])){
                result.push_back(exp[i]);
            }
            else if(exp[i] =='('){
                s.push('(');
            }
            else if(isOperator(exp[i])){
                while(precedence[s.top()] >= precedence[exp[i]]){
                    char c = s.top();
                    s.pop();
                    result.push_back(c);
                }
                s.push(exp[i]);
            }
            else{
                while(s.top() != '('){
                    char c = s.top();
                    s.pop();
                    result.push_back(c);
                }
                s.pop();
            }
        }

        exp = result;
    }

    void addConcats(){
        for(int i = 0; i < exp.length() - 1; i++){
            if(isCharacter(exp[i]) && isCharacter(exp[i+1])){
                    exp.insert(i+1, "+");
                    i++;
                }
            if(exp[i] == ')' || exp[i] == '*')
                if(isCharacter(exp[i+1])){
                    exp.insert(i+1, "+");
                    i++;
                }
            if(isCharacter(exp[i]) && exp[i+1] == '('){
                exp.insert(i+1, "+");
                i++;
            }
            if(exp[i] == '*' && exp[i+1] == '('){
                exp.insert(i+1, "+");
                i++;
            }
        }
    }

    Automaton& atomic(char c){
        Automaton *tmp = new Automaton();
        tmp->addLetter(c);

        tmp->addState();
        tmp->setStart(tmp->getID() - 1);

        tmp->addState();
        tmp->setFinal(tmp->getID() - 1);

        tmp->addTransition(tmp->getID() - 2, c, tmp->getID() - 1);

        return *tmp;
    }

    void convert(){
        for(int i = 0; i < exp.length(); i++){
            if(isCharacter(exp[i])){
                opStack.push(atomic(exp[i]));
            }
            else{
                if(exp[i] == '+'){
                    Automaton t1 = opStack.top();
                    opStack.pop();

                    Automaton t2 = opStack.top();
                    opStack.pop();

                    matcher = Connect(t2, t1);

                    opStack.push(matcher);
                }
                else if(exp[i] == '|'){
                    Automaton t1 = opStack.top();
                    opStack.pop();

                    Automaton t2 = opStack.top();
                    opStack.pop();

                    matcher = Union(t1, t2);
                    opStack.push(matcher);
                }
                else if(exp[i] == '*'){
                    Automaton t1 = opStack.top();
                    opStack.pop();

                    matcher = Star(t1);
                    opStack.push(matcher);
                }
            }
        }
    }

public:
    RegularExpression(std::string &s){
        initPrecedence();
        exp = s;
        addConcats();
        std::cout<<exp<<std::endl;
        infixToPostfix();
        std::cout<<exp<<std::endl;
        convert();
    }

    void printAutomaton(){
        matcher.print();
    }
};

int main(){

    std::string exp;
    std::cin>>exp;
    RegularExpression reg(exp);

    reg.printAutomaton();

    return 0;
}
