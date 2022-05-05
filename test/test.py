import subprocess, sys

def test_compiler(string, expect):
    asm = subprocess.run(['./../src/5cc', string], capture_output=True).stdout.decode('utf-8')
    with open('tmp.s', 'w', encoding='utf-8') as file:
        file.write(asm)
    subprocess.run(['cc', '-o', 'tmp', 'tmp.s'])
    result = subprocess.run(['./tmp']).returncode
    if result != expect:
        err = string + " => " + str(expect) + " expected, but got " + str(result) + "\n"
        sys.stdout.write(err)
    else:
        out = string + " => " + str(result) + "\n"
        sys.stdout.write(out)

def main():
    print("===================================================================")
    test_compiler("int main() {return 0;}", 0)
    test_compiler("int main(){int a; a = 0; return a;}", 0)
    test_compiler("int fib(int n) {return n;} int main() {return fib(2);}", 2)
    test_compiler("int main() {int *a;int x; x = 10; a= &x;return *a;}", 10)
    test_compiler("int main() {int *a;int x; x = 5; a= &x;*a = 10;return *a;}", 10)
    print("===================================================================")

if __name__ == '__main__':
    main()