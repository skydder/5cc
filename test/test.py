import subprocess, sys

def test_compiler(string, expect):
    asm = subprocess.run(['./../src/5cc', string], capture_output=True).stdout.decode('utf-8')
    with open('tmp.s', 'w', encoding='utf-8') as file:
        file.write(asm)
    subprocess.run(['cc', '-o', 'tmp', 'tmp.s'])
    try:
        result = subprocess.run(['./tmp']).returncode
        if result != expect:
            out = f"{string} => {expect} expected, but got {result}\n"
            sys.stdout.write(out)
        else:
            out = f"{string} => {result}\n"
            sys.stdout.write(out)
    except FileNotFoundError:
        sys.stdout.write("FileNotFoundError: [Errno 2] No such file or directory: './tmp'\n")

def test_complie_2(string, expect):
    asm = subprocess.run(['./../src/5cc', string], capture_output=True).stdout.decode('utf-8')
    with open('tmp.s', 'w', encoding='utf-8') as file:
        file.write(asm)
    subprocess.run(['gcc', '-c', 'test.c', 'tmp.s'])
    subprocess.run(['gcc', 'test.o', 'tmp.o', '-o', 'tmp'])
    try:
        result = subprocess.run(['./tmp']).returncode
        if result != expect:
            out = f"{string} => {expect} expected, but got {result}\n"
            sys.stdout.write(out)
        else:
            out = f"{string} => {result}\n"
            sys.stdout.write(out)
    except FileNotFoundError:
        sys.stdout.write("FileNotFoundError: [Errno 2] No such file or directory: './tmp'\n")
    
def main():
    print("===================================================================")
    test_compiler("int main() {return 0;}", 0)
    test_compiler("int main() {return 0 + 1;}", 1)
    #test_compiler("int main() {int a;int *b; int c;a =1; b = &a; c =*(&b+1);return c;}", 1)
    #test_compiler( "int main() {int a;int *b; int c;a =1; b = &a; c =*(b+1);return c;}", 1)
    test_compiler("int main(){int a; a = 0; return a;}", 0)
    test_compiler("int fib(int n) {return n;} int main() {return fib(2);}", 2)
    test_compiler("int main() {int *a;int x; x = 10; a= &x;return *a;}", 10)
    test_compiler("int main() {int *a;int x; x = 5; a= &x;*a = 10;return *a;}", 10)
    test_complie_2("""int main() {int *p;
    p = alloc4(&p, 1, 2, 4, 8);
    int *q;

    q = p + 2;
    q = p +3;
    return *q;
    }""", 8)

    print("===================================================================")

if __name__ == '__main__':
    main()