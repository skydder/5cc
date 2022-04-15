#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./5cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# assert 0 'return 0;'
# assert 42 'return 42;'

# assert 21 "return 5+20-4;"

# assert 41 "return 12 + 34 -5 ;"

# assert 47 'return 5+6*7;'
# assert 15 ' return 5*(9-6);'
# assert 4 'return (3+5)/2;'

# assert 10 'return -10+20;'

# assert 1  'return 12 < 20;'
# assert 1 'return 20 == 20;'
# assert 0 'return 12 > 20;'
# assert 0 'return 12 >= 20;'
# assert 1 'return 5*(9-6) >= (3+5)/2;'

# assert 3 'a = 2; return a + 1;'
# assert 10 "te = 3; wh = 3; return te * wh + wh / te;"
# assert 10 "test = 3; white = 3; return test * white + white / test;"
# assert 3 "tes = 3; return tes;"
# assert 7 "tes = 3; tes = 7; return tes;"

# assert 0 'return 0;'
# assert 42 'return 42;'
# assert 21 'return 5+20-4;'
# assert 41 'return  12 + 34 - 5 ;'
# assert 47 'return 5+6*7;'
# assert 15 'return 5*(9-6);'
# assert 4 'return (3+5)/2;'
# assert 10 'return -10+20;'
# assert 10 'return - -10;'
# assert 10 'return - - +10;'

# assert 0 'return 0==1;'
# assert 1 'return 42==42;'
# assert 1 'return 0!=1;'
# assert 0 'return 42!=42;'

# assert 1 'return 0<1;'
# assert 0 'return 1<1;'
# assert 0 'return 2<1;'
# assert 1 'return 0<=1;'
# assert 1 'return 1<=1;'
# assert 0 'return 2<=1;'

# assert 1 'return 1>0;'
# assert 0 'return 1>1;'
# assert 0 'return 1>2;'
# assert 1 'return 1>=0;'
# assert 1 'return 1>=1;'
# assert 0 'return 1>=2;'

# assert 3 'a=3; return a;'
# assert 8 'a=3; z=5; return a+z;'

# assert 3 'a=3; return a;'
# assert 8 'a=3; z=5; return a+z;'
# assert 6 'a=b=3; return a+b;'
# assert 3 'foo=3; return foo;'
# assert 8 'foo123=3; bar=5; return foo123+bar;'

# assert 1 'return 1; 2; 3;'
# assert 2 '1; return 2; 3;'
# assert 3 '1; 2; return 3;'

# assert 3 "for (;;) return 3; return 5;"
# assert 55 "i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;"

# assert 3 ' if (0) return 2; return 3; '
# assert 3 ' if (1-1) return 2; return 3; '
# assert 2 ' if (1) return 2; return 3; '
# assert 2 ' if (2-1) return 2; return 3; '

# assert 10 ' i=0; while(i<10)  i=i+1;  return i; '

# assert 0 '{ return 0; }'
# assert 42 '{ return 42; }'
# assert 21 '{ return 5+20-4; }'
# assert 41 '{ return  12 + 34 - 5 ; }'
# assert 47 '{ return 5+6*7; }'
# assert 15 '{ return 5*(9-6); }'
# assert 4 '{ return (3+5)/2; }'
# assert 10 '{ return -10+20; }'
# assert 10 '{ return - -10; }'
# assert 10 '{ return - - +10; }'

# assert 0 '{ return 0==1; }'
# assert 1 '{ return 42==42; }'
# assert 1 '{ return 0!=1; }'
# assert 0 '{ return 42!=42; }'

# assert 1 '{ return 0<1; }'
# assert 0 '{ return 1<1; }'
# assert 0 '{ return 2<1; }'
# assert 1 '{ return 0<=1; }'
# assert 1 '{ return 1<=1; }'
# assert 0 '{ return 2<=1; }'

# assert 1 '{ return 1>0; }'
# assert 0 '{ return 1>1; }'
# assert 0 '{ return 1>2; }'
# assert 1 '{ return 1>=0; }'
# assert 1 '{ return 1>=1; }'
# assert 0 '{ return 1>=2; }'

 assert 3 'int main(){ int a; a=3; return a; }'
 assert 8 'int main(){ a=3; z=5; return a+z; }'

assert 3 'int main(){ a=3; return a; }'
 assert 8 'int main(){ a=3; z=5; return a+z; }'
 assert 6 'int main(){ a=b=3; return a+b; }'
 assert 3 'int main(){ return2=3; return return2; }'
 assert 8 'int main(){ foo123=3; bar=5; return foo123+bar; }'

 assert 1 'int main(){ return 1; 2; 3; }'
 assert 2 'int main(){ 1; return 2; 3; }'
 assert 3 'int main(){ 1; 2; return 3; }'

 assert 3 'int main(){ {1; {2;} return 3;} }'
 #assert 5 '{ ;;; return 5; }'

 assert 3 'int main(){ if (0) return 2; return 3; }'
 assert 3 'int main(){ if (1-1) return 2; return 3; }'
 assert 2 'int main(){ if (1) return 2; return 3; }'
 assert 2 'int main(){ if (2-1) return 2; return 3; }'
  assert 4 'int main(){ if (0) { 1; 2; return 3; } else { return 4; } }'
 assert 3 'int main(){ if (1) { 1; 2; return 3; } else { return 4; } }'

 assert 55 'int main(){ i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
 assert 3 'int main(){ for (;;) {return 3;} return 5; }'

 assert 10 'int main(){ i=0; while(i<10) { i=i+1; } return i; }'

assert 3 "int func(a,b) {return a + b;} int main() {i = func(1,2); return i;}"
assert 3 "int main() {i = 3; y = &i; return *y;}"
echo OK