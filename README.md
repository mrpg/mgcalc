mgcalc
========

Simple arbitrary-precision calculator

This is a simple arbitrary-precision calculator that is capable of handling fractions. Commands are read from left to right, i.e. "2+3*4" is interpreted as "(2+3)*4" (you will want to use a lot of brackets!). Compile using

> make

install with (root privileges required)

> make install

Usage
----

Run the program using ./mgcalc, or, if you have used make install,

> mgcalc

No arguments are accepted. You can then enter commands. If you enter something that can be calculated, the result will be shown immediately. *If the exact output is used, mgcalc will always output a fraction.* Example:

> 2+3*4

> 20/1

... which is correct since (2+3)*4 = 20 and mgcalc always reads from left to right. (Multiplication does *not* precede addition.) Another example:

> 5/(4+(2/8 + 1))

> 40/42

It is possible to suppress the output of fractions using

> $echo off

Output can be re-enabled using

> $echo

It's also possible to redirect output to a file using

> $file out.txt

To go back to stdout, use

> $file

Finally, and perhaps most importantly, it is possible to use long division to express the result of a calculation as a decimal number. Example:

> 5/(4+(2/8 + 1))

> 40/42

> $div 10

> 0.9523809523

The argument given to $div (here: 10) specifies the number of decimal places. *$echo off does not suppress the output of the long division.* For example, consider approximating Euler's number e:

> $echo off

> 1 + (1/(1!0)) + (1/(2!0)) + (1/(3!0)) + (1/(4!0)) + (1/(5!0))

> $div 2

> 2.71

Note three things: First of all, *you want to use a lot of brackets*. Secondly, since +, -, *, /, ^ are strictly binary, so is ! (factorial) -- sorry! Obviously, the second argument to ! is meaningless, i.e. it can be anything. If you type "5!0", it is just 5! = 120. Thirdly, note that "$echo off" does not suppress the output of $div. It only suppressed the output of the fraction after line 2.

That's all. There are no limitations other than those implied by your hardware. You could do the following, although I am not sure why you would:

> $echo off

> $file out.txt

> (1/(2+(1/(2+(1/(2+(1/(2+(1/(2+(1/(2+(1/(2+(1/(2+(1/(2+(1/(2+3))))))))))))))))))))^(1+(2*(1+(2*(1+(2*(1+(2*(1+(2*(1+(2*(1+(2*3))))))))))))))

> $div 1000000

Todo
----

  - Make factorial more unary
  - Rewrite some algorithms
  - Parallelization
