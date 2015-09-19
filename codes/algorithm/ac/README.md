Aho-Corasick State Machine - DFA learning
=========================================

the behaviour of the pattern matching machine is dictated by three functions: a goto function *g*, a failure function *f*, and an output function *output*


       h      e      r      s
    0----->1----->2----->8----->9
    |      |  i      s
    |      ------>6----->7
    |
    |  s      h      e
    ------>3----->4----->5

    i:     1  2  3  4  5  6  7  8  9
    f(i):  0  0  0  1  2  0  3  0  3

    i     output(i)
    2     {he}
    5     {he, she}
    7     {his}
    9     {hers}

* goto function *g*
</br>  maps <state,symbol> into a state or *fail*
</br>  e.g. the edge labeled h from 0 to 1 indicates that g(0,h) = 1
</br>  the absense of an arrow indicates *fail*
</br>  g(0, a) != 0 for all input symbols 'a'

* failure function /f/
</br>  maps state into a state
</br>  failure functioin is called whenever the goto function report *fail*
</br>  centain states are designated as output states which indicate that a set of keywords has been found.
</br>  the output function formalizes this concept by associating a set of keywords with every state.

+ operating cycle
</br>  s: current state of the machine
</br>  a: current symbol of the input string x
</br>  1. if g(s,a) = s', the machine make a _goto transition_.
</br>     s' and next symbol of x becomes the current input symbol.
</br>     if output(s') != empty, the operating now complete.
</br>  2. if g(s,a) = *fail*, the machine call f, is said to make a _fail transition_. if f(s) = s', the macine report the cycle with s' as the current state and a as the current input symbol.

* Time complexity
  The actual time complexity of matching algorithm depends on how expensive it is:
  1. to determine g(s,a) for each state s and input symbol a
  2. to determine f(s) for each state s
  3. to determine whether output(s) is empty and
  4. to emit output(s)

## Usage ##
``` shell
ENGTYPE=1 ./a.out keyword dir
```

## Reference

> Efficient String matching: An Aid to Bibliographic Search
